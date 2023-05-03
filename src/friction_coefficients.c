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

#define OBJ_HALF_X 100.f
#define OBJ_HALF_Y 50.f
#define OBJ_HALF_WIDTHS \
	(ImVec2) { OBJ_HALF_X, OBJ_HALF_Y }

typedef struct
{
	ImVec2 pos;
} Object;

static void object_show(ImVec2 wc, Object *obj)
{
	ImVec2 rmin = m_vadd(obj->pos, OBJ_HALF_WIDTHS);
	ImVec2 rmax = m_vsub(obj->pos, OBJ_HALF_WIDTHS);

	ImDrawList_AddRectFilled(__dl, m_rct(wc, rmin), m_rct(wc, rmax), IM_COL32(231,184,63, 255), 0.f, 0);
}

static void object_show_midpoint(ImVec2 wc, Object *obj)
{
	ImVec2 p0 = (ImVec2){obj->pos.x, obj->pos.y + 10.f};
	ImVec2 p1 = (ImVec2){obj->pos.x, obj->pos.y - 10.f};
	ImVec2 p2 = (ImVec2){obj->pos.x + 10.f, obj->pos.y};
	ImVec2 p3 = (ImVec2){obj->pos.x - 10.f, obj->pos.y};

	ImDrawList_AddLine(__dl, m_rct(wc, p0), m_rct(wc, p1), IM_COL32_BLACK, 1.f);
	ImDrawList_AddLine(__dl, m_rct(wc, p2), m_rct(wc, p3), IM_COL32_BLACK, 1.f);
}

#define FORCE_VEC_TIMES 8.f
#define TRIDISP_S 0.03f
#define TRIDISP_L 0.9f

static void object_force(ImVec2 wc, Object *obj, float r_dir, float mag, ImU32 col, const char *name)
{
	if (mag == 0.f)
		return;
	float rmag = mag * FORCE_VEC_TIMES;
	ImVec2 tail = m_vadd(obj->pos, m_vmuls(m_vrotate(HRZ_VEC, r_dir), rmag));
	ImVec2 p1 = m_vadd(obj->pos, m_vmuls(m_vrotate((ImVec2){TRIDISP_L, -TRIDISP_S}, r_dir), rmag));
	ImVec2 p3 = m_vadd(obj->pos, m_vmuls(m_vrotate((ImVec2){TRIDISP_L, TRIDISP_S}, r_dir), rmag));

	ImDrawList_AddLine(__dl, m_rct(wc, obj->pos), m_rct(wc, tail), col, 2.f);
	ImDrawList_AddTriangleFilled(__dl, m_rct(wc, p1), m_rct(wc, tail), m_rct(wc, p3), col);
	char buf[100];
	snprintf(buf, sizeof(buf), "%s: %gN %g°", name, mag, r_dir * RAD_TO_DEG);
	ImDrawList_AddText_Vec2(__dl, m_rct(wc, p3), IM_COL32_WHITE, buf, NULL);
}

static void object_force_side(ImVec2 wc, Object *obj, int dir, float mag, ImU32 col, const char *name, ImVec2 offset)
{
	if (mag == 0.f)
		return;
	ImVec2 vdir = (ImVec2[]){
		{1.f, 0.f},
		{0.f, 1.f},
		{-1.f, 0.f},
		{0.f, -1.f},
	}[dir];
	ImVec2 p1dir = (ImVec2[]){
		{TRIDISP_L, -TRIDISP_S},
		{-TRIDISP_S, TRIDISP_L},
		{-TRIDISP_L, -TRIDISP_S},
		{-TRIDISP_S, -TRIDISP_L},
	}[dir];
	ImVec2 p3dir = (ImVec2[]){
		{TRIDISP_L, TRIDISP_S},
		{TRIDISP_S, TRIDISP_L},
		{-TRIDISP_L, TRIDISP_S},
		{TRIDISP_S, -TRIDISP_L},
	}[dir];

	float rmag = mag * FORCE_VEC_TIMES;
	ImVec2 vstart = m_vadd(m_vmul(vdir, OBJ_HALF_WIDTHS), offset);
	ImVec2 tail = m_vadd(obj->pos, m_vadd(m_vmuls(vdir, rmag), vstart));
	ImVec2 p1 = m_vadd(obj->pos, m_vadd(m_vmuls(p1dir, rmag), vstart));
	ImVec2 p3 = m_vadd(obj->pos, m_vadd(m_vmuls(p3dir, rmag), vstart));
	ImVec2 ftext = m_vadd(obj->pos, m_vadd(m_vadd(m_vmuls(p3dir, rmag + 2.5f), (ImVec2){0.f, 45.f}), vstart));
	vstart = m_vadd(obj->pos, vstart);

	ImDrawList_AddLine(__dl, m_rct(wc, vstart), m_rct(wc, tail), col, 2.f);
	ImDrawList_AddTriangleFilled(__dl, m_rct(wc, p1), m_rct(wc, tail), m_rct(wc, p3), col);
	char buf[100];
	snprintf(buf, sizeof(buf), "%s: %gN %g°", name, mag, (float)dir * 90.f);
	ImDrawList_AddText_Vec2(__dl, m_rct(wc, ftext), IM_COL32_WHITE, buf, NULL);
}

static Object g_obj = {.pos = {0.f, 50.f}};

#define F_COLOUR IM_COL32(240, 100, 100, 255)
#define F_META_COLOUR IM_COL32(100, 120, 255, 255)

static bool show_about = true;

