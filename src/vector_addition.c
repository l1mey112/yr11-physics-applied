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
#define USE_INIT2
#include "demos.h"

static bool show_math;
static bool show_about;

LOCAL_STORAGE_INIT(bool, show_math, false);
LOCAL_STORAGE_INIT(bool, show_about, true);

static void init2(void)
{
	show_math = LOCAL_STORAGE_GET(show_math);
	show_about = LOCAL_STORAGE_GET(show_about);
}

typedef struct
{
	ImVec2 pos;
	ImU32 col;
} Point;

#define POINTS_CAP 20
#define POINTS_RAD 10.f
static Point g_points[POINTS_CAP] = {
	(Point){.pos = {-261.f, -149.f}, .col = IM_COL32(100, 100, 225, 255)},
	(Point){.pos = {-73.f, 76.f}, .col = IM_COL32(225, 100, 100, 255)},
	(Point){.pos = {56.f, -16.f}, .col = IM_COL32(100, 225, 100, 255)},
	(Point){.pos = {244.f, 234.f}, .col = IM_COL32(100, 225, 100, 255)},
};
static int g_points_len = 4;

static inline float norm_degrees(float a)
{
	a = fmodf(a, 360.0f);
	if (a < 0)
	{
		a += 360.0f;
	}
	return a;
}

#define RESULTANT_VECTOR_COL IM_COL32(54, 229, 223, 60)
#define RESULTANT_VECTOR_COL_FULL IM_COL32(54, 229, 223, 255)
#define TIP_END_POINT_COL IM_COL32(255, 255, 255, 40)

static void vector_final(ImDrawList *__dl, const ImVec2 world_center)
{
	const Point a = g_points[0];
	const Point b = g_points[g_points_len - 1];

	ImVec2 deg = {b.pos.x, a.pos.y};
	ImDrawList_AddLine(__dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(deg), RESULTANT_VECTOR_COL, 1.5f);
	ImDrawList_AddLine(__dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(deg), RESULTANT_VECTOR_COL, 1.5f);

	float dx = b.pos.x - a.pos.x;
	float dy = b.pos.y - a.pos.y;


	if (show_math)
	{
		float mx = (a.pos.x + b.pos.x) / 2.f;
		float my = (a.pos.y + b.pos.y) / 2.f;
		ImVec2 t0 = {mx, a.pos.y};
		ImVec2 t1 = {b.pos.x, my};
		ImVec2 t2 = {mx, my};
		char buf[50];

		float hypot = hypotf(dx, dy);
		
		snprintf(buf, sizeof(buf), "%g", dx);
		ImDrawList_AddText_Vec2(__dl, TO_REAL_COORDS(t0), RESULTANT_VECTOR_COL_FULL, buf, NULL);
		snprintf(buf, sizeof(buf), "%g", dy);
		ImDrawList_AddText_Vec2(__dl, TO_REAL_COORDS(t1), RESULTANT_VECTOR_COL_FULL, buf, NULL);
		snprintf(buf, sizeof(buf), "%g", hypot);
		ImDrawList_AddText_Vec2(__dl, TO_REAL_COORDS(t2), RESULTANT_VECTOR_COL_FULL, buf, NULL);
	}

	ImDrawList_AddLine(__dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(b.pos), RESULTANT_VECTOR_COL, 2.f);
}

