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

static bool show_about;
static float g;
static float m;

LOCAL_STORAGE_INIT(bool, show_about, true);
LOCAL_STORAGE_INIT(float, g, 9.8);
LOCAL_STORAGE_INIT(float, m, 100.0);

static void init2(void)
{
	show_about = LOCAL_STORAGE_GET(show_about);
	g = LOCAL_STORAGE_GET(g);
	m = LOCAL_STORAGE_GET(m);
}

typedef struct Square Square;

struct Square
{
	ImVec2 pos;
	float radius;
};

#define DEFAULT_HEIGHT 550.f
#define DEFAULT_TIMER 0.5f

#define SQUARE_DEFAULT                 \
	(Square)                           \
	{                                  \
		.pos = {.x = -50.f, .y = DEFAULT_HEIGHT}, \
		.radius = 50.f,                \
	}

static void frame(void)
{
	FRAME_PASS_BEGIN;
	MOVE_UP_Y(3.f);

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igTextWrapped("Welcome to the Sinusoidal Wave Simulation!");
		igTextWrapped("This simulation allows you to visualise the transfer of energy as an object falls under the influence of gravity.");
		igSeparator();
		igTextWrapped("It allows you to control the initial position, mass, and gravity strength of an object.");
		igTextWrapped("By clicking and dragging at the top point, you can set the starting position.");
		igSeparator();
		igTextWrapped("As the object falls, you can observe the transfer of potential energy to kinetic energy. Adjusting the mass and gravity helps you explore different energy scenarios.");
		igSeparator();
		if (igCheckbox("Show About", &show_about))
			LOCAL_STORAGE_SET(show_about, show_about);
	}
	igEnd();

	ImVec2 wc = HANDLE_PAN();
	RENDER_GRID(wc);

	const float phys_dt = 1.f / 60.f;
	static float acc = 0.f;
	static Square square = SQUARE_DEFAULT;
	static float ypos = DEFAULT_HEIGHT;
	static float yvel = 0.f;
	static float yvel_real = 0.f;
	static float yfall_timer = 0.f;

	acc += __io->DeltaTime;
	while (acc >= phys_dt)
	{
		if (yfall_timer > 0.f) {
			yfall_timer -= phys_dt;

			if (yfall_timer <= 0.f) ypos = square.pos.y;
		} else if (ypos > square.radius) {
			yvel += g * 25 * phys_dt;
			yvel_real += g * phys_dt;
			ypos -= yvel * phys_dt;

			if (ypos < square.radius) {
				yvel = 0.f;
				yvel_real = 0.f;
				ypos = square.radius;
				yfall_timer = 2.f;
			}
		}
		acc -= phys_dt;
	}

	float yproject = square.radius;

	static bool is_hitting = false;
	bool is_hovering = false;
	if (!is_hitting && !__io->WantCaptureMouse)
	{
		ImVec2 rc = m_rct(wc, square.pos);

		if (igIsMouseHoveringRect(m_offset(rc, -square.radius, -square.radius), m_offset(rc, square.radius, square.radius), 0.f))
		{
			is_hitting = true;
		}
	}

	if (igIsMouseDragging(ImGuiMouseButton_Left, 0.f))
	{
		if (is_hitting)
		{
			square.pos = m_ract(wc, __io->MousePos);
			if (square.pos.y < square.radius)
				square.pos.y = square.radius;

			yfall_timer = 0.f;
			ypos = square.pos.y;
			yvel = 0.f;
			yvel_real = 0.f;
		}
	}
	else
	{
		if (is_hitting)
		{
			is_hovering = true;
		}
		is_hitting = false;
	}

	static char buf[128];

	static float ticks[256];
	static int ticks_cycle = 0;

	float total_gpot = m * g * (square.pos.y - square.radius);
	float h = ypos - square.radius;
	float gpot = m * g * h;

	ticks[ticks_cycle] = gpot;
	ticks_cycle = (ticks_cycle + 1) % IM_ARRAYSIZE(ticks);

	ImVec2 avail;
	float isp = igGetStyle()->ItemInnerSpacing.x;
	igGetContentRegionAvail(&avail);

	igSetNextWindowPos((ImVec2){__io->DisplaySize.x / 2.0f + 100.f, __io->DisplaySize.y / 2.0f}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Control Window", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		if (igSliderFloat("Mass (m)", &m, 10.f, 1000.f, "%g kg", ImGuiSliderFlags_AlwaysClamp))
			LOCAL_STORAGE_SET(m, m);
		if (igSliderFloat("Gravity (g)", &g, 0.05f, 100.f, "%g m/s^2", ImGuiSliderFlags_AlwaysClamp))
			LOCAL_STORAGE_SET(g, g);
	}
	igSeparator();
	{
		nice_box(FORMAT(buf, "m: %5.4g", m), IM_COL32(0, 0, 255, 255));
		igSameLine(.0f, isp);
		igText("*");
		igSameLine(.0f, isp);
		nice_box(FORMAT(buf, "g: %4.3g", g), IM_COL32(100, 160, 80, 255));
		igSameLine(.0f, isp);
		igText("*");
		igSameLine(.0f, isp);
		nice_box(FORMAT(buf, "h: %4.3g", h), IM_COL32(209,115,185, 255));
		igSameLine(.0f, isp);
		igText("=");
		igSameLine(.0f, isp);
		nice_box(FORMAT(buf, "U: %8dJ", (int)gpot), IM_COL32(140, 0, 130, 255));
	}
	igSeparator();
	{
		igPlotLines_FloatPtr(NULL, ticks, IM_ARRAYSIZE(ticks), ticks_cycle, FORMAT(buf, "U: %dJ", (int)gpot), __FLT_MIN__, __FLT_MAX__, (ImVec2){avail.x, 80.f}, sizeof(float));
	}
	igEnd();

	ImU32 col = is_hovering	 ? IM_COL32(255, 255, 255, 80)
				: is_hitting ? IM_COL32(255, 0, 0, 100)
							 : IM_COL32(231, 184, 63, 255);

	// ImDrawList_AddRectFilledMultiColor()

	// igText("total gpot: %g", );
	// igText("current gpot: %g", );

	float offset_x = square.pos.x + square.radius + 10.f;
	float offset_x2 = square.pos.x - square.radius - 50.f;

	// ImDrawList_AddRectFilled(__dl, m_rct(wc, Vec2(offset_x, square.pos.y)), m_rct(wc, Vec2(offset_x + 10.f, ypos)), IM_COL32(255, 133, 0, 255), 0.f, 0);
	// ImDrawList_AddRectFilled(__dl, m_rct(wc, Vec2(offset_x, yproject)), m_rct(wc, Vec2(offset_x + 10.f, ypos)), IM_COL32(140, 0, 130, 255), 0.f, 0);

	float total_gpot_r = m * g * (square.pos.y - square.radius);
	float gpot_r = m * g * (ypos - square.radius);

	float ygpot_max = square.pos.y + square.radius;
	float ygpot = (gpot_r / total_gpot_r) * ygpot_max;
	ImDrawList_AddRectFilled(__dl, m_rct(wc, Vec2(offset_x2, 0.f)), m_rct(wc, Vec2(offset_x2 - 10.f, ygpot)), IM_COL32(140, 0, 130, 255), 0.f, 0);
	ImDrawList_AddRectFilled(__dl, m_rct(wc, Vec2(offset_x2 - 20.f, 0.f)), m_rct(wc, Vec2(offset_x2 - 30.f, ygpot_max - ygpot)), IM_COL32(255, 133, 0, 255), 0.f, 0);

	ImDrawList_AddText_Vec2(__dl, m_rct(wc, Vec2(offset_x + 20.f, ygpot_max - ygpot + 15.f)), IM_COL32_WHITE, FORMAT(buf, "K = %gJ", total_gpot - gpot), NULL);
	ImDrawList_AddText_Vec2(__dl, m_rct(wc, Vec2(offset_x + 20.f, ygpot + 15.f)), IM_COL32_WHITE, FORMAT(buf, "U = %gJ", gpot), NULL);

	ImDrawList_AddRectFilled(__dl, m_rct(wc, Vec2(square.pos.x + square.radius, ypos + square.radius)), m_rct(wc, Vec2(square.pos.x - square.radius, ypos - square.radius)), col, 0.f, 0);
	ImDrawList_AddRect(__dl, m_rct(wc, m_offset(square.pos, square.radius, square.radius)), m_rct(wc, m_offset(square.pos, -square.radius, -square.radius)), IM_COL32(255, 255, 255, 255), 0.f, 0, 0.f);
	ImDrawList_AddRect(__dl, m_rct(wc, Vec2(square.pos.x + square.radius, yproject + square.radius)), m_rct(wc, Vec2(square.pos.x - square.radius, yproject - square.radius)), IM_COL32(255, 255, 255, 255), 0.f, 0, 0.f);

	ImDrawList_AddCircleFilled(__dl, m_rct(wc, square.pos), 4.f, IM_COL32(255, 255, 255, 255), 0);
	ImDrawList_AddCircleFilled(__dl, m_rct(wc, Vec2(square.pos.x, yproject)), 4.f, IM_COL32(255, 255, 255, 255), 0);
	ImDrawList_AddLine(__dl, m_rct(wc, square.pos), m_rct(wc, Vec2(square.pos.x, yproject)), IM_COL32(255, 255, 255, 255), 0.f);

	if (show_about)
	{
		ABOUT_WIDGET();
	}

	FRAME_PASS_END;
}
