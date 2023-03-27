#define HANDLE_PAN_NO_X
#include "demos.h"

// FRICTION IS NOT TAKEN INTO ACCOUNT

#define OBJ_HALF_X 100.f
#define OBJ_HALF_Y 50.f
#define OBJ_HALF_WIDTHS \
	(ImVec2) { OBJ_HALF_X, OBJ_HALF_Y }

/* static void object_show(ImVec2 wc, Object *obj)
{
	ImVec2 rmin = m_vadd(obj->pos, OBJ_HALF_WIDTHS);
	ImVec2 rmax = m_vsub(obj->pos, OBJ_HALF_WIDTHS);

	ImDrawList_AddRectFilled(dl, m_rct(wc, rmin), m_rct(wc, rmax), IM_COL32(231,184,63, 255), 0.f, 0);
} */

static void frame(void)
{
	FRAME_PASS_BEGIN

	ImGuiIO *io = igGetIO();
	ImDrawList *dl = igGetBackgroundDrawList_Nil();
	ImVec2 wc = HANDLE_PAN();

	static float incline_angle_deg = 12.f;

	igSetNextWindowPos((ImVec2){io->DisplaySize.x / 2.0f, io->DisplaySize.y / 2.0f}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Control Window", 0, ImGuiWindowFlags_AlwaysAutoResize);
	igSliderFloat("Incline Angle", &incline_angle_deg, 0.f, 90.f, "%g°", ImGuiSliderFlags_AlwaysClamp);
	igEnd();

	float incline_rad = (180.f - incline_angle_deg) * DEG_TO_RAD;
	RENDER_GRID(wc);
	ImDrawList_AddRectFilled(dl, (ImVec2){0.f, io->DisplaySize.y}, (ImVec2){io->DisplaySize.x, wc.y}, IM_COL32(36,36,36, 255), 0.f, 0);

	const float end_y = tanf(incline_rad) * io->DisplaySize.x;
	const ImVec2 p0 = {io->DisplaySize.x, wc.y};
	const ImVec2 p1 = {0.f, wc.y};
	const ImVec2 p2 = {0.f, end_y + wc.y};
	ImDrawList_AddTriangleFilled(dl, p0, p1, p2, IM_COL32(36,36,36, 255));

	const float mid_end_y = tanf(incline_rad) * wc.x;
	r

	ABOUT_WIDGET();
	FRAME_PASS_END
}