static void tip_to_tail_pairs(ImDrawList *__dl, const ImVec2 world_center, const Point a, const Point b)
{
	ImDrawList_AddLine(__dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(b.pos), a.col, 2.f);

	ImVec2 deg = {b.pos.x, a.pos.y};
	ImDrawList_AddLine(__dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(deg), IM_COL32(255, 255, 255, 70), 1.5f);
	ImDrawList_AddLine(__dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(deg), IM_COL32(255, 255, 255, 70), 1.5f);

	float dx = b.pos.x - a.pos.x;
	float dy = b.pos.y - a.pos.y;
	float a1 = atan2f(-dy, dx);
	float a1_r = atan2f(dy, dx);

	if (show_math)
	{
		ImVec2 t0 = {(a.pos.x + b.pos.x) / 2.f, a.pos.y};
		ImVec2 t1 = {b.pos.x, (a.pos.y + b.pos.y) / 2.f};
		char buf[50];

		snprintf(buf, sizeof(buf), "%g", dx);
		ImDrawList_AddText_Vec2(__dl, TO_REAL_COORDS(t0), IM_COL32(255, 255, 255, 255), buf, NULL);
		snprintf(buf, sizeof(buf), "%g", dy);
		ImDrawList_AddText_Vec2(__dl, TO_REAL_COORDS(t1), IM_COL32(255, 255, 255, 255), buf, NULL);
	}

	float a0 = 0;
	if (dx < 0.f)
		a0 = M_PI;
	if (dx < 0.f && dy > 0.f)
		a0 = -M_PI;

	float arc_rad = m_min(m_abs(dx), m_abs(dy)) * 0.4f;
	if (arc_rad > 40.f)
		arc_rad = 40.f;

	ImDrawList_PathArcTo(__dl, TO_REAL_COORDS(a.pos), arc_rad, a0, a1, 0);
	ImDrawList_PathStroke(__dl, IM_COL32(255, 255, 255, 70), ImDrawFlags_None, 1.5f);

	/* ImVec2 arrw0 = m_vrotate((ImVec2){30.f, 0.f}, a1_r + 45.f * DEG_TO_RAD);
	ImVec2 arrw1 = m_vrotate((ImVec2){30.f, 0.f}, a1_r - 45.f * DEG_TO_RAD);
	arrw0 = (ImVec2){a.pos.x + arrw0.x, a.pos.y + arrw0.y};
	arrw1 = (ImVec2){a.pos.x + arrw1.x, a.pos.y + arrw1.y};

	ImDrawList_AddLine(__dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(arrw0), IM_COL32(255, 50, 50, 120), 1.5f);
	ImDrawList_AddLine(__dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(arrw1), IM_COL32(255, 50, 50, 120), 1.5f); */

	ImVec2 arrw3 = m_vrotate((ImVec2){40.f, 0.f}, a1_r + 225.f * DEG_TO_RAD);
	ImVec2 arrw4 = m_vrotate((ImVec2){40.f, 0.f}, a1_r - 225.f * DEG_TO_RAD);
	arrw3 = (ImVec2){b.pos.x + arrw3.x, b.pos.y + arrw3.y};
	arrw4 = (ImVec2){b.pos.x + arrw4.x, b.pos.y + arrw4.y};

	ImDrawList_AddLine(__dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(arrw3), a.col, 1.5f);
	ImDrawList_AddLine(__dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(arrw4), a.col, 1.5f);

	if (show_math) {
		ImVec2 theta_pos = m_vrotate((ImVec2){40.f, 0.f}, a1);
		theta_pos = (ImVec2){a.pos.x + theta_pos.x, a.pos.y + theta_pos.y};

		char buf[50];
		snprintf(buf, sizeof(buf), "(%g, %g) N%g°E", a.pos.x, a.pos.y, norm_degrees(a1_r * RAD_TO_DEG));

		ImDrawList_AddText_Vec2(__dl, TO_REAL_COORDS(theta_pos), IM_COL32(255, 255, 255, 255), buf, NULL);

		/* {
			Point last_point = g_points[0];
			for (int idx = 1; idx < g_points_len; idx++)
			{
				Point point = g_points[idx];
				igTextColored(u32_to_vec4(point.col), "(%g, %g)", point.pos.x, point.pos.y);
				igSameLine(0, 0);
				igText(" -> ");
				igSameLine(0, 0);
				igTextColored(u32_to_vec4(last_point.col), "(%g, %g)", last_point.pos.x, last_point.pos.y);
				last_point = point;
			}
		} */
	}
}

#define RECT_OFFSET 10.f