static void frame(void)
{
	FRAME_PASS_BEGIN

	ImVec2 wc = HANDLE_PAN();

	static float mass = 5.f;
	static float f_applied = 20.f;
	const float little_g = 9.8f;
	const float f_normal = little_g * mass;
	static float static_coefficient = 0.f;
	static float kinetic_coefficient = 0.f;

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	igTextWrapped("Welcome!");
	igSeparator();
	igTextWrapped("This is an interactive demo to visualise static and kinetic friction forces.");
	igSeparator();
	igTextWrapped("Static friction keeps objects from moving, but if enough force is applied, it will give way to kinetic friction, which is less than static friction. Adding mass to an object makes it harder to start moving, and when it is in motion, more force is needed to keep it in motion.");
	igSeparator();
	igTextWrapped("An interactive free body diagram of an object at rest is shown. Change properties such as it's mass and applied force using the control window.");
	igTextWrapped("The static and kinetic friction coefficents can be edited. A progress bar showing the force required to overpower the max static friction is underneath.");
	igCheckbox("Show About", &show_about);
	igEnd();

	igSetNextWindowPos((ImVec2){__io->DisplaySize.x / 2.0f, __io->DisplaySize.y / 2.0f}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Control Window", 0, ImGuiWindowFlags_AlwaysAutoResize);
	igSliderFloat("Mass", &mass, 0.1f, 25.f, "%g kg", ImGuiSliderFlags_AlwaysClamp);
	igSliderFloat("Applied Force", &f_applied, 1.f, 100.f, "%g N", ImGuiSliderFlags_AlwaysClamp);
	igSeparator();
	{
		const char* items[] = { "Fictional object", "Rubber on concrete", "Shoes on wood", "Steel on ice" };
		const float static_coefficents[] = { 0.6f, 1.0f, 0.9f, 0.4f };
		const float kinetic_coefficents[] = { 0.4f, 0.7f, 0.7f, 0.02f };
        static int item_curr = 0;
		if (item_curr != -1) {
			static_coefficient = static_coefficents[item_curr];
			kinetic_coefficient = kinetic_coefficents[item_curr];
		}
		if (igSliderFloat("Static Friction", &static_coefficient, 0.f, 1.f, "%g us", ImGuiSliderFlags_AlwaysClamp))
			item_curr = -1;
		if (igSliderFloat("Kinetic Friction", &kinetic_coefficient, 0.f, 1.f, "%g uk", ImGuiSliderFlags_AlwaysClamp))
			item_curr = -1;
        igCombo_Str_arr("Preset Coefficents", &item_curr, items, 4, 0);
	}

	RENDER_GRID(wc);
	ImDrawList_AddRectFilled(__dl, (ImVec2){0.f, __io->DisplaySize.y}, (ImVec2){__io->DisplaySize.x, wc.y}, IM_COL32(36,36,36, 255), 0.f, 0);

	float f_net = 0.f;
	float static_coefficient_max = static_coefficient * f_normal;
	float kinetic_coefficient_applied = 0.f;
	float overpower_applied_percentage = 0.f;

	float static_coefficient_actual = 0.f;
	if (f_applied > static_coefficient_max) {
		kinetic_coefficient_applied = kinetic_coefficient * f_normal;
		f_net = f_applied - kinetic_coefficient_applied;
		static_coefficient_max = 0.f;
		overpower_applied_percentage = 1.f;
	} else {
		static_coefficient_actual = f_applied;
		overpower_applied_percentage = static_coefficient_actual / static_coefficient_max;
	}

	object_show(wc, &g_obj);
	object_force(wc, &g_obj, 270.f * DEG_TO_RAD, little_g * mass, F_COLOUR, "Fg");
	object_force(wc, &g_obj, 90.f * DEG_TO_RAD, f_normal, F_COLOUR, "Fn");
	object_force_side(wc, &g_obj, 0, f_applied, F_COLOUR, "Fa", V2ZERO);
	object_force_side(wc, &g_obj, 2, static_coefficient_max, F_META_COLOUR, "Ffs max", (ImVec2){0.f,-10.f});
	object_force_side(wc, &g_obj, 2, static_coefficient_actual, F_COLOUR, "Ffs", (ImVec2){0.f,10.f});
	object_force_side(wc, &g_obj, 2, kinetic_coefficient_applied, F_COLOUR, "Ffk", V2ZERO);
	object_force_side(wc, &g_obj, 0, f_net, F_META_COLOUR, "Fnet", (ImVec2){0.f,10.f});

	object_show_midpoint(wc, &g_obj);

	{
		igSeparator();
		igProgressBar(overpower_applied_percentage, (ImVec2){0.f, 0.f}, NULL);
        igSameLine(.0f, igGetStyle()->ItemInnerSpacing.x);
		if (overpower_applied_percentage >= 1.f) {
			igTextColored(u32_to_vec4(IM_COL32(0,162,60, 255)), "Net Force (Fnet): %8g N", f_net);
			igTextWrapped("The current applied force is overpowering the negative force applied by static friction!");
			igTextWrapped("The current net force is no longer zero, the object is now in motion. Kinetic friction now affects the applied force.");
		} else {
			igTextColored(u32_to_vec4(F_META_COLOUR), "Net Force (Fnet): %8g N", f_net);
			igTextWrapped("The current applied force is being opposed equally by the negative force from static friction. The object is not in motion, the net force is zero.");
			igTextWrapped("Try changing the applied force and the objects mass.");
		}
	}

	igEnd();

	if (show_about) {
		ABOUT_WIDGET();
	}

	FRAME_PASS_END
}