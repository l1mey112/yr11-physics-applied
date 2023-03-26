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
#ifndef DEMOS_H
#define DEMOS_H

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_imgui.h"
#include "HandmadeMath.h"
#include <math.h>
#include <emscripten.h>

#define DEG_TO_RAD (M_PI / 180.f)
#define RAD_TO_DEG (180.f / M_PI)

#define V2ZERO (ImVec2){0.f, 0.f}

#define TO_REAL_COORDS(vec) \
	m_vadd(m_real_coords(vec), world_center)
#define TO_RELA_COORDS(vec) \
	(ImVec2) { vec.x - world_center.x, -(vec.y - world_center.y) }

static inline ImVec2 m_real_coords(ImVec2 a)
{
	return (ImVec2){a.x, -a.y};
}

static inline ImVec2 m_vadd(ImVec2 a, ImVec2 b)
{
	return (ImVec2){a.x + b.x, a.y + b.y};
}

static inline ImVec2 m_vsub(ImVec2 a, ImVec2 b)
{
	return (ImVec2){a.x - b.x, a.y - b.y};
}

static inline ImVec2 m_vmul(ImVec2 a, ImVec2 b)
{
	return (ImVec2){a.x * b.x, a.y * b.y};
}

static inline ImVec2 m_vmuls(ImVec2 a, float b)
{
	return (ImVec2){a.x * b, a.y * b};
}

static inline ImVec2 m_rct(ImVec2 world_center, ImVec2 a)
{
	return m_vadd(m_real_coords(a), world_center);
}

static ImVec2 m_vrotate(ImVec2 v, float theta)
{
	return (ImVec2){v.x * cosf(theta) - v.y * sinf(theta), v.x * sinf(theta) + v.y * cosf(theta)};
}

static inline float m_max(float a, float b)
{
	return a > b ? a : b;
}

static inline float m_min(float a, float b)
{
	return a < b ? a : b;
}

static inline float m_abs(float a)
{
	return a < 0.f ? -a : a;
}

static inline ImVec4 u32_to_vec4(ImU32 color)
{
	return (ImVec4){
		(float)((color >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f,
		(float)((color >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f,
		(float)((color >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f,
		(float)((color >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f};
}

// Open the URL in `msg` in a new tab.
//
EM_JS(void, open_in_new_tab, (const char *msg), {
	window.open(UTF8ToString(msg), "mozillaTab");
});

// SOKOL

static struct
{
	sg_pass_action pass_action;
} state;

static void init(void)
{
	sg_setup(&(sg_desc){
		.context = sapp_sgcontext(),
		.logger.func = slog_func,
	});
	simgui_setup(&(simgui_desc_t){0});

	// initial clear color
	state.pass_action = (sg_pass_action){
		.colors[0] = {.action = SG_ACTION_CLEAR, .value = {0.0f, 0.0f, 0.0f, 1.0f}}};
}

static void frame(void);

static void cleanup(void)
{
	simgui_shutdown();
	sg_shutdown();
}

static void event(const sapp_event *ev)
{
	simgui_handle_event(ev);
}

sapp_desc sokol_main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;
	return (sapp_desc){
		.init_cb = init,
		.frame_cb = frame,
		.cleanup_cb = cleanup,
		.event_cb = event,
		.window_title = "Hello Sokol + Dear ImGui",
		.width = 800,
		.height = 600,
		.logger.func = slog_func,
	};
}

#define FRAME_PASS_BEGIN \
	simgui_new_frame(&(simgui_frame_desc_t){ \
		.width = sapp_width(), \
		.height = sapp_height(), \
		.delta_time = sapp_frame_duration(), \
		.dpi_scale = sapp_dpi_scale(), \
	});

#define FRAME_PASS_END \
	sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height()); \
	simgui_render(); \
	sg_end_pass(); \
	sg_commit();

static void ABOUT_WIDGET() {
	ImGuiWindowFlags pinned_window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImVec2 window_pos;
	const ImGuiViewport* viewport = igGetMainViewport();
	const float PAD = 10.0f;
	// Bottom Left!
	window_pos.x = viewport->WorkPos.x + PAD;
	window_pos.y = viewport->WorkPos.y + viewport->WorkSize.y - PAD;
	igSetNextWindowPos(window_pos, ImGuiCond_Always, (ImVec2){0.f, 1.f});
	igBegin("About", 0, pinned_window_flags);
	{
		igText("Copyright (C) 2022 l-m.dev.");
		igText("Software is held according to the MIT open source license.");
		igSeparator();
		igText("Created at ");
		igSameLine(0, 0);
		if (igButton("https://l-m.dev", (ImVec2){0.f, 0.f})) {
			open_in_new_tab("https://l-m.dev");
		}
		igSameLine(0, 0);
		igText(" in C, using Emscripten and Dear ImGui.");
		igSeparator();
		igText("Full source code resides at ");
		igSameLine(0, 0);
		if (igButton("l1mey112/yr11-physics-applied", (ImVec2){0.f, 0.f})) {
			open_in_new_tab("https://github.com/l1mey112/yr11-physics-applied");
		}
	}
	igEnd();
}

static void RENDER_GRID(ImVec2 world_center) {
	ImGuiIO *io = igGetIO();
	ImDrawList *dl = igGetBackgroundDrawList_Nil();

	ImVec2 canvas_sz = io->DisplaySize;
	ImVec2 canvas_p0 = {0.f, 0.f};
	ImVec2 canvas_p1 = {canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y};

	ImDrawList_AddRectFilled(dl, canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255), 0.f, 0);
	
	const float GRID_STEP = 100.0f;
	for (float x = fmodf(world_center.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
	{
		if (x == world_center.x)
			ImDrawList_AddLine(dl, (ImVec2){canvas_p0.x + x, canvas_p0.y}, (ImVec2){canvas_p0.x + x, canvas_p1.y}, IM_COL32(255, 255, 255, 100), 1.0f);
		else
			ImDrawList_AddLine(dl, (ImVec2){canvas_p0.x + x, canvas_p0.y}, (ImVec2){canvas_p0.x + x, canvas_p1.y}, IM_COL32(200, 200, 200, 40), 1.0f);
	}
	for (float y = fmodf(world_center.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
	{
		if (y == world_center.y)
			ImDrawList_AddLine(dl, (ImVec2){canvas_p0.x, canvas_p0.y + y}, (ImVec2){canvas_p1.x, canvas_p0.y + y}, IM_COL32(255, 255, 255, 100), 1.0f);
		else
			ImDrawList_AddLine(dl, (ImVec2){canvas_p0.x, canvas_p0.y + y}, (ImVec2){canvas_p1.x, canvas_p0.y + y}, IM_COL32(200, 200, 200, 40), 1.0f);
	}
}

static ImVec2 HANDLE_PAN() {
	ImGuiIO *io = igGetIO();
	ImDrawList *dl = igGetBackgroundDrawList_Nil();

	ImVec2 canvas_sz = io->DisplaySize;

	static ImVec2 delta_scroll = {0.f, 0.f};
	if (igIsMouseDragging(ImGuiMouseButton_Right, 0.f))
	{
		delta_scroll.x += io->MouseDelta.x;
		delta_scroll.y += io->MouseDelta.y;
	}
	ImVec2 world_center = {delta_scroll.x + canvas_sz.x / 2.0f, delta_scroll.y + canvas_sz.y / 2.0f};

	return world_center;
}

#endif // DEMOS_H