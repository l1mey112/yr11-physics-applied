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

typedef struct Square Square;

struct Square
{
	ImVec2 pos;
	float radius;
};

#define SQUARE_DEFAULT                 \
	(Square)                           \
	{                                  \
		.pos = {.x = 0.f, .y = 250.f}, \
		.radius = 50.f,                \
	}

static void frame(void)
{
	FRAME_PASS_BEGIN;

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igCheckbox("Show About", &show_about);
	}
	igEnd();

	ImVec2 wc = HANDLE_PAN();
	RENDER_GRID(wc);

	const float phys_dt = 1.f / 60.f;
	static float acc = 0.f;
	static Square square = SQUARE_DEFAULT;
	static float ypos = 250.f;
	static float yvel = 0.f;

	if (ypos <= square.radius)
	{
		ypos = square.pos.y;
		yvel = 0.f;
	}
	else
	{
		acc += __io->DeltaTime;
		while (acc >= phys_dt)
		{
			yvel += 490 * phys_dt;
			ypos -= yvel * phys_dt;
			acc -= phys_dt;
		}
	}

	float yproject = square.radius;

	// float m = 100.f;
	// float square_pot = m * 9.8 * (square.pos.y - square.radius);

	// float height = square.pos.y - project.pos.y;

	// igText("pot: %d", (int)square_pot);

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

			ypos = square.pos.y;
			yvel = 0.f;
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

	ImU32 col = is_hovering ? IM_COL32(255, 255, 255, 80) : is_hitting ? IM_COL32(255, 0, 0, 100)
																	   : IM_COL32(231, 184, 63, 255);

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
