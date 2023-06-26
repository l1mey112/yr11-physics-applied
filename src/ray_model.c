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

#define MIRROR_HSPAN 100.f

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
	ret.y = sinf(rad);
	return ret;
}

static void points_from_mirror(ImVec2 pos, float rot, ImVec2 *p1, ImVec2 *p2)
{
	ImVec2 span = m_vmuls(nrm_angle(rot), MIRROR_HSPAN);
	*p1 = m_vadd(span, pos);
	*p2 = m_vadd(m_vflip(span), pos);
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

float cross(ImVec2 p1, ImVec2 p2) { return p1.x * p2.y - p1.y * p2.x; }

// returns the amount of interesection points
static int intersect(Object *o, ImVec2 ro, ImVec2 rd, ImVec2 *p1, ImVec2 *p2)
{
	switch (o->type)
	{
	case MIRROR:
	{
		ImVec2 m1, m2;
		points_from_mirror(o->pos, o->rot, &m1, &m2);

		// a line `m1` to `m2` vs an intersection by a ray `ro` of normalised direction `rd`

		// intersect the ray origin `ro`, with ray normalised direction `rd`
		// with the points `m1` and `m2`

		/* float denominator = (m2.x - m1.x) * (rd.y) - (m2.y - m1.y) * (rd.x);

		if (fabs(denominator) < 0.0001)
		{
			break;
		}

		float t = ((m1.x - ro.x) * (rd.y) - (m1.y - ro.y) * (rd.x)) / denominator;
		float u = -((m1.x - m2.x) * (m1.y - ro.y) - (m1.y - m2.y) * (m1.x - ro.x)) / denominator;

		if (t > 0 && t < 1 && u > 0)
		{
			p1->x = m1.x + t * (m2.x - m1.x);
			p1->y = m1.y + t * (m2.y - m1.y);
			
			printf("hit!\n");
			return 1;
		} */

		/* float x1 = ro.x;
		float y1 = ro.y;
		float x2 = ro.x + rd.x;
		float y2 = ro.y + rd.y;
		float x3 = m1.x;
		float y3 = m1.y;
		float x4 = m2.x;
		float y4 = m2.y;

		float denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

		if (denominator == 0) {
			break;
		}

		float numerator1 = (x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4);
		float numerator2 = (x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3);

		float t = numerator1 / denominator;
		float u = -numerator2 / denominator;

		if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
			p1->x = x1 + t * (x2 - x1);
			p1->y = y1 + t * (y2 - y1);
			return 1;
		} */

		float denominator = rd.x * (m2.y - m1.y) - rd.y * (m2.x - m1.x);
		if (denominator != 0) {
			float t = ((ro.x - m1.x) * (m2.y - m1.y) - (ro.y - m1.y) * (m2.x - m1.x)) / denominator;
			if (t >= 0) {
				p1->x = ro.x + t * rd.x;
				p1->y = ro.y + t * rd.y;
				return 1;
			}
		}

		break;
	}
	}
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

		if ((c = intersect(o, ro, rd, &p1, &p2)) == 0)
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
			ImVec2 p1, p2;
			points_from_mirror(object->pos, object->rot, &p1, &p2);

			// printf("(%f, %f) (%f, %f)\n", p1.x, p1.y, p2.x, p2.y);

			ImDrawList_AddLine(__dl, m_rct(wc, p1), m_rct(wc, p2), IM_COL32(255, 255, 255, 255), 2.f);
			ImDrawList_AddCircleFilled(__dl, m_rct(wc, p1), 4.f, IM_COL32(255, 255, 255, 255), 0);
			ImDrawList_AddCircleFilled(__dl, m_rct(wc, p2), 4.f, IM_COL32(255, 255, 255, 255), 0);
			break;
		}
		}
	}

	for (int i = 0; i < path_len; i++)
	{
		Stroke *stroke = &path[i];

		// printf("[%d] %d (%f, %f) (%f, %f)\n", stroke->type, i, stroke->start.x, stroke->start.y, stroke->end.x, stroke->end.y);

		ImVec2 start, end;

		start = m_rct(wc, stroke->start);

		switch (stroke->type)
		{
		case BASIC:
			end = m_rct(wc, stroke->end);
			break;
		case ENDLESS:
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