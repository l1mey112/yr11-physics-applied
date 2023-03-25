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
#include <emscripten.h>

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
	(Point){.pos = {-261.f, -149.f}, .col = IM_COL32(100, 100, 225, 255)},
	(Point){.pos = {-73.f, 76.f}, .col = IM_COL32(225, 100, 100, 255)},
	(Point){.pos = {56.f, -16.f}, .col = IM_COL32(100, 225, 100, 255)},
	(Point){.pos = {244.f, 234.f}, .col = IM_COL32(100, 225, 100, 255)},
};
static int g_points_len = 4;

#define TO_REAL_COORDS(vec) \
	m_vadd(m_real_coords(vec), world_center)
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

static bool show_math = false;
static bool show_about = true;

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

static void vector_final(ImDrawList *dl, const ImVec2 world_center)
{
	const Point a = g_points[0];
	const Point b = g_points[g_points_len - 1];

	ImVec2 deg = {b.pos.x, a.pos.y};
	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(deg), RESULTANT_VECTOR_COL, 1.5f);
	ImDrawList_AddLine(dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(deg), RESULTANT_VECTOR_COL, 1.5f);

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
		ImDrawList_AddText_Vec2(dl, TO_REAL_COORDS(t0), RESULTANT_VECTOR_COL_FULL, buf, NULL);
		snprintf(buf, sizeof(buf), "%g", dy);
		ImDrawList_AddText_Vec2(dl, TO_REAL_COORDS(t1), RESULTANT_VECTOR_COL_FULL, buf, NULL);
		snprintf(buf, sizeof(buf), "%g", hypot);
		ImDrawList_AddText_Vec2(dl, TO_REAL_COORDS(t2), RESULTANT_VECTOR_COL_FULL, buf, NULL);
	}

	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(b.pos), RESULTANT_VECTOR_COL, 2.f);
}

