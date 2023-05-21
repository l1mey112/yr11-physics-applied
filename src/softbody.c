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

#define VERT_COUNT_MAX 18

typedef struct Vertex Vertex;
typedef struct Spring Spring;
typedef struct SoftbodyCircle SoftbodyCircle;

struct Vertex
{
	ImVec2 pos;
	ImVec2 vel;
	float mass;
	bool is_fixed;
	ImVec2 gravity_force;
	ImVec2 spring_force;
	ImVec2 pressure_force;
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
	float radius;
	ImVec2 position;
	int obj_count;
	bool is_real;
	Vertex vertices[VERT_COUNT_MAX];
	Spring springs[VERT_COUNT_MAX];
};

SoftbodyCircle softbody = {
	.radius = 150.f,
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

static void integrate_softbody(float dt, float nrt, float *out_internal_pressure, float *out_area)
{
	const float little_g = -9.8f * 80.f;

	// --- gravity [reset forces]
	for (int i = 0; i < softbody.obj_count; i++)
	{
		Vertex *vertex = &softbody.vertices[i];

		if (vertex->is_fixed)
			continue;

		vertex->spring_force.x = 0.f;
		vertex->spring_force.y = 0.f;
		vertex->pressure_force.x = 0.f;
		vertex->pressure_force.y = 0.f;
		vertex->gravity_force.x = 0.f;
		vertex->gravity_force.y = little_g * vertex->mass;
	}

	// --- spring hookes law
	for (int i = 0; i < softbody.obj_count; i++)
	{
		Spring *spring = &softbody.springs[i];

		float distance = m_distance(spring->a->pos, spring->b->pos) - spring->rest_length;
		float force = distance * spring->stiffness;

		ImVec2 fnet_dir = m_normalise(m_vsub(spring->a->pos, spring->b->pos));

		float dot = m_dot(fnet_dir, m_vsub(spring->a->vel, spring->b->vel));
		float damping_force = dot * spring->damping;

		force += damping_force;

		if (!spring->a->is_fixed)
		{
			ImVec2 a_to_b = m_normalise(m_vsub(spring->b->pos, spring->a->pos));

			spring->a->spring_force.x += a_to_b.x * force / spring->a->mass;
			spring->a->spring_force.y += a_to_b.y * force / spring->a->mass;
		}

		if (!spring->b->is_fixed)
		{
			ImVec2 b_to_a = fnet_dir;

			spring->b->spring_force.x += b_to_a.x * force / spring->b->mass;
			spring->b->spring_force.y += b_to_a.y * force / spring->b->mass;
		}

		spring->factor = distance / spring->rest_length;
		if (spring->factor < 0.f)
			spring->factor = 0.f;
		if (spring->factor > 1.f)
			spring->factor = 1.f;
	}

	// --- apply ideal gas law [softbody physics]

	// clang-format off
	// PV = nRT: the ideal gas law
	//     P = Pressure
	//     V = Volume
	//     n = Moles
	//     R = Universal gas constant
	//     T = Temperature in kelvin
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
		area += softbody.vertices[i].pos.x *
					softbody.vertices[i_next].pos.y -
				softbody.vertices[i_next].pos.x *
					softbody.vertices[i].pos.y;
	}
	area = fabs(area * 0.5);

	float internal_pressure = nrt / area;
	*out_internal_pressure = internal_pressure;
	*out_area = area;

	// --- perform ideal gas law
	for (int i = 0; i < softbody.obj_count; i++)
	{
		Spring *spring = &softbody.springs[i];
		Spring *opposite_spring = &softbody.springs[(i + softbody.obj_count / 2) % softbody.obj_count];

		float distance = m_distance(spring->a->pos, spring->b->pos);
		// p = F/A
		// F = pA
		float force = internal_pressure * distance;

		ImVec2 midpoint = m_midpoint(spring->a->pos, spring->b->pos);
		ImVec2 opposite_midpoint = m_midpoint(opposite_spring->a->pos, opposite_spring->b->pos);
		ImVec2 outward_vec = m_vsub(midpoint, opposite_midpoint);

		ImVec2 nrm_dir = m_normalise(m_vsub(spring->b->pos, spring->a->pos));
		nrm_dir = (ImVec2){nrm_dir.y, -nrm_dir.x};

		// calculate the right normal vector
		if (m_dot(nrm_dir, outward_vec) < 0.f)
		{
			nrm_dir.x *= -1;
			nrm_dir.y *= -1;
		}

		float dx = nrm_dir.x * force;
		float dy = nrm_dir.y * force;

		spring->a->pressure_force.x += dx;
		spring->a->pressure_force.y += dy;
		spring->b->pressure_force.x += dx;
		spring->b->pressure_force.y += dy;
	}

	// --- integrate forces and apply collision
	for (int i = 0; i < softbody.obj_count; i++)
	{
		Vertex *vertex = &softbody.vertices[i];

		if (vertex->is_fixed)
			continue;

		// --- semi implicit euler

		// a = F / m
		// v += a * dt

		float fx = vertex->gravity_force.x + vertex->spring_force.x + vertex->pressure_force.x;
		float fy = vertex->gravity_force.y + vertex->spring_force.y + vertex->pressure_force.y;

		vertex->vel.x += fx / vertex->mass * dt;
		vertex->vel.y += fy / vertex->mass * dt;

		// p += v * dt
		vertex->pos.x += vertex->vel.x * dt;
		vertex->pos.y += vertex->vel.y * dt;

		// --- apply line collision
		const float collision_damping = 0.5f;

		if (vertex->pos.y < 0.f)
		{
			vertex->pos.y = 0.f;
			vertex->vel.y = -vertex->vel.y * collision_damping;
			vertex->vel.x = vertex->vel.x * collision_damping;
		}
	}
}

