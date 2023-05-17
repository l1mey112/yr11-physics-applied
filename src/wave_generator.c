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


// def ratio(v1, v2, resultant_value):
//     ratio = v2 / v1
//     v1 = (resultant_value / ratio) ** 0.5
//     v2 = v1 * ratio
//     return v1, v2


static inline void ratio_product(float *v1, float *v2, float resultant)
{
	float ratio = *v1 / *v2;

	*v1 = sqrt(resultant / ratio);
	*v2 = *v1 * ratio;
}


// def ratio(v1, v2, quotient):
//     ratio = v1 / v2
//     new_v2 = v2
//     new_v1 = quotient * new_v2
//     return new_v1, new_v2

static inline void ratio_quotient(float *v1, float *v2, float resultant)
{
	float ratio = *v1 / *v2;

	*v1 = resultant * *v2;
}

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
	RENDER_GRID(wc);

	const float interval = 2.f;
	int x_size = (int)(__io->DisplaySize.x / interval);
	
	static float speed = 20.f;
	static float amplitude = 100.f;
	static float wavelength = 100.f;
	static float frequency = 0.2f;

	// _Static_assert(wavelength * frequency == speed);

	igSetNextWindowPos((ImVec2){__io->DisplaySize.x / 2.0f, __io->DisplaySize.y / 2.0f}, ImGuiCond_Once, (ImVec2){0, 0});
	igSetNextWindowSize((ImVec2){400.f, 400.f}, ImGuiCond_Once);
	igSetNextWindowCollapsed(is_inside_iframe(), ImGuiCond_Once);
	igBegin("Control Window", 0, ImGuiWindowFlags_AlwaysAutoResize);
	{
		igSliderFloat("Amplitude (A)", &amplitude, 10.f, 1000.f, "%g m", ImGuiSliderFlags_AlwaysClamp);
		bool is_frequency = igSliderFloat("Frequency (f)", &frequency, 0.01f, 2.f, "%g Hz", ImGuiSliderFlags_AlwaysClamp);
		bool is_wavelength = igSliderFloat("Wavelength (lambda)", &wavelength, 100.f, 1000.f, "%g m", ImGuiSliderFlags_AlwaysClamp);
		bool is_speed = igSliderFloat("Wave Velocity (m/s)", &speed, 10.f, 1000.f, "%g m/s", ImGuiSliderFlags_AlwaysClamp);

		if (is_frequency)
			ratio_quotient(&wavelength, &speed, frequency);
	}
	igSeparator();
	{
		igText("speed: %g", wavelength * frequency);
	}
	igEnd();



	ImVec2 ds = DELTA_SCROLL();
	float hw = __io->DisplaySize.x / 2.f + ds.x;

	ImVec2 p1 = {0.f, wc.y - ycord(amplitude, wavelength, frequency, 0.f - hw)};

	int x = 0;
	for (;;) {
		float x_new = p1.x + interval;
		ImVec2 p2 = {x_new, wc.y - ycord(amplitude, wavelength, frequency, x_new - hw)};

		ImDrawList_AddLine(__dl, p1, p2, IM_COL32(255, 255, 255, 255), 1.0f);

		p1 = p2;

		x++;
		if (x > x_size) break;
	}

	if (show_about) {
		ABOUT_WIDGET();
	}	

	FRAME_PASS_END
}