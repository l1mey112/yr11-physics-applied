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
    ImVec2 acc;
    float mass;
};

struct World {
    Object objects[MAX_OBJECTS];
    int obj_count;
} __world;

bool world_add_object(ImVec2 pos, float mass)
{
    if __world.obj_count >= MAX_OBJECTS {
        return false;
    }

    __world.objects[__world.obj_count] = {
        .pos = pos,
        .__prev_pos = pos,
        .mass = mass,
    };
    __world.obj_count++;

    return true;
}

void world_collide(float dt)
{
    for (int i = 0; i < __world.obj_count; i++) {
        Object *obj = &__world.objects[i]

        // semi-implicit euler
        obj->vec.x += obj->acc.x * dt;
        obj->vec.y += obj->acc.y * dt;
        obj->pos.x += obj->vel.x * dt;
        obj->pos.y += obj->vel.y * dt;
    }
}

void world_integrate(float dt)
{
    for (int i = 0; i < __world.obj_count; i++) {
        Object *obj = &__world.objects[i]

        // semi-implicit euler
        obj->vel.x += obj->acc.x * dt;
        obj->vel.y += obj->acc.y * dt;
        {
            
        }
        obj->pos.x += obj->vel.x * dt;
        obj->pos.y += obj->vel.y * dt;
    }
}

#endif // WORLD_H