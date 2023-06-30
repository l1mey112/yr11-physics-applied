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

#include <math.h>
#include <stdbool.h>
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_imgui.h"
#include "HandmadeMath.h"
#include <emscripten.h>

static inline ImVec2 Vec2(float x, float y)
{
	return (ImVec2){x, y};
}

#define M_TAU (2 * M_PI)
#define DEG_TO_RAD (M_PI / 180.f)
#define RAD_TO_DEG (180.f / M_PI)

#define V2ZERO   \
	(ImVec2)     \
	{            \
		0.f, 0.f \
	}

#define F_COLOUR IM_COL32(240, 100, 100, 255)
#define F_META_COLOUR IM_COL32(100, 120, 255, 255)

#define TO_REAL_COORDS(vec) \
	m_vadd(m_real_coords(vec), world_center)
#define TO_RELA_COORDS(vec)                               \
	(ImVec2)                                              \
	{                                                     \
		vec.x - world_center.x, -(vec.y - world_center.y) \
	}

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

static inline ImVec2 m_vdiv(ImVec2 a, ImVec2 b)
{
	return (ImVec2){a.x / b.x, a.y / b.y};
}

static inline ImVec2 m_vdivs(ImVec2 a, float b)
{
	return (ImVec2){a.x / b, a.y / b};
}

static inline ImVec2 m_vflip(ImVec2 a)
{
	return (ImVec2){-a.x, -a.y};
}

static inline ImVec2 m_offset(ImVec2 a, float x, float y)
{
	return (ImVec2){a.x + x, a.y + y};
}

static inline float m_distance(ImVec2 a, ImVec2 b)
{
	float dx = a.x - b.x;
	float dy = a.y - b.y;
	return sqrtf(dx * dx + dy * dy);
}

static inline ImVec2 m_normalise(ImVec2 a)
{
	float length = a.x * a.x + a.y * a.y;
	if (length != 0.0f)
	{
		float inv = 1.0f / sqrtf(length);
		a.x *= inv;
		a.y *= inv;
	}
	return a;
}

static inline float m_vdot(ImVec2 a, ImVec2 b)
{
	return a.x * b.x + a.y * b.y;
}

static inline ImVec2 m_vmidpoint(ImVec2 a, ImVec2 b)
{
	return (ImVec2){
		(a.x + b.x) / 2.f,
		(a.y + b.y) / 2.f,
	};
}

static inline ImVec2 m_rct(ImVec2 world_center, ImVec2 a)
{
	return m_vadd(m_real_coords(a), world_center);
}

static inline ImVec2 m_ract(ImVec2 world_center, ImVec2 vec)
{
	return (ImVec2){vec.x - world_center.x, -(vec.y - world_center.y)};
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

#define HRZ_VEC  \
	(ImVec2)     \
	{            \
		1.f, 0.f \
	}

static inline ImVec4 u32_to_vec4(ImU32 color)
{
	return (ImVec4){
		(float)((color >> IM_COL32_R_SHIFT) & 0xFF) / 255.0f,
		(float)((color >> IM_COL32_G_SHIFT) & 0xFF) / 255.0f,
		(float)((color >> IM_COL32_B_SHIFT) & 0xFF) / 255.0f,
		(float)((color >> IM_COL32_A_SHIFT) & 0xFF) / 255.0f};
}

static ImU32 lerp_color(ImU32 color1, ImU32 color2, float t)
{
	ImVec4 col1 = u32_to_vec4(color1);
	ImVec4 col2 = u32_to_vec4(color2);

	float r = col1.x + (col2.x - col1.x) * t;
	float g = col1.y + (col2.y - col1.y) * t;
	float b = col1.z + (col2.z - col1.z) * t;
	float a = col1.w + (col2.w - col1.w) * t;

	ImU32 out = 0;

	out |= (unsigned char)(r * 255.0f) << IM_COL32_R_SHIFT;
	out |= (unsigned char)(g * 255.0f) << IM_COL32_G_SHIFT;
	out |= (unsigned char)(b * 255.0f) << IM_COL32_B_SHIFT;
	out |= (unsigned char)(a * 255.0f) << IM_COL32_A_SHIFT;

	return out;
}

// Open the URL in `msg` in a new tab.
//
EM_JS(void, open_in_new_tab, (const char *msg), {
	window.open(UTF8ToString(msg), "mozillaTab");
});

// Is this page embedded inside an `iframe`?
//
EM_JS(bool, is_inside_iframe, (), {
	return window.location != window.parent.location;
});

// SOKOL

static ImDrawList *__dl;
static ImGuiIO *__io;

#ifdef USE_INIT2
static void init2(void);
#endif // USE_INIT2

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

#ifdef USE_INIT2
	init2();
#endif // USE_INIT2
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

#define FRAME_PASS_BEGIN                         \
	do                                           \
	{                                            \
		simgui_new_frame(&(simgui_frame_desc_t){ \
			.width = sapp_width(),               \
			.height = sapp_height(),             \
			.delta_time = sapp_frame_duration(), \
			.dpi_scale = sapp_dpi_scale(),       \
		});                                      \
		__dl = igGetBackgroundDrawList_Nil();    \
		__io = igGetIO();                        \
	} while (0)

#define FRAME_PASS_END                                                          \
	do                                                                          \
	{                                                                           \
		sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height()); \
		simgui_render();                                                        \
		sg_end_pass();                                                          \
		sg_commit();                                                            \
	} while (0)

