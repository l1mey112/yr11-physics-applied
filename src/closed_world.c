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
#include "world.h"

static bool show_about = true;

// Object *obj;

static void init2(void)
{
	world_add_object(V2ZERO, 100.f, 100.f);
	/* obj = &__world.objects[world_add_object(V2ZERO, 100.f, 100.f)];
	obj->pos.y = 350.f; */
	//obj->vel.y = -100.f;
}

static void frame(void)
{
	FRAME_PASS_BEGIN

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	igText("%f", __io->DeltaTime);
	igCheckbox("Show About", &show_about);
	igEnd();

	ImVec2 wc = HANDLE_PAN();

	static float acc = 0.0f;
	const float phys_dt = 1.f / 50.f; // 50fps phys update

	acc += __io->DeltaTime;
	while (acc >= phys_dt) {
		world_integrate(phys_dt);
		acc -= phys_dt;
	}

	BLIT_BG(IM_COL32(50, 50, 50, 255));
	//RENDER_GRID(wc);

	//igShowMetricsWindow(false);
	//igShowDemoWindow(false);

	static bool init_drag = false;
	static ImVec2 drag_start;
	static ImVec2 drag_delta;
	static float sz = 80.f; // default
	if (igIsMouseDragging(ImGuiMouseButton_Left, 15.f) && !__io->WantCaptureMouse) {
		if (!init_drag) {
			// DRAG START
			drag_start = __io->MouseClickedPos[ImGuiMouseButton_Left];
		}
		init_drag =  true;
		igGetMouseDragDelta(&drag_delta, ImGuiMouseButton_Left, 0.f);
		// DRAGGING

		ImDrawList_AddCircle(__dl, drag_start, sz, IM_COL32(255, 255, 255, 80), 0, 0.f);
		ImDrawList_AddCircleFilled(__dl, drag_start, 8.f, F_COLOUR, 0);
		ImDrawList_AddLine(__dl, drag_start, m_vadd(drag_start, drag_delta), F_COLOUR, 0.f);
	} else if (init_drag) {
		init_drag = false;
		// END DRAG

		float dx = -drag_delta.x;
		float dy = drag_delta.y;

		int idx = world_add_object(m_ract(wc, drag_start), sz, sz);
		Object *obj = &__world.objects[idx];
		obj->vel.x = dx;
		obj->vel.y = dy;

		// float rad = sqrt(drag_delta.x * drag_delta.x + drag_delta.y * drag_delta.y);
		// ImDrawList_AddCircleFilled(__dl, drag_start, rad, IM_COL32(255, 255, 255, 255), 0);
	} else {
		ImDrawList_AddCircle(__dl, __io->MousePos, sz, IM_COL32(255, 255, 255, 80), 0, 0.f);
		ImDrawList_AddCircleFilled(__dl, __io->MousePos, 8.f, F_COLOUR, 0);

		if (igIsMouseReleased_Nil(ImGuiMouseButton_Left))
			world_add_object(m_ract(wc, __io->MousePos), sz, sz);
	}

	if (igIsKeyPressed_Bool(ImGuiKey_UpArrow, true))
			sz += 20.f;
	else if (igIsKeyPressed_Bool(ImGuiKey_DownArrow, true))
		sz -= 20.f;
	else if (__io->MouseWheel)
		sz += __io->MouseWheel;
	if (sz < 15.f) sz = 15.f;

	float mx = 0.f;
	float my = 0.f;
	float kx = 0.f;
	float ky = 0.f;
	for (int i = 0; i < __world.obj_count; i++) {
		Object* obj = &__world.objects[i];

		mx += obj->mass * obj->vel.x;
		my += obj->mass * obj->vel.y;
		kx += 0.5 * obj->mass * (obj->vel.x * obj->vel.x);
		ky += 0.5 * obj->mass * (obj->vel.y * obj->vel.y);
	}

	igText("(%f, %f), %f", mx, my, sqrt(mx * mx + my * my));
	igText("(%f, %f), %f", kx, ky, sqrt(kx * kx + ky * ky));

	for (int i = 0; i < __world.obj_count; i++) {
		Object *obj = &__world.objects[i];
		ImDrawList_AddCircleFilled(__dl, m_rct(wc, obj->pos), obj->rad, IM_COL32(255, 255, 255, 255), 0);
	}

	if (show_about) {
		ABOUT_WIDGET();
	}	

	FRAME_PASS_END
}