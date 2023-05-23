#define HANDLE_PAN_NO_X
#include "demos.h"

// FRICTION IS NOT TAKEN INTO ACCOUNT

#define OBJ_HALF_X 100.f
#define OBJ_HALF_Y 50.f
#define OBJ_HALF_WIDTHS \
	(ImVec2) { OBJ_HALF_X, OBJ_HALF_Y }

void slice_rectangle(float x, float y, float width, float height, float angle, ImVec2* points) {
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    float hw = width / 2.0f;
    float hh = height / 2.0f;

    // Calculate the four corners of the rectangle
    ImVec2 corners[4];
    corners[0] = (ImVec2){x - hw, y - hh};
    corners[1] = (ImVec2){x + hw, y - hh};
    corners[2] = (ImVec2){x + hw, y + hh};
    corners[3] = (ImVec2){x - hw, y + hh};

    // Rotate the corners around the center point
    for (int i = 0; i < 4; i++) {
        float rx = (corners[i].x - x) * cos_a - (corners[i].y - y) * sin_a + x;
        float ry = (corners[i].x - x) * sin_a + (corners[i].y - y) * cos_a + y;
        corners[i].x = rx;
        corners[i].y = ry;
    }

    // Find the bottom half of the sliced rectangle
    float y_min = 3.402823466e+38F;
    int indices[4] = { -1, -1, -1, -1 };
    int count = 0;
    for (int i = 0; i < 4; i++) {
        if (corners[i].y <= y && corners[(i + 1) % 4].y <= y) {
            continue;
        }
        else if (corners[i].y > y && corners[(i + 1) % 4].y > y) {
            continue;
        }
        else {
            float x1 = corners[i].x;
            float y1 = corners[i].y;
            float x2 = corners[(i + 1) % 4].x;
            float y2 = corners[(i + 1) % 4].y;
            float m = (y2 - y1) / (x2 - x1);
            float b = y1 - m * x1;
            float x_intercept = (y - b) / m;
            if (x_intercept < corners[i].x || x_intercept > corners[(i + 1) % 4].x) {
                continue;
            }
            else {
                if (corners[i].y > corners[(i + 1) % 4].y) {
                    indices[count] = i;
                    count++;
                }
                else {
                    indices[count] = (i + 1) % 4;
                    count++;
                }
            }
        }
    }

    // Sort the indices of the bottom half of the rectangle
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (corners[indices[j]].x < corners[indices[i]].x) {
                int temp = indices[i];
                indices[i] = indices[j];
                indices[j] = temp;
            }
        }
    }

    // Copy the points of the bottom half into the output array
    for (int i = 0; i < count; i++) {
        points[i] = corners[indices[i]];
    }
}

static void frame(void)
{
	FRAME_PASS_BEGIN;

	ImVec2 wc = HANDLE_PAN();

	static float incline_angle_deg = 30.f;

	igSetNextWindowPos((ImVec2){__io->DisplaySize.x / 2.0f, __io->DisplaySize.y / 2.0f}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Control Window", 0, ImGuiWindowFlags_AlwaysAutoResize);
	igSliderFloat("Incline Angle", &incline_angle_deg, -90.f, 90.f, "%g°", ImGuiSliderFlags_AlwaysClamp);
	igEnd();

	float incline_rad = (180.f - incline_angle_deg) * DEG_TO_RAD;
	RENDER_GRID(wc);

	/* const float end_y = tanf(incline_rad) * wc.x;
	ImVec2 b0 = {0.f, end_y + wc.y};
	ImVec2 b1 = {__io->DisplaySize.x, wc.y - end_y};
	ImVec2 b2 = {__io->DisplaySize.x, __io->DisplaySize.y};
	ImVec2 b3 = {0.f, __io->DisplaySize.y};
	
	bool b0_low = b0.y < 0.f;
	bool b1_low = b1.y < 0.f;

	igText("(%g, %g) (%g, %g)", b0.x, b0.y, b1.x, b1.y);
	clip_line_segment(&b0, &b1, 0.f);
	igText("(%g, %g) (%g, %g)", b0.x, b0.y, b1.x, b1.y);

	if (b0_low) b3 = b0;
	if (b1_low) b2 = b1; */

	// 0.f, __io->DisplaySize.y/2.f, __io->DisplaySize.x, __io->DisplaySize.y

	/* ImVec2 points[4] = {
		{0.f, 0.f},
		{__io->DisplaySize.x, 0.f},
		{__io->DisplaySize.x, __io->DisplaySize.y},
		{0.f, __io->DisplaySize.y},
	}; */
	ImVec2 slice[4];
	slice_rectangle(wc.x, wc.y, wc.x, wc.y, incline_rad, slice);
	
	ImDrawList_AddQuad(__dl, slice[0], slice[1], slice[2], slice[3], IM_COL32(255,255,255,255), 2.f);

	const float mid_end_y = tanf(incline_rad) * wc.x;
	ImVec2 r0 = {-OBJ_HALF_X, 0.f};
	ImVec2 r1 = {OBJ_HALF_X, 0.f};
	ImVec2 r2 = {OBJ_HALF_X, -(OBJ_HALF_Y * 2.f)};
	ImVec2 r3 = {-OBJ_HALF_X, -(OBJ_HALF_Y * 2.f)};
	r0 = m_vrotate(r0, incline_rad);
	r1 = m_vrotate(r1, incline_rad);
	r2 = m_vrotate(r2, incline_rad);
	r3 = m_vrotate(r3, incline_rad);

	ImDrawList_AddQuad(__dl, m_rct(wc, r0), m_rct(wc, r1), m_rct(wc, r2), m_rct(wc, r3), IM_COL32(255,255,255, 255), 1.f);

	ABOUT_WIDGET();
	FRAME_PASS_END;
}