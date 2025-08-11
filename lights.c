#pragma once

#include "lib/types.c"
#include "lib/arena.c"
#include "lib/list.c"
#include "lines.c"

typedef struct {
    Vector2 position;
    usize *ray_indices;
} PointLight;

typedef struct {
    PointLight *data;
    usize length;
    usize capacity;
} PointLights;

PointLight *PointLights_Get(PointLights *point_lights, usize index) {
    if (index < 0 || index >= point_lights->length) { raise(SIGTRAP); }
    return point_lights->data + index;
}

void PointLights_Add(PointLights *point_lights, Rays *light_rays, Vector2 *position, Arena *arena) {
    // TODO: way to do this without global arena? in case point light is deleted?
    usize *ray_indices = Arena_Alloc(arena, usize, POINT_SOURCE_RAY_NUMBER);
    for (usize i = 0; i < POINT_SOURCE_RAY_NUMBER; i++) {
        ray_indices[i] = light_rays->length;
        *List_Push(light_rays, arena) = (Ray) {
            .start = *position,
            .angle = 2 * PI * i / POINT_SOURCE_RAY_NUMBER
        };
    }

    *List_Push(point_lights, arena) = (PointLight) {
        .position = *position,
        .ray_indices = ray_indices
    };
}

void PointLight_Update(PointLight *point_light, Rays *light_rays, Vector2 *position) {
    point_light->position = *position;
    for (usize i = 0; i < POINT_SOURCE_RAY_NUMBER; i++) {
        usize ray_index = point_light->ray_indices[i];
        Ray *light_ray = Rays_Get(light_rays, ray_index);
        light_ray->start = *position;
    }
}
