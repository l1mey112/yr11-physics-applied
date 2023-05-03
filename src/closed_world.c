#include "demos.h"

static bool show_about = true;

static void frame(void)
{
	FRAME_PASS_BEGIN

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	igCheckbox("Show About", &show_about);
	igEnd();

	ImVec2 wc = HANDLE_PAN();

	//
	BLIT_BG(IM_COL32(50, 50, 50, 255));

	//RENDER_GRID(wc);

	if (show_about) {
		ABOUT_WIDGET();
	}	

	FRAME_PASS_END
}