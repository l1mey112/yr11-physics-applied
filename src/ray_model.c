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
#include "demos.h"

static bool show_about = true;

typedef struct Object Object;
typedef struct Stroke Stroke;
typedef struct Pointer Pointer;

struct Pointer
{
	ImVec2 pos;
	float rot;
};

struct Object
{
	ImVec2 pos;
	float rot;
	enum
	{
		MIRROR,
	} type;
};

struct Stroke
{
	enum
	{
		BASIC,
		ENDLESS, // end is a normalised vector
	} type;
	ImVec2 start, end;
};

static int pointers_len = 1;
static Pointer pointers[8] = {
	{
		.pos = {0.f, 0.f},
	},
};

static int world_len = 1;
static Object world[32] = {
	{.pos = {300.f, 0.f}, .rot = M_PI / 4.0f, .type = MIRROR},
};

static int path_len = 0;
static Stroke path[32];

static ImVec2 nrm_angle(float rad)
{
	ImVec2 ret;
	ret.x = cosf(rad);
	ret.y = -sinf(rad);
	return ret;
}

static ImVec2 screen_intersection(ImVec2 ro, ImVec2 rd)
{
	float in_x;
	float in_y;

	if (rd.x != 0.f)
	{
		if (rd.x > 0.f)
			in_x = __io->DisplaySize.x;
		else
			in_x = 0.f;
		in_y = ro.y + (in_x - ro.x) * rd.y / rd.x;
	}
	else
	{
		in_x = ro.x;
		if (rd.y > 0.f)
			in_y = __io->DisplaySize.y;
		else
			in_y = 0.f;
	}

	if (in_y >= 0.f && in_y <= __io->DisplaySize.y)
	{
		return (ImVec2){in_x, in_y};
	}

	if (rd.y > 0.f)
		in_y = __io->DisplaySize.y;
	else
		in_y = 0.f;

	in_x = ro.x + (in_y - ro.y) * rd.x / rd.y;

	return (ImVec2){in_x, in_y};
}

// returns the amount of interesection points
static int intersect(Object *o, ImVec2 *p1, ImVec2 *p2)
{
	return 0;
}

static bool append_stroke(Stroke stroke)
{
	if (path_len + 1 >= 32)
		return false;

	path[path_len++] = stroke;

	return true;
}

static void traceray(ImVec2 ro, ImVec2 rd)
{
	float sq_distance = INFINITY;

	Object *c_hit = NULL;
	ImVec2 c_hit_p1, c_hit_p2;

	int c;

	for (int i = 0; i < world_len; i++)
	{
		Object *o = &world[i];
		ImVec2 p1, p2;

		if ((c = intersect(o, &p1, &p2)) == 0)
			continue;

		float nsqx = p1.x - ro.x;
		float nsqy = p1.y - ro.y;
		float nsq = nsqx * nsqx + nsqy * nsqy;

		if (nsq < sq_distance)
		{
			sq_distance = nsq;
			c_hit = o;
			c_hit_p1 = p1;
			c_hit_p2 = p2;
		}
	}

	if (!c_hit)
	{
		Stroke stroke = {ENDLESS, ro, rd};
		append_stroke(stroke);
		return;
	}

	Stroke stroke = {BASIC, ro, c_hit_p1};
	append_stroke(stroke);
}

static void trace(Pointer *p)
{
	ImVec2 ro, rd;

	ro = p->pos;
	rd = nrm_angle(p->rot);

	traceray(ro, rd);
}

static void recalculate()
{
	path_len = 0;

	for (int p = 0; p < pointers_len; p++)
	{
		trace(&pointers[p]);
	}
}

static void frame(void)
{
	FRAME_PASS_BEGIN;

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	igText("%f", __io->DeltaTime);
	igCheckbox("Show About", &show_about);
	igEnd();

	ImVec2 wc = HANDLE_PAN();
	RENDER_GRID(wc);

	pointers[0].rot += __io->DeltaTime;
	recalculate();

	for (int i = 0; i < pointers_len; i++)
	{
		ImDrawList_AddCircleFilled(__dl, m_rct(wc, pointers[i].pos), 4.f, IM_COL32(255, 255, 255, 255), 0);
	}

	for (int i = 0; i < world_len; i++)
	{
		Object *object = &world[i];

		switch (object->type)
		{
		case MIRROR:
		{
			const float MIRROR_HSPAN = 100.f;
			
			ImVec2 span = m_vmuls(nrm_angle(object->rot), MIRROR_HSPAN);
			ImVec2 p1 = m_vadd(span, object->pos);
			ImVec2 p2 = m_vadd(m_vflip(span), object->pos);

			printf("(%f, %f) (%f, %f)\n", p1.x, p1.y, p2.x, p2.y);
			
			ImDrawList_AddLine(__dl, m_rct(wc, p1), m_rct(wc, p2), IM_COL32(255, 255, 255, 255), 2.f);
			break;
		}
		}
	}

	for (int i = 0; i < path_len; i++)
	{
		Stroke *stroke = &path[i];

		// printf("[%d] %d (%f, %f) (%f, %f)\n", stroke->type, i, stroke->start.x, stroke->start.y, stroke->end.x, stroke->end.y);

		ImVec2 start, end;

		switch (stroke->type)
		{
		case BASIC:
			start = m_rct(wc, stroke->start);
			end = m_rct(wc, stroke->end);
			break;
		case ENDLESS:
			start = m_rct(wc, stroke->start);
			end = screen_intersection(start, stroke->end);
			break;
		}

		ImDrawList_AddLine(__dl, start, end, IM_COL32(255, 255, 255, 255), 2.f);
	}

	if (show_about)
	{
		ABOUT_WIDGET();
	}

	FRAME_PASS_END;
}