static void ABOUT_WIDGET()
{
	if (is_inside_iframe())
		return;

	ImGuiWindowFlags pinned_window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImVec2 window_pos;
	const ImGuiViewport *viewport = igGetMainViewport();
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
		if (igButton("https://l-m.dev", (ImVec2){0.f, 0.f}))
		{
			open_in_new_tab("https://l-m.dev");
		}
		igSameLine(0, 0);
		igText(" in C, using Emscripten and Dear ImGui.");
		igSeparator();
		igText("Full source code resides at ");
		igSameLine(0, 0);
		if (igButton("l1mey112/yr11-physics-applied", (ImVec2){0.f, 0.f}))
		{
			open_in_new_tab("https://github.com/l1mey112/yr11-physics-applied");
		}
	}
	igEnd();
}

static void BLIT_BG(ImU32 col)
{
	ImVec2 canvas_sz = __io->DisplaySize;
	ImVec2 canvas_p0 = {0.f, 0.f};
	ImVec2 canvas_p1 = {canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y};

	ImDrawList_AddRectFilled(__dl, canvas_p0, canvas_p1, col, 0.f, 0);
}

static void RENDER_GRID(ImVec2 world_center)
{
	ImVec2 canvas_sz = __io->DisplaySize;
	ImVec2 canvas_p0 = {0.f, 0.f};
	ImVec2 canvas_p1 = {canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y};

	BLIT_BG(IM_COL32(50, 50, 50, 255));

	const float GRID_STEP = 100.0f;
	for (float x = fmodf(world_center.x, GRID_STEP); x < canvas_sz.x; x += GRID_STEP)
	{
		if (x == world_center.x)
			ImDrawList_AddLine(__dl, (ImVec2){canvas_p0.x + x, canvas_p0.y}, (ImVec2){canvas_p0.x + x, canvas_p1.y}, IM_COL32(255, 255, 255, 100), 1.0f);
		else
			ImDrawList_AddLine(__dl, (ImVec2){canvas_p0.x + x, canvas_p0.y}, (ImVec2){canvas_p0.x + x, canvas_p1.y}, IM_COL32(200, 200, 200, 40), 1.0f);
	}
	for (float y = fmodf(world_center.y, GRID_STEP); y < canvas_sz.y; y += GRID_STEP)
	{
		if (y == world_center.y)
			ImDrawList_AddLine(__dl, (ImVec2){canvas_p0.x, canvas_p0.y + y}, (ImVec2){canvas_p1.x, canvas_p0.y + y}, IM_COL32(255, 255, 255, 100), 1.0f);
		else
			ImDrawList_AddLine(__dl, (ImVec2){canvas_p0.x, canvas_p0.y + y}, (ImVec2){canvas_p1.x, canvas_p0.y + y}, IM_COL32(200, 200, 200, 40), 1.0f);
	}
}

static ImVec2 __delta_scroll = {0.f, 0.f};

EM_JS(float, touch_get_x, (), {
	let dx = window.__t_cx - window.__t_px;
	window.__t_px = window.__t_cx;
	return dx;
});

EM_JS(float, touch_get_y, (), {
	let dy = window.__t_cy - window.__t_py;
	window.__t_py = window.__t_cy;
	return dy;
});

