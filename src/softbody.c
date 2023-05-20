/*
 * Copyright (C) 2022 l-m.dev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
// This code is a port of my "Softbody dynamics in the terminal" softbody simulation
// Blog post: https://l-m.dev/cs/softbody-dynamics-terminal/
// Github link: https://github.com/l1mey112/v-2Dsoftbodies
#define USE_INIT2
#include "demos.h"
#include <assert.h>

static bool show_about = true;

#define VERT_COUNT_MAX 16

typedef struct Vertex Vertex;
typedef struct Spring Spring;
typedef struct SoftbodyCircle SoftbodyCircle;

struct Vertex
{
	ImVec2 pos;
	ImVec2 vel;
	ImVec2 force;
	float mass;
	bool is_fixed;
};

struct Spring
{
	Vertex *a;
	Vertex *b;
	float rest_length;
	float stiffness;
	float damping;
	float factor;
};

struct SoftbodyCircle
{
	float radius;	 // will be useless after made "real"
	ImVec2 position; // will be useless after made "real"
	int obj_count;
	bool is_real;
	Vertex vertices[VERT_COUNT_MAX];
	Spring springs[VERT_COUNT_MAX];
};

SoftbodyCircle softbody = {
	.radius = 100.f,
	.position = {0},
	.obj_count = VERT_COUNT_MAX,
	.is_real = false,
};

static void make_real_softbody(float mass, float stiffness, float damping)
{
	assert(softbody.obj_count >= 3);
	assert(softbody.obj_count % 2 == 0);
	assert(!softbody.is_real);
	softbody.is_real = true;

	float mass_per = mass / (float)softbody.obj_count;

	for (int i = 0; i < softbody.obj_count; i++)
	{
		float angle = (float)i / (float)softbody.obj_count * 2.f * M_PI;

		ImVec2 position = {
			softbody.position.x + softbody.radius * cos(angle),
			softbody.position.y + softbody.radius * sin(angle),
		};

		softbody.vertices[i] = (Vertex){
			.pos = position,
			.vel = {0},
			.force = 0,
			.mass = mass_per,
			.is_fixed = false,
		};
	}

	float rest_length = m_distance(softbody.vertices[0].pos, softbody.vertices[1].pos);

	for (int i = 0; i < softbody.obj_count; i++)
	{
		softbody.springs[i] = (Spring){
			.a = &softbody.vertices[i],
			.b = &softbody.vertices[(i + 1) % softbody.obj_count],
			.rest_length = rest_length,
			.stiffness = stiffness,
			.damping = damping,
		};
	}
}

static void render_softbody(ImVec2 wc)
{
	for (int i = 0; i < softbody.obj_count; i++)
	{
		Vertex *vertex = &softbody.vertices[i];
		ImDrawList_AddCircleFilled(__dl, m_rct(wc, vertex->pos), 2.f, IM_COL32_WHITE, 0);
		// ImDrawList_AddLine(__dl, m_rct(wc, vertex->pos), m_rct(wc, m_vadd(vertex->pos, vertex->force)), IM_COL32_WHITE, 0);
	}

	for (int i = 0; i < softbody.obj_count; i++)
	{
		Spring *spring = &softbody.springs[i];
		ImDrawList_AddLine(__dl, m_rct(wc, spring->a->pos), m_rct(wc, spring->b->pos), IM_COL32_WHITE, 0);
	}
}

static void integrate_softbody(float dt)
{
	const float little_g = -9.8f * 60.f;

	// --- gravity [reset forces]
	for (int i = 0; i < softbody.obj_count; i++)
	{
		Vertex *vertex = &softbody.vertices[i];

		if (vertex->is_fixed)
			continue;

		vertex->force.x = 0.f;
		vertex->force.y = little_g * vertex->mass;
	}

	// --- spring hookes law
	for (int i = 0; i < softbody.obj_count; i++)
	{
		Spring *spring = &softbody.springs[i];

		float distance = m_distance(spring->a->pos, spring->b->pos) - spring->rest_length;
		float force = distance * spring->stiffness;

		ImVec2 fnet_dir = m_normalise(m_vsub(spring->a->pos, spring->b->pos));
		float dot = m_dot(fnet_dir, m_vsub(spring->a->vel, spring->b->vel));

		force += dot * spring->damping;

		if (!spring->a->is_fixed)
		{
			ImVec2 a_to_b = m_normalise(m_vsub(spring->b->pos, spring->a->pos));

			spring->a->force.x += a_to_b.x * force;
			spring->a->force.y += a_to_b.y * force;
		}

		if (!spring->b->is_fixed)
		{
			ImVec2 b_to_a = fnet_dir;

			spring->b->force.x += b_to_a.x * force;
			spring->b->force.y += b_to_a.y * force;
		}

		spring->factor = distance / spring->rest_length;
	}

	// --- apply ideal gas law [softbody physics]

	// clang-format off
	// PV = nRT: the ideal gas law
	//     P = Pressure
	//     V = Volume
	//     n = Moles
	//     R = Universal gas constant
	//     T = Temperature in kelvin
	const float nrt = 5000.0f             /* moles */ 
	                * 8.31446261815324 /* molar gas constant */
	                * 293.15;          /* room temperature in kelvin */
	//
	// P = F/A
	//     Pressure = Force / Area
	//
	// --- use a little algebra...
	//
	//     Force = Area * nRT / Volume
	//
	// --- we live in a 2D world...
	//
	//     Force = Spring Length * nRT / Shape Area
	//
	// clang-format on

	// --- calculate area
	float area = 0.f;
	for (int i = 0; i < softbody.obj_count; i++)
	{
		int i_next = (i + 1) % softbody.obj_count;
		area += softbody.vertices[i].pos.y * softbody.vertices[i_next].pos.x - softbody.vertices[i].pos.x * softbody.vertices[i_next].pos.x;
	}
	area *= -0.5f;

	// --- perform ideal gas law
	for (int i = 0; i < softbody.obj_count; i++)
	{
		Spring *spring = &softbody.springs[i];

		float distance = m_distance(spring->a->pos, spring->b->pos);
		float force = distance * nrt / area;

		// rotate by 90 degrees to obtain **A** normal vector, not specifically the right one
		ImVec2 nrm_dir = m_normalise(m_vsub(spring->a->pos, spring->b->pos));
		nrm_dir = (ImVec2){nrm_dir.y, -nrm_dir.x};

		Spring *opposite_spring = &softbody.springs[(i + softbody.obj_count / 2) % softbody.obj_count];
		ImVec2 opposite_midpoint = m_midpoint(opposite_spring->a->pos, opposite_spring->b->pos);
		ImVec2 midpoint = m_midpoint(spring->a->pos, spring->b->pos);
		ImVec2 outward_vec = m_normalise(m_vsub(midpoint, opposite_midpoint));

		// calculate the right normal vector
		if (m_dot(nrm_dir, outward_vec) < 0.f)
		{
			nrm_dir.x *= -1;
			nrm_dir.y *= -1;
		}

		float dx = nrm_dir.x * force;
		float dy = nrm_dir.y * force;

		igText("(%g, %g)", dx, dy);

		spring->a->force.x += dx;
		spring->a->force.y += dy;
		spring->b->force.x += dx;
		spring->b->force.y += dy;
	}

	// --- integrate forces and apply colision
	for (int i = 0; i < softbody.obj_count; i++)
	{
		Vertex *vertex = &softbody.vertices[i];

		if (vertex->is_fixed)
			continue;

		// --- semi implicit euler

		// a = F / m
		// v += a * dt
		vertex->vel.x += vertex->force.x / vertex->mass * dt;
		vertex->vel.y += vertex->force.y / vertex->mass * dt;

		// p += v * dt
		vertex->pos.x += vertex->vel.x * dt;
		vertex->pos.y += vertex->vel.y * dt;

		// --- apply line colision
		const float collision_damping = 0.5f;

		if (vertex->pos.y < 0.f)
		{
			float t = -vertex->pos.y / vertex->vel.y;

			vertex->pos.x += vertex->vel.x * t;
			vertex->pos.y += vertex->vel.y * t;
			vertex->vel.y *= -collision_damping;
		}
	}
}

static void init2(void)
{
	softbody.position.y = 100.f;
	make_real_softbody(15.f, 1000.f, 15.f);
}

static void frame(void)
{
	FRAME_PASS_BEGIN

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igCheckbox("Show About", &show_about);
	}
	igEnd();

	ImVec2 wc = HANDLE_PAN();
	RENDER_GRID(wc);

	static float acc = 0.0f;
	const float phys_dt = 1.f / 60.f; // 50fps phys update

	acc += __io->DeltaTime;
	while (acc >= phys_dt)
	{
		integrate_softbody(phys_dt);
		acc -= phys_dt;
	}

	render_softbody(wc);

	if (show_about)
	{
		ABOUT_WIDGET();
	}

	FRAME_PASS_END
}