static void frame(void)
{
	FRAME_PASS_BEGIN;

	ImVec2 world_center = HANDLE_PAN();
	RENDER_GRID(world_center);

	static int is_hitting = -1;
	int is_hovering = -1;

	if (is_hitting == -1 && !__io->WantCaptureMouse)
	{
		for (int idx = 0; idx < g_points_len; idx++)
		{
			Point point = g_points[idx];

			ImVec2 rc = TO_REAL_COORDS(point.pos);

			if (igIsMouseHoveringRect((ImVec2){rc.x - (POINTS_RAD * 2.f), rc.y - (POINTS_RAD * 2.f)},
									(ImVec2){rc.x + (POINTS_RAD * 2.f), rc.y + (POINTS_RAD * 2.f)}, false))
			{
				is_hitting = idx;
				break;
			}
		}
	}

	if (igIsMouseDragging(ImGuiMouseButton_Left, 0.f))
	{
		if (is_hitting != -1)
			g_points[is_hitting].pos = TO_RELA_COORDS(__io->MousePos);
	}
	else
	{
		if (is_hitting != -1)
			is_hovering = is_hitting;
		is_hitting = -1;
	}

	for (int idx = 0; idx < g_points_len; idx++)
	{
		Point point = g_points[idx];
		if (is_hovering == idx)
			ImDrawList_AddCircleFilled(__dl, TO_REAL_COORDS(point.pos), POINTS_RAD * 1.1f, IM_COL32(255, 255, 255, 80), 0);
		else if (is_hitting == idx || idx + 1 == g_points_len)
			ImDrawList_AddCircleFilled(__dl, TO_REAL_COORDS(point.pos), POINTS_RAD, TIP_END_POINT_COL, 0);
		else
			ImDrawList_AddCircleFilled(__dl, TO_REAL_COORDS(point.pos), POINTS_RAD, point.col, 0);
	}

	ImVec2 rect_pos = g_points[0].pos;
	
	{
		for (int idx = 1; idx < g_points_len; idx++)
		{
			Point point = g_points[idx];

			if (point.pos.x > rect_pos.x)
				rect_pos.x = point.pos.x;
			if (point.pos.y > rect_pos.y)
				rect_pos.y = point.pos.y;
		}
	}

	{
		Point a = g_points[0];
		for (int idx = 1; idx < g_points_len; idx++)
		{
			Point b = g_points[idx];
			
			{
				ImVec2 b0 = (ImVec2){a.pos.x, rect_pos.y};
				ImVec2 b1 = (ImVec2){b.pos.x, rect_pos.y + RECT_OFFSET};
				b0.y += RECT_OFFSET * 2.f * idx;
				b1.y += RECT_OFFSET * 2.f * idx;
				ImDrawList_AddRectFilled(__dl, TO_REAL_COORDS(b0), TO_REAL_COORDS(b1), a.col, 0.f, 0);
			}
			
			{
				ImVec2 b0 = (ImVec2){rect_pos.x, a.pos.y};
				ImVec2 b1 = (ImVec2){rect_pos.x + RECT_OFFSET, b.pos.y};
				b0.x += RECT_OFFSET * 2.f * idx;
				b1.x += RECT_OFFSET * 2.f * idx;
				ImDrawList_AddRectFilled(__dl, TO_REAL_COORDS(b0), TO_REAL_COORDS(b1), a.col, 0.f, 0);
			}
			
			/* ImVec2 b0 = (ImVec2){a.pos.x, rect_pos.y};
			ImVec2 b1 = (ImVec2){a.pos.x, rect_pos.y + RECT_OFFSET};
			ImVec2 b2 = (ImVec2){b.pos.x, rect_pos.y + RECT_OFFSET / 2.f};
			b0.y += RECT_OFFSET * 2.f * idx;
			b1.y += RECT_OFFSET * 2.f * idx;
			b2.y += RECT_OFFSET * 2.f * idx;
			// ImDrawList_AddRectFilled(__dl, TO_REAL_COORDS(b0), TO_REAL_COORDS(b1), a.col, 0.f, 0);
			ImDrawList_AddTriangleFilled(__dl, TO_REAL_COORDS(b0), TO_REAL_COORDS(b1), TO_REAL_COORDS(b2), a.col); */
			
			tip_to_tail_pairs(__dl, world_center, a, b);
			a = b;
		}
	}

	vector_final(__dl, world_center);

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igTextWrapped("Welcome!");
		igSeparator();
		igTextWrapped("This is an interactive demo to visualise the summation of vectors.");
		igSeparator();
		igTextWrapped("Going \"tip to tail\", each vector is lined up continuing the other.");
		igTextWrapped("Finding the resultant vector is simple, and you can probably tell by the visualisation.");
		igTextWrapped("Calculate each vectors displacement on the X and Y axis using trig functions, the sum of these will be the resultant vectors displacement from the starting position.");
		igSeparator();
		igTextWrapped("Each vector contains their own respective colour.");
		igTextWrapped("Each vectors displacement is visualised with rectangles.");
		igTextWrapped("Right click and drag to pan around the scene.");
		igTextWrapped("Left click and drag on the circle control points to move the vectors around.");
	}
	igPushStyleColor_U32(ImGuiCol_Text, RESULTANT_VECTOR_COL_FULL);
	{
		igTextWrapped("The resultant vector is in this special colour.");
	}
	igPopStyleColor(1);
	igSeparator();
	if (igCheckbox("Show Math Overlay", &show_math))
		LOCAL_STORAGE_SET(show_math, show_math);
	if (igCheckbox("Show About", &show_about))
		LOCAL_STORAGE_SET(show_about, show_about);
	/* igSeparator();
	{
		Point last_point = g_points[0];
		for (int idx = 1; idx < g_points_len; idx++)
		{
			Point point = g_points[idx];
			igTextColored(u32_to_vec4(last_point.col), "(%g, %g)", last_point.pos.x, last_point.pos.y);
			igSameLine(0, 0);
			igText(" -> ");
			igSameLine(0, 0);
			ImU32 ecol = idx + 1 == g_points_len ? TIP_END_POINT_COL : point.col;
			igTextColored(u32_to_vec4(ecol), "(%g, %g)", point.pos.x, point.pos.y);
			last_point = point;
		}
	} */
	igEnd();

	if (show_about) {
		ABOUT_WIDGET();
	}

	FRAME_PASS_END;
}