static ImVec2 HANDLE_PAN()
{
	ImVec2 canvas_sz = __io->DisplaySize;

	if (igIsMouseDragging(ImGuiMouseButton_Right, 0.f) || igIsMouseDragging(ImGuiMouseButton_Middle, 0.f))
	{
#ifndef HANDLE_PAN_NO_X
		__delta_scroll.x += __io->MouseDelta.x;
#endif
#ifndef HANDLE_PAN_NO_Y
		__delta_scroll.y += __io->MouseDelta.y;
#endif
	} else {
		__delta_scroll.x += touch_get_x(); // handle mobile
		__delta_scroll.y += touch_get_y(); // handle mobile
	}
	ImVec2 world_center = {__delta_scroll.x + canvas_sz.x / 2.0f, __delta_scroll.y + canvas_sz.y / 2.0f};

	return world_center;
}

static inline ImVec2 DELTA_SCROLL()
{
	return __delta_scroll;
}

#define MOVE_UP_Y(v) \
	do { \
		static bool __run_once = true; \
		if (__run_once) { \
			__run_once = false; \
			__delta_scroll.y += __io->DisplaySize.y / v; \
		} \
	} while(0)


static void arrow(ImVec2 start, ImVec2 end, ImU32 color, float thickness, float sz)
{
	ImDrawList_AddLine(__dl, start, end, color, thickness);

	ImVec2 dir = {end.x - start.x, end.y - start.y};
	float length = sqrtf(dir.x * dir.x + dir.y * dir.y);
	ImVec2 norm_dir = {dir.x / length, dir.y / length};

	ImVec2 p1 = {end.x - norm_dir.x * sz - norm_dir.y * sz * 0.6,
				 end.y - norm_dir.y * sz + norm_dir.x * sz * 0.6};
	ImVec2 p2 = {end.x - norm_dir.x * sz + norm_dir.y * sz * 0.6,
				 end.y - norm_dir.y * sz - norm_dir.x * sz * 0.6};

	ImDrawList_AddTriangleFilled(__dl, end, p1, p2, color);
}

void nice_box(const char *label, ImU32 col)
{
	ImGuiStyle *style = &GImGui->Style;
	ImVec2 label_size;

	igCalcTextSize(&label_size, label, NULL, true, -1.0f);

	ImVec2 pos = igGetCurrentWindow()->DC.CursorPos;
	ImVec2 size = {label_size.x + style->FramePadding.x * 2.0f, label_size.y + style->FramePadding.y * 2.0f};

	const ImRect bb = {pos, (ImVec2){pos.x + size.x, pos.y + size.y}};
	igItemSize_Vec2(size, style->FramePadding.y);

	igRenderFrame(bb.Min, bb.Max, col, true, style->FrameRounding);

	ImVec2 pmin = {bb.Min.x + style->FramePadding.x, bb.Min.y + style->FramePadding.y};
	ImVec2 pmax = {bb.Max.x - style->FramePadding.x, bb.Max.y - style->FramePadding.y};

	igRenderTextClipped(pmin, pmax, label, NULL, &label_size, style->ButtonTextAlign, &bb);
}

#define FORMAT(buffer, format, ...) \
	(snprintf((buffer), sizeof(buffer), (format), __VA_ARGS__), (buffer))

#define LOCAL_STORAGE_GET(name) ({ local_storage_##name(); })
#define LOCAL_STORAGE_SET(name, val) ({ local_storage_set_##name(val); })
#define LOCAL_STORAGE_INIT0(...) EM_JS(__VA_ARGS__)
#define LOCAL_STORAGE_INIT(type, name, _default)                      \
    LOCAL_STORAGE_INIT0(type, local_storage_##name, (), {             \
        let v;                                                        \
        if (v = window.sessionStorage.getItem(__FILE__ + "_" + #name))       \
            return v;                                                 \
        return _default;                                              \
    });                                                               \
    LOCAL_STORAGE_INIT0(void, local_storage_set_##name, (type val), { \
        window.sessionStorage.setItem(__FILE__ + "_" + #name, val)           \
    })

// TODO: impl later
//
// #define IMGUI_PERSISTENT_COLLAPSE_INIT(name) LOCAL_STORAGE_INIT(bool, name, false)
// #define IMGUI_PERSISTENT_COLLAPSE(name) igSetNextWindowCollapsed(LOCAL_STORAGE_GET(name) || is_inside_iframe(), ImGuiCond_Once)

#endif // DEMOS_H