#pragma once

#include "lib/arena.c"
#include "lib/list.c"
#include "optics.c"
#include "lights.c"

void test_mirror(Rays *light_rays, Lines *mirrors, Arena *arena) {
    *List_Push(light_rays, arena) = (Ray) {
        .start = { 500, 500 },
        .angle = -1
    };

    *List_Push(mirrors, arena) = (Line) {
        .start = { 700, 100 },
        .end = { 800, 100 }
    };
}

void test_lens(Rays *light_rays, Lenses *lenses, Arena *arena) {
    i32 num_rays = 11;
    for (i32 i = 0; i < num_rays; i++) {
        *List_Push(light_rays, arena) = (Ray) {
            .start = {
                200,
                150 + 50 * i
            },
            .angle = 0.0
        };
    }

    *List_Push(lenses, arena) = (Lens) {
        .focal_length = 300,
        .line = {
            .start = { 500, 100 },
            .end = { 500, 700 },
        }
    };
}

void test_lens_2(Rays *light_rays, Lenses *lenses, Arena *arena) {
    *List_Push(light_rays, arena) = (Ray) {
        .start = { 200, 200 },
        .angle = 0
    };

    *List_Push(lenses, arena) = (Lens) {
        .focal_length = 300,
        .line = {
            .start = { 500, 100 },
            .end = { 500, 700 },
        }
    };

    *List_Push(lenses, arena) = (Lens) {
        .focal_length = 300,
        .line = {
            .start = { 700, 100 },
            .end = { 700, 700 },
        }
    };
}

void test_update_setup(Rays *light_rays, PointLights *point_lights, Lenses *lenses, Arena *arena) {
    Vector2 position = { 600, 400 };
    PointLights_Add(point_lights, light_rays, &position, arena);

    *List_Push(lenses, arena) = (Lens) {
        .line = {
            .start = { 900, 200 },
            .end = { 900, 600 }
        },
        .focal_length = 200
    };
}

void test_update_main(Rays *light_rays, PointLights *point_lights) {
    f32 time = GetTime();
    Vector2 position = { 600 + 200 * cos(time), 400 + 200 * sin(time) };

    PointLight *point_light = PointLights_Get(point_lights, 0);
    PointLight_Update(point_light, light_rays, &position);
}
