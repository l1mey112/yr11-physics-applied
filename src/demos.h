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

#endif // DEMOS_H