static void tip_to_tail_pairs(ImDrawList *dl, const ImVec2 world_center, const Point a, const Point b)
{
	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(b.pos), a.col, 2.f);

	ImVec2 deg = {b.pos.x, a.pos.y};
	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(deg), IM_COL32(255, 255, 255, 70), 1.5f);
	ImDrawList_AddLine(dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(deg), IM_COL32(255, 255, 255, 70), 1.5f);

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
		ImDrawList_AddText_Vec2(dl, TO_REAL_COORDS(t0), IM_COL32(255, 255, 255, 255), buf, NULL);
		snprintf(buf, sizeof(buf), "%g", dy);
		ImDrawList_AddText_Vec2(dl, TO_REAL_COORDS(t1), IM_COL32(255, 255, 255, 255), buf, NULL);
	}

	float a0 = 0;
	if (dx < 0.f)
		a0 = M_PI;
	if (dx < 0.f && dy > 0.f)
		a0 = -M_PI;

	float arc_rad = m_min(m_abs(dx), m_abs(dy)) * 0.4f;
	if (arc_rad > 40.f)
		arc_rad = 40.f;

	ImDrawList_PathArcTo(dl, TO_REAL_COORDS(a.pos), arc_rad, a0, a1, 0);
	ImDrawList_PathStroke(dl, IM_COL32(255, 255, 255, 70), ImDrawFlags_None, 1.5f);

	/* ImVec2 arrw0 = rotate((ImVec2){30.f, 0.f}, a1_r + 45.f * DEG_TO_RAD);
	ImVec2 arrw1 = rotate((ImVec2){30.f, 0.f}, a1_r - 45.f * DEG_TO_RAD);
	arrw0 = (ImVec2){a.pos.x + arrw0.x, a.pos.y + arrw0.y};
	arrw1 = (ImVec2){a.pos.x + arrw1.x, a.pos.y + arrw1.y};

	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(arrw0), IM_COL32(255, 50, 50, 120), 1.5f);
	ImDrawList_AddLine(dl, TO_REAL_COORDS(a.pos), TO_REAL_COORDS(arrw1), IM_COL32(255, 50, 50, 120), 1.5f); */

	ImVec2 arrw3 = rotate((ImVec2){40.f, 0.f}, a1_r + 225.f * DEG_TO_RAD);
	ImVec2 arrw4 = rotate((ImVec2){40.f, 0.f}, a1_r - 225.f * DEG_TO_RAD);
	arrw3 = (ImVec2){b.pos.x + arrw3.x, b.pos.y + arrw3.y};
	arrw4 = (ImVec2){b.pos.x + arrw4.x, b.pos.y + arrw4.y};

	ImDrawList_AddLine(dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(arrw3), a.col, 1.5f);
	ImDrawList_AddLine(dl, TO_REAL_COORDS(b.pos), TO_REAL_COORDS(arrw4), a.col, 1.5f);

	if (show_math) {
		ImVec2 theta_pos = rotate((ImVec2){40.f, 0.f}, a1);
		theta_pos = (ImVec2){a.pos.x + theta_pos.x, a.pos.y + theta_pos.y};

		char buf[50];
		snprintf(buf, sizeof(buf), "(%g, %g) N%g°E", a.pos.x, a.pos.y, norm_degrees(a1_r * RAD_TO_DEG));

		ImDrawList_AddText_Vec2(dl, TO_REAL_COORDS(theta_pos), IM_COL32(255, 255, 255, 255), buf, NULL);

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

static inline ImVec4 u32_to_vec4(ImU32 color)
{
	return (ImVec4){
		(float)((color >> 16) & 0xFF) / 255.0f,
		(float)((color >> 8) & 0xFF) / 255.0f,
		(float)(color & 0xFF) / 255.0f,
		(float)((color >> 24) & 0xFF) / 255.0f};
}

#define RECT_OFFSET 10.f

// Open the URL in `msg` in a new tab.
//
EM_JS(void, open_in_new_tab, (const char *msg), {
	window.open(UTF8ToString(msg), "mozillaTab");
});

static void frame(void)
{
	simgui_new_frame(&(simgui_frame_desc_t){
		.width = sapp_width(),
		.height = sapp_height(),
		.delta_time = sapp_frame_duration(),
		.dpi_scale = sapp_dpi_scale(),
	});

	ImGuiIO *io = igGetIO();

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
		if (is_hitting == idx || idx + 1 == g_points_len)
			ImDrawList_AddCircleFilled(dl, TO_REAL_COORDS(point.pos), POINTS_RAD, IM_COL32(255, 255, 255, 40), 0);
		else
			ImDrawList_AddCircleFilled(dl, TO_REAL_COORDS(point.pos), POINTS_RAD, point.col, 0);
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
				ImDrawList_AddRectFilled(dl, TO_REAL_COORDS(b0), TO_REAL_COORDS(b1), a.col, 0.f, 0);
			}
			
			{
				ImVec2 b0 = (ImVec2){rect_pos.x, a.pos.y};
				ImVec2 b1 = (ImVec2){rect_pos.x + RECT_OFFSET, b.pos.y};
				b0.x += RECT_OFFSET * 2.f * idx;
				b1.x += RECT_OFFSET * 2.f * idx;
				ImDrawList_AddRectFilled(dl, TO_REAL_COORDS(b0), TO_REAL_COORDS(b1), a.col, 0.f, 0);
			}
			
			/* ImVec2 b0 = (ImVec2){a.pos.x, rect_pos.y};
			ImVec2 b1 = (ImVec2){a.pos.x, rect_pos.y + RECT_OFFSET};
			ImVec2 b2 = (ImVec2){b.pos.x, rect_pos.y + RECT_OFFSET / 2.f};
			b0.y += RECT_OFFSET * 2.f * idx;
			b1.y += RECT_OFFSET * 2.f * idx;
			b2.y += RECT_OFFSET * 2.f * idx;
			// ImDrawList_AddRectFilled(dl, TO_REAL_COORDS(b0), TO_REAL_COORDS(b1), a.col, 0.f, 0);
			ImDrawList_AddTriangleFilled(dl, TO_REAL_COORDS(b0), TO_REAL_COORDS(b1), TO_REAL_COORDS(b2), a.col); */
			
			tip_to_tail_pairs(dl, world_center, a, b);
			a = b;
		}
	}

	vector_final(dl, world_center);

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	igPushStyleColor_U32(ImGuiCol_Text, IM_COL32(225, 225, 225, 255));
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
	igPopStyleColor(1);
	igPushStyleColor_U32(ImGuiCol_Text, RESULTANT_VECTOR_COL_FULL);
	{
		igTextWrapped("The resultant vector is in this special colour.");
	}
	igPopStyleColor(1);
	igSeparator();
	igCheckbox("Show Math Overlay", &show_math);
	igCheckbox("Show About", &show_about);
	igSeparator();
	{
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
	}
	igEnd();

	if (show_about) {
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
			/* if (igButton("https://l-m.dev", (ImVec2){0.f, 0.f})) {
				open_in_new_tab("https://l-m.dev");
			} */
			
			igText("Created at ");
			igSameLine(0, 0);
			if (igButton("https://l-m.dev", (ImVec2){0.f, 0.f})) {
				open_in_new_tab("https://l-m.dev");
			}
			igSameLine(0, 0);
			igText(" in C, using Emscripten and Dear ImGui.");
		}
		igEnd();
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
