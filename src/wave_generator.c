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

static inline float ycord(float amplitude, float wavelength, float frequency, float x)
{
	return amplitude * sin(M_TAU * frequency * x / wavelength);
}

static void render_wave(ImVec2 wc, float y_diff, float x_start, float x_end, float interval, ImU32 col, float thickness, float amplitude, float wavelength, float frequency)
{
	ImVec2 ds = DELTA_SCROLL();
	float hw = __io->DisplaySize.x / 2.f + ds.x;

	ImVec2 p1 = {x_start, wc.y - ycord(amplitude, wavelength, frequency, x_start - hw - y_diff)};

	for (;;) {
		float x_new = p1.x + interval;
		ImVec2 p2 = {x_new, wc.y - ycord(amplitude, wavelength, frequency, x_new - hw - y_diff)};

		ImDrawList_AddLine(__dl, p1, p2, col, thickness);

		if (p2.x > x_end) break;
		p1 = p2;
	}
}

static ImVec2 wave_point(ImVec2 wc, float y_diff, float x_pos, float amplitude, float wavelength, float frequency)
{
	ImVec2 ds = DELTA_SCROLL();
	float hw = __io->DisplaySize.x / 2.f + ds.x;

	ImVec2 p1 = {x_pos, wc.y - ycord(amplitude, wavelength, frequency, x_pos - hw - y_diff)};

	return p1;
}

#define VEC_RED (IM_COL32(255, 6, 0, 255))

static void frame(void)
{
	FRAME_PASS_BEGIN

	igSetNextWindowPos((ImVec2){10, 10}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Hello Dear ImGui!", 0, ImGuiWindowFlags_AlwaysAutoResize);
	igCheckbox("Show About", &show_about);
	igText("dt: %g", __io->DeltaTime);
	igEnd();

	ImVec2 wc = HANDLE_PAN();
	RENDER_GRID(wc);

	static float speed = 300.f;
	static float amplitude = 200.f;
	static float frequency = 1.f;

	float wavelength = speed / frequency;

	igSetNextWindowPos((ImVec2){__io->DisplaySize.x / 2.0f, __io->DisplaySize.y / 2.0f}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Control Window", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igSliderFloat("Amplitude (A)", &amplitude, 10.f, 1000.f, "%g m", ImGuiSliderFlags_AlwaysClamp);
		igSliderFloat("Frequency (f)", &frequency, 0.01f, 2.f, "%g Hz", ImGuiSliderFlags_AlwaysClamp);
		//igSliderFloat("Wavelength (lambda)", &wavelength, 100.f, 1000.f, "%g m", ImGuiSliderFlags_AlwaysClamp);
		igSliderFloat("Wave Velocity (m/s)", &speed, 10.f, 1000.f, "%g m/s", ImGuiSliderFlags_AlwaysClamp);

		// if (is_frequency)
		// 	ratio_quotient(&wavelength, &speed, frequency);
	}
	igSeparator();
	{
		// igText("speed: %g", wavelength * frequency);
	}
	igEnd();

	static float f_diff = 0.f;
	f_diff += __io->DeltaTime * speed;

	ImVec2 ds = DELTA_SCROLL();

	// float wavelength2 = wavelength;
	// if (f_diff > wavelength2) {
	// 	f_diff -= wavelength2;
	// }

	const float interval = 2.f;

	render_wave(wc, f_diff, 0.f, __io->DisplaySize.x, interval, IM_COL32(255, 255, 255, 255), 1.f, amplitude, wavelength, frequency);

	float hw = __io->DisplaySize.x / 2.f + ds.x;
	float hh = __io->DisplaySize.x / 2.f + ds.y;
	float p1 = hw;
	float p2 = hw + wavelength;

	render_wave(wc, f_diff, hw, hw + wavelength, interval, IM_COL32(255, 6, 0, 255), 2.f, amplitude, wavelength, frequency);

	ImVec2 p1_v = wave_point(wc, f_diff, p1, amplitude, wavelength, frequency);
	ImVec2 p2_v = wave_point(wc, f_diff, p2, amplitude, wavelength, frequency);

	ImDrawList_AddCircleFilled(__dl, p1_v, 6.f, VEC_RED, 0);
	ImDrawList_AddCircleFilled(__dl, p2_v, 6.f, VEC_RED, 0);

	ImVec2 b1_v = {p1_v.x, hh - (amplitude + 100.f)};
	ImVec2 b2_v = {p2_v.x, hh - (amplitude + 100.f)};

	ImDrawList_AddLine(__dl, b1_v, b2_v, VEC_RED, 1.5f);

	if (show_about) {
		ABOUT_WIDGET();
	}	

	FRAME_PASS_END
}