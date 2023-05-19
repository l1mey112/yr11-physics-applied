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

static inline float ycord(float amplitude, float wavelength, float x)
{
	return amplitude * sin(M_TAU / wavelength * x);
}

static void render_wave(ImVec2 wc, float y_diff, float x_start, float x_end, float interval, ImU32 col, float thickness, float amplitude, float wavelength)
{
	ImVec2 ds = DELTA_SCROLL();
	float hw = __io->DisplaySize.x / 2.f + ds.x;

	ImVec2 p1 = {x_start, wc.y - ycord(amplitude, wavelength, x_start - hw - y_diff)};

	for (;;) {
		float x_new = p1.x + interval;
		ImVec2 p2 = {x_new, wc.y - ycord(amplitude, wavelength, x_new - hw - y_diff)};

		ImDrawList_AddLine(__dl, p1, p2, col, thickness);

		if (p2.x > x_end) break;
		p1 = p2;
	}
}

static ImVec2 wave_point(ImVec2 wc, float y_diff, float x_pos, float amplitude, float wavelength)
{
	ImVec2 ds = DELTA_SCROLL();
	float hw = __io->DisplaySize.x / 2.f + ds.x;

	ImVec2 p1 = {x_pos, wc.y - ycord(amplitude, wavelength, x_pos - hw - y_diff)};

	return p1;
}

static void arrow(ImVec2 start, ImVec2 end, ImU32 color, float thickness, float sz)
{
    ImDrawList_AddLine(__dl, start, end, color, thickness);

    ImVec2 dir = {end.x - start.x, end.y - start.y};
    float length = sqrt(dir.x * dir.x + dir.y * dir.y);
    ImVec2 norm_dir = {dir.x / length, dir.y / length};

    ImVec2 p1 = {end.x - norm_dir.x * sz - norm_dir.y * sz * 0.6,
                                      end.y - norm_dir.y * sz + norm_dir.x * sz * 0.6};
    ImVec2 p2 = {end.x - norm_dir.x * sz + norm_dir.y * sz * 0.6,
                                      end.y - norm_dir.y * sz - norm_dir.x * sz * 0.6};

    ImDrawList_AddTriangleFilled(__dl, end, p1, p2, color);
}

#define VEC_RED (IM_COL32(255, 6, 0, 255))
#define VEC_RED_A (IM_COL32(255, 6, 0, 100))

static void frame(void)
{
	FRAME_PASS_BEGIN

	static bool freeze = false;
	static bool show_arrows = true;

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igTextWrapped("Welcome to the Sinusoidal Wave Simulation!");
		igTextWrapped("This simulation allows you to visualize a sinusoidal wave in real-time.");
		igSeparator();
		igTextWrapped("Amplitude (A): Determines the maximum displacement of the wave from its equilibrium position.");
		igSeparator();
		igTextWrapped("Frequency (f): Represents the number of complete cycles the wave completes in one second.");
		igSeparator();
		igTextWrapped("Wave Velocity (v): Determines how fast the wave propagates through the medium.");

		// igTextWrapped("Adjust the following parameters to control the wave characteristics:");
		// igTextWrapped("Key Concepts:");
		igSeparator();
		igCheckbox("Show About", &show_about);
		igCheckbox("Show Arrows", &show_arrows);
	}
	igEnd();

	ImVec2 wc = HANDLE_PAN();
	RENDER_GRID(wc);

	static float speed = 200.f;
	static float amplitude = 200.f;
	static float frequency = 0.6f;

	float wavelength = speed / frequency;

	igSetNextWindowPos((ImVec2){__io->DisplaySize.x / 2.0f + 100.f, __io->DisplaySize.y / 2.0f + 100.f}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Control Window", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igSliderFloat("Amplitude (A)", &amplitude, 10.f, 1000.f, "%g m", ImGuiSliderFlags_AlwaysClamp);
		igSliderFloat("Frequency (f)", &frequency, 0.05f, 2.f, "%g Hz", ImGuiSliderFlags_AlwaysClamp);
		igSliderFloat("Wave Velocity (m/s)", &speed, 100.f, 1000.f, "%g m/s", ImGuiSliderFlags_AlwaysClamp);
	}
	igSeparator();
	{
		igCheckbox("Freeze Wave", &freeze);
	}
	igEnd();

	static float f_diff = 0.f;
	ImVec2 ds = DELTA_SCROLL();
	
	if (freeze) {
		f_diff = 0.f;
	} else {
		f_diff += __io->DeltaTime * speed;
	}

	const float interval = 2.f;

	render_wave(wc, f_diff, 0.f, __io->DisplaySize.x, interval, IM_COL32(255, 255, 255, 255), 1.f, amplitude, wavelength);

	static float p_offset_x = 0.f;

	float hw = __io->DisplaySize.x / 2.f + ds.x;
	float hh = __io->DisplaySize.y / 2.f + ds.y;
	float p1 = hw + p_offset_x;
	float p2 = p1 + wavelength;

	ImVec2 ap1 = m_offset(wc, 0.f, -amplitude);
	ImVec2 ap2 = m_offset(wc, wavelength, 0.f);
	ImVec2 ap3 = m_offset(wc, speed, -10.f);

	if (show_arrows)
	{
		arrow(wc, ap1, IM_COL32(252, 217, 137, 255) /* IM_COL32(185, 246, 170, 255) */, 2.f, 10.f);
		arrow(wc, ap2, IM_COL32(252, 217, 137, 255), 2.f, 10.f);
		arrow(m_offset(wc, 0.f, -10.f), ap3, IM_COL32(252, 217, 137, 255), 2.f, 10.f);
	}

	render_wave(wc, f_diff, p1, p2, interval, IM_COL32(255, 6, 0, 255), 3.f, amplitude, wavelength);

	ImVec2 p1_v = wave_point(wc, f_diff, p1, amplitude, wavelength);
	ImVec2 p2_v = wave_point(wc, f_diff, p2, amplitude, wavelength);

	ImDrawList_AddCircleFilled(__dl, p1_v, 6.f, VEC_RED, 0);
	ImDrawList_AddCircleFilled(__dl, p2_v, 6.f, VEC_RED, 0);

	if (show_arrows)
	{
		static char buf[128];

		snprintf(buf, sizeof(buf), "Amplitude (A): %gm", amplitude);
		ImDrawList_AddText_Vec2(__dl, m_offset(ap1, 20.f, -20.f), IM_COL32_WHITE, buf, NULL);

		snprintf(buf, sizeof(buf), "Wavelength (lambda): %gm", wavelength);
		ImDrawList_AddText_Vec2(__dl, m_offset(ap2, 20.f, -20.f), IM_COL32_WHITE, buf, NULL);

		snprintf(buf, sizeof(buf), "Wave Velocity (v): %gm/s", speed);
		ImDrawList_AddText_Vec2(__dl, m_offset(ap3, 20.f, 25.f), IM_COL32_WHITE, buf, NULL);
	}


	if (show_about) {
		ABOUT_WIDGET();
	}	

	FRAME_PASS_END
}