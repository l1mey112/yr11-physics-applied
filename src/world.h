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
#ifndef WORLD_H
#define WORLD_H

#include <math.h>
#include <stdbool.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui.h"

#define MAX_OBJECTS 100
typedef struct Object Object;
typedef struct World World;

struct Object {
	ImVec2 pos;
	ImVec2 vel;
	// ImVec2 acc;
	float mass;
	float rad;
};

struct World {
	Object objects[MAX_OBJECTS];
	int obj_count;
} __world;

int world_add_object(ImVec2 pos, float mass, float rad)
{
	if (__world.obj_count >= MAX_OBJECTS) {
		return -1;
	}

	Object obj = {0};
	obj.pos = pos;
	obj.mass = mass;
	obj.rad = rad;

	int idx = __world.obj_count++;
	__world.objects[idx] = obj;

	return idx;
}

void world_integrate_collision(Object *obj1, Object *obj2) {
	float dx = obj1->pos.x - obj2->pos.x;
    float dy = obj1->pos.y - obj2->pos.y;
    float dist_squared = dx * dx + dy * dy;

    float sum_radii_squared = (obj1->rad + obj2->rad) * (obj1->rad + obj2->rad);

    bool is_colliding = dist_squared < sum_radii_squared;

	if (!is_colliding)
		return;

	// magnitude
	float dist = sqrt(dist_squared);

	// normal vectors, magnitude of 1.0f
	float nx = dx / dist;
	float ny = dy / dist;

	float rvx = obj1->vel.x - obj2->vel.x;
	float rvy = obj1->vel.y - obj2->vel.y;
	float v_mag_normal = rvx * nx + rvy * ny;

	// moving away from eachother
	// TODO: assert false?
	/* if (v_mag_normal > 0.f)
		return; */

	// obj1->coeff + obj2->coeff
	// 0.0: completely inelastic collision
	// 1.0: completely elastic collision
	// float e = (1.f + 1.f) / 2.f;	
	float e = 1.f;

	// implulse scalar
	float j = -(1.f + e) * v_mag_normal;
	j /= 1.f / obj1->mass + 1.f / obj2->mass;

	float ximp = j * nx;
	float yimp = j * ny;

	// apply impulse, a = F / m
	obj1->vel.x += 1.0f / obj1->mass * ximp;
    obj1->vel.y += 1.0f / obj1->mass * yimp;
    obj2->vel.x -= 1.0f / obj2->mass * ximp;
    obj2->vel.y -= 1.0f / obj2->mass * yimp;

	float overlap = obj1->rad + obj2->rad - dist;
	if (overlap >= 0.f) {
		float ov_x = overlap * nx / 2.f;
        float ov_y = overlap * ny / 2.f;

		obj1->pos.x += ov_x;
		obj1->pos.y += ov_y;
		obj2->pos.x -= ov_x;
		obj2->pos.y -= ov_y;
	}
}

void world_integrate(float dt)
{
	/* for (int i = 0; i < __world.obj_count; i++) {
		Object *obj = &__world.objects[i];

		// semi-implicit euler
		// obj->vel.x += obj->acc.x * dt;
		// obj->vel.y += obj->acc.y * dt;
	} */

	for (int i = 0; i < __world.obj_count; i++) {
        for (int j = i + 1; j < __world.obj_count; j++) {
            Object* obj1 = &__world.objects[i];
            Object* obj2 = &__world.objects[j];

			world_integrate_collision(obj1, obj2);
        }
    }

	/* for (int i = 0; i < __world.obj_count; i++) {
        for (int j = 0; j < __world.obj_count; j++) {
			if (i != j) {
				Object* obj1 = &__world.objects[i];
            	Object* obj2 = &__world.objects[j];

				world_integrate_collision(obj1, obj2);
			}
        }
    } */

	for (int i = 0; i < __world.obj_count; i++) {
		Object *obj = &__world.objects[i];

		// semi-implicit euler
		obj->pos.x += obj->vel.x * dt;
		obj->pos.y += obj->vel.y * dt;
		// obj->acc.x = 0.f;
		// obj->acc.y = 0.f;
	}
}

#endif // WORLD_H