static void init2(void)
{
	softbody.position.y = 150.f;
	make_real_softbody(15.f, 1000.f, 15.f);
}

// 90fps phys update, yeah I know.
// I can easily get away with 60fps, 50fps is where the simulation breaks down.
// 50fps is possible with verlet integration, but then makes the code unergonomic and annoying to use.
// Semi Implicit Euler is good for now.
const float phys_dt = 1.f / 90.f;

static void show_arrow(ImVec2 wc, ImVec2 pos, ImVec2 force, float p, ImU32 col)
{
	force.x *= p;
	force.y *= p;

	arrow(m_rct(wc, pos), m_rct(wc, m_vadd(pos, force)), col, 2.f, 10.f);
}

static void frame(void)
{
	FRAME_PASS_BEGIN

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igTextWrapped("Welcome to the Softbody Pressure Simulation!");
		igTextWrapped("This simulation allows you to explore the behavior of a softbody system.");
		igSeparator();
		igTextWrapped("The softbody is a collection of points constrained by springs, being pushed out by an internal pressure bounded by the Ideal Gas Law.");
		igTextWrapped("Internal pressure is a product of the current temperature and number of substance, this value is then used to calculate internal pressure forces using the shapes volume.");
		igSeparator();
		igTextWrapped("You can click and drag the points to move them around, observing how the softbody deforms and reacts to your actions. Have fun!");
		igSeparator();
		igCheckbox("Show About", &show_about);
	}
	igEnd();

	static float pressure_plot_points[128] = {0};
	static int pressure_plot_cycle = 0;

	static float n = 2000.0f;
	const float r = 8.31446261815324;
	static float t = 293.15;

	ImVec2 wc = HANDLE_PAN();
	RENDER_GRID(wc);

	static int is_hitting = -1;
	int is_hovering = -1;

	if (is_hitting == -1 && !__io->WantCaptureMouse)
	{
		for (int i = 0; i < softbody.obj_count; i++)
		{
			Vertex *vertex = &softbody.vertices[i];

			ImVec2 rc = m_rct(wc, vertex->pos);

			if (igIsMouseHoveringRect(m_offset(rc, -25.f, -25.f), m_offset(rc, 25.f, 25.f), false))
			{
				is_hitting = i;
				break;
			}
		}
	}

	if (igIsMouseDragging(ImGuiMouseButton_Left, 0.f))
	{
		if (is_hitting != -1)
		{
			Vertex *vertex = &softbody.vertices[is_hitting];

			vertex->is_fixed = true;
			vertex->pos = m_ract(wc, __io->MousePos);
		}
	}
	else
	{
		if (is_hitting != -1)
		{
			Vertex *vertex = &softbody.vertices[is_hitting];

			vertex->is_fixed = false;
			is_hovering = is_hitting;
		}
		is_hitting = -1;
	}

	// --- physics

	static float acc = 0.0f;
	float internal_pressure;
	float area;
	float nrt = n * r * t;

	acc += __io->DeltaTime;
	while (acc >= phys_dt)
	{
		integrate_softbody(phys_dt, nrt, &internal_pressure, &area);
		acc -= phys_dt;
	}

	pressure_plot_points[pressure_plot_cycle] = internal_pressure;
	pressure_plot_cycle = (pressure_plot_cycle + 1) % IM_ARRAYSIZE(pressure_plot_points);

	static bool show_total_force_lines = false;
	static bool show_sp_force_lines = true;
	static char buf[32];

	ImVec2 avail;
	float isp = igGetStyle()->ItemInnerSpacing.x;
	igGetContentRegionAvail(&avail);

	igSetNextWindowPos((ImVec2){__io->DisplaySize.x / 2.0f, __io->DisplaySize.y / 2.0f}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Control Window", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igSliderFloat("Moles (n)", &n, 100.f, 10000.f, "%g mol", 0);
		igSliderFloat("Temperature (T)", &t, 100.f, 700.f, "%g K", 0);
		igSeparator();
		{
			nice_box(FORMAT(buf, "n: %6.5g", n), IM_COL32(0, 0, 255, 255));
			igSameLine(.0f, isp);
			igText("*");
			igSameLine(.0f, isp);
			nice_box(FORMAT(buf, "R: %3g", r), IM_COL32(128, 0, 128, 255));
			igSameLine(.0f, isp);
			igText("*");
			igSameLine(.0f, isp);
			nice_box(FORMAT(buf, "T: %5.4g", t), IM_COL32(200, 145, 0, 255));
			igSameLine(.0f, isp);
			igText("=");
			igSameLine(.0f, isp);
			nice_box(FORMAT(buf, "nRT: %d", (int)nrt), IM_COL32(100, 160, 80, 255));
		}
		{
			nice_box(FORMAT(buf, "P: %4.3g", internal_pressure), IM_COL32(200, 50, 0, 255));
			igSameLine(.0f, isp);
			igText("=");
			igSameLine(.0f, isp);
			nice_box(FORMAT(buf, "nRT: %d", (int)nrt), IM_COL32(100, 160, 80, 255));
			igSameLine(.0f, isp);
			igText("/");
			igSameLine(.0f, isp);
			nice_box(FORMAT(buf, "V: %d", (int)area), IM_COL32(200, 20, 40, 255));
		}
	}
	igSeparator();
	{
		sprintf(buf, "Pressure: %8g", internal_pressure);
		igPlotLines_FloatPtr(NULL, pressure_plot_points, IM_ARRAYSIZE(pressure_plot_points), pressure_plot_cycle, buf, __FLT_MIN__, __FLT_MAX__, (ImVec2){avail.x, 80.f}, sizeof(float));
	}
	igSeparator();
	{
		igCheckbox("Show Total Force Lines", &show_total_force_lines);
		igCheckbox("Show Spring And Pressure Force Lines", &show_sp_force_lines);
	}
	igEnd();

	// --- render

	// ImDrawList_AddQuadFilled(__dl, (ImVec2){0.f, wc.y}, (ImVec2){__io->DisplaySize.x, wc.y}, (ImVec2){__io->DisplaySize.x, __io->DisplaySize.y}, (ImVec2){0.f, __io->DisplaySize.y}, IM_COL32(50, 50, 50, 255));

	for (int i = 0; i < softbody.obj_count; i++)
	{
		Spring *spring = &softbody.springs[i];

		// igText("factor: %g", spring->factor);

		ImU32 c = lerp_color(IM_COL32(0, 255, 0, 255), IM_COL32(255, 0, 0, 255), spring->factor);

		ImDrawList_AddLine(__dl, m_rct(wc, spring->a->pos), m_rct(wc, spring->b->pos), c, (1.f - spring->factor) * 2.f + 1.f);
	}

	for (int i = 0; i < softbody.obj_count; i++)
	{
		Vertex *vertex = &softbody.vertices[i];

		/* {
			ImVec2 nvec = m_normalise(vertex->force);
			nvec.x *= 100.f;
			nvec.y *= 100.f;

			ImDrawList_AddLine(__dl, m_rct(wc, vertex->pos), m_rct(wc, m_vadd(vertex->pos, nvec)), IM_COL32(255, 255, 255, 80), 0);
		} */

		/* if (!vertex->is_fixed)
		{
			ImVec2 nvec = vertex->force;
			nvec.x *= 0.010f;
			nvec.y *= 0.010f;

			// ImDrawList_AddLine(__dl, m_rct(wc, vertex->pos), m_rct(wc, m_vadd(vertex->pos, nvec)), IM_COL32(255, 255, 255, 80), 0);
			arrow(m_rct(wc, vertex->pos), m_rct(wc, m_vadd(vertex->pos, nvec)), IM_COL32(255, 255, 255, 80), 2.f, 10.f);
		} */


		if (!vertex->is_fixed)
		{
			if (show_sp_force_lines)
			{
				show_arrow(wc, vertex->pos, vertex->spring_force, 0.01f, IM_COL32(255, 255, 255, 80));
				show_arrow(wc, vertex->pos, vertex->pressure_force, 0.01f, IM_COL32(255, 255, 255, 80));
			}
			if (show_total_force_lines)
			{
				ImVec2 total = m_vadd(m_vadd(vertex->gravity_force, vertex->spring_force), vertex->pressure_force);
				show_arrow(wc, vertex->pos, total, 0.01f, IM_COL32(152,138,189, 255));
			}
		}

		ImDrawList_AddCircleFilled(__dl, m_rct(wc, vertex->pos), 4.f, IM_COL32_WHITE, 0);
		if (is_hovering == i)
		{
			ImDrawList_AddCircleFilled(__dl, m_rct(wc, vertex->pos), 25.f, IM_COL32(0, 0, 0, 100), 0);
		}
		else if (is_hitting == i)
		{
			ImDrawList_AddCircleFilled(__dl, m_rct(wc, vertex->pos), 20.f, IM_COL32(255, 0, 0, 100), 0);
		}
	}

	/* avg_center.x /= (float)softbody.obj_count;
	avg_center.y /= (float)softbody.obj_count;
	avg_force.x /= (float)softbody.obj_count;
	avg_force.y /= (float)softbody.obj_count;

	show_arrow(wc, avg_center, avg_force, 0.01f, IM_COL32(255, 255, 255, 255)); */

	if (show_about)
	{
		ABOUT_WIDGET();
	}

	FRAME_PASS_END
}