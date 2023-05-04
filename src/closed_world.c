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

Object *obj;

static void init2(void)
{
	world_add_object(V2ZERO, 100.f, 100.f);
	obj = &__world.objects[world_add_object(V2ZERO, 100.f, 100.f)];
	obj->pos.y = 350.f;
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

	igText("(%f %f)", obj->vel.x, obj->vel.y);

	BLIT_BG(IM_COL32(50, 50, 50, 255));
	//RENDER_GRID(wc);

	for (int i = 0; i < __world.obj_count; i++) {
		Object *obj = &__world.objects[i];
		ImDrawList_AddCircleFilled(__dl, m_rct(wc, obj->pos), obj->rad, IM_COL32(255, 255, 255, 255), 0);
	}

	//if (if __io->MouseClicked[0])

	static bool init_drag = false;
	static ImVec2 drag_start;
	static ImVec2 drag_delta;
	if (igIsMouseDragging(ImGuiMouseButton_Left, 0.f)) {
		if (!init_drag) {
			// DRAG START
			drag_start = __io->MouseClickedPos[ImGuiMouseButton_Left];
		}
		init_drag =  true;
		igGetMouseDragDelta(&drag_delta, ImGuiMouseButton_Left, 0.f);

		// DRAGGING
	} else if (init_drag) {
		init_drag = false;
		igText("DRAGGING: (%f, %f), delta (%f, %f)", drag_start.x, drag_start.y, drag_delta.x, drag_delta.y);
		// END DRAG		
	}

	if (show_about) {
		ABOUT_WIDGET();
	}	

	FRAME_PASS_END
}