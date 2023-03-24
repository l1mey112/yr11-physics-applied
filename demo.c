//------------------------------------------------------------------------------
//  Simple C99 cimgui+sokol starter project for Win32, Linux and macOS.
//------------------------------------------------------------------------------
#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_log.h"
#include "sokol_glue.h"
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"
#include "sokol_imgui.h"
#include "HandmadeMath.h"
#include <math.h>

#define DEG_TO_RAD (M_PI / 180.f)
#define RAD_TO_DEG (180.f / M_PI)

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

typedef struct
{
	ImVec2 pos;
	ImU32 col;
} Point;

#define POINTS_CAP 20
#define POINTS_RAD 10.f
static Point g_points[POINTS_CAP] = {
	(Point){.pos = {-80.0f, -100.0f}, .col = IM_COL32(100, 100, 225, 255)},
	(Point){.pos = {200.0f, 100.0f}, .col = IM_COL32(225, 100, 100, 255)},
	(Point){.pos = {-120.0f, 200.0f}, .col = IM_COL32(100, 225, 100, 255)},
	(Point){.pos = {180.0f, 320.0f}, .col = IM_COL32(236, 219, 87, 255)},
};
static int g_points_len = 4;

#define TO_REAL_COORDS(vec) \
	(ImVec2) { vec.x + world_center.x, -vec.y + world_center.y }
#define TO_RELA_COORDS(vec) \
	(ImVec2) { vec.x - world_center.x, -(vec.y - world_center.y) }

#define V2 \
	(a, b)(ImVec2) { a, b }
#define V2ADD(a, b) \
	(ImVec2) { a.x + b.x, a.y + b.y }
#define V2SUB(a, b) \
	(ImVec2) { a.x - b.x, a.y - b.y }

static ImVec2 rotate(ImVec2 v, float theta)
{
	return (ImVec2){v.x * cosf(theta) - v.y * sinf(theta), v.x * sinf(theta) + v.y * cosf(theta)};
}

static void tip_to_tail_pairs(ImDrawList *dl, const ImVec2 world_center, const Point a, const Point b)
{
	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(b.pos), IM_COL32(255, 50, 50, 255), 2.f);

	ImVec2 deg = {a.pos.x, b.pos.y};
	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(deg), IM_COL32(255, 255, 255, 70), 1.5f);
	ImDrawList_AddLine(dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(deg), IM_COL32(255, 255, 255, 70), 1.5f);

	float dx = a.pos.x - b.pos.x;
	float dy = a.pos.y - b.pos.y;
	float a1 = atan2f(dy, -dx);
	float r_a1 = atan2f(dy, dx);

	float a0 = M_PI / 2.f;
	if (a1 < 0.f)
		a0 = -a0;

	float arc_rad = (dy < 0.f ? -dy : dy) * 0.4f;
	if (arc_rad > 40.f)
		arc_rad = 40.f;

	ImDrawList_PathArcTo(dl, TO_REAL_COORDS(a.pos), arc_rad, a0, a1, 0);
	ImDrawList_PathStroke(dl, IM_COL32(255, 255, 255, 70), ImDrawFlags_None, 1.5f);

	/* ImVec2 arrw0 = rotate((ImVec2){30.f, 0.f}, r_a1 + 45.f * DEG_TO_RAD);
	ImVec2 arrw1 = rotate((ImVec2){30.f, 0.f}, r_a1 - 45.f * DEG_TO_RAD);
	arrw0 = (ImVec2){a.pos.x + arrw0.x, a.pos.y + arrw0.y};
	arrw1 = (ImVec2){a.pos.x + arrw1.x, a.pos.y + arrw1.y};

	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(arrw0), IM_COL32(255, 50, 50, 120), 1.5f);
	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(arrw1), IM_COL32(255, 50, 50, 120), 1.5f); */

	ImVec2 arrw3 = rotate((ImVec2){30.f, 0.f}, r_a1 + 45.f * DEG_TO_RAD);
	ImVec2 arrw4 = rotate((ImVec2){30.f, 0.f}, r_a1 - 45.f * DEG_TO_RAD);
	arrw3 = (ImVec2){b.pos.x + arrw3.x, b.pos.y + arrw3.y};
	arrw4 = (ImVec2){b.pos.x + arrw4.x, b.pos.y + arrw4.y};

	ImDrawList_AddLine(dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(arrw3), IM_COL32(255, 50, 50, 180), 1.5f);
	ImDrawList_AddLine(dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(arrw4), IM_COL32(255, 50, 50, 180), 1.5f);
}

static void frame(void)
{
	simgui_new_frame(&(simgui_frame_desc_t){
		.width = sapp_width(),
		.height = sapp_height(),
		.delta_time = sapp_frame_duration(),
		.dpi_scale = sapp_dpi_scale(),
	});

	ImGuiIO *io = igGetIO();

	static ImVec4 colf = {0};

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400, 100}, ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_None);
	igColorEdit3("Background", &state.pass_action.colors[0].value.r, ImGuiColorEditFlags_None);
	igEnd();

	ImDrawList *dl = igGetBackgroundDrawList_Nil();

	ImVec2 canvas_sz = io->DisplaySize;
	ImVec2 canvas_p0 = {0.f, 0.f};
	ImVec2 canvas_p1 = {canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y};

	static ImVec2 delta_scroll = {0.f, 0.f};
	ImDrawList_AddRectFilled(dl, canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255), 0.f, 0);
	if (igIsMouseDragging(ImGuiMouseButton_Right, 0.f))
	{
		delta_scroll.x += io->MouseDelta.x;
		delta_scroll.y += io->MouseDelta.y;
	}
	ImVec2 world_center = {delta_scroll.x + canvas_sz.x / 2.0f, delta_scroll.y + canvas_sz.y / 2.0f};
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

	static int is_hitting = -1;

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

	if (igIsMouseDragging(ImGuiMouseButton_Left, 0.f))
	{
		if (is_hitting != -1)
			g_points[is_hitting].pos = TO_RELA_COORDS(io->MousePos);
	}
	else
	{
		is_hitting = -1;
	}

	for (int idx = 0; idx < g_points_len; idx++)
	{
		Point point = g_points[idx];
		if (is_hitting == idx)
			ImDrawList_AddCircleFilled(dl, TO_REAL_COORDS(point.pos), POINTS_RAD, IM_COL32(255, 255, 255, 40), 0);
		else
			ImDrawList_AddCircleFilled(dl, TO_REAL_COORDS(point.pos), POINTS_RAD, point.col, 0);
	}

	Point last_point = g_points[0];
	for (int idx = 1; idx < g_points_len; idx++)
	{
		tip_to_tail_pairs(dl, world_center, last_point, g_points[idx]);
		last_point = g_points[idx];
	}

	sg_begin_default_pass(&state.pass_action, sapp_width(), sapp_height());
	simgui_render();
	sg_end_pass();
	sg_commit();
}

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
