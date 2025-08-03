#include <float.h>
#include <math.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define Ray RAYLIB_Ray
#include <raylib.h>
#undef Ray

#include "types.c"
#include "constants.c"
#include "vectors.c"
#include "lines.c"

//////////////
// OVERVIEW //
//////////////

// 1. When a light source is created or changed, update `light_rays` array.
// 2. When a component (like lenses or mirrors) is created or changed, iterate
//    over `LightRays` and components to generate `light_lines` array.
// 3. Iterate over `light_lines` to render final output.

typedef struct {
    bool drawing_line_source;
    bool drawing_mirror;
    bool drawing_lens;
    Vector2 line_source_start;
    Vector2 lens_start;
    Vector2 mirror_start;
} DrawState;

typedef struct {
    Line line;
    f32 focal_length;
} Lens;

typedef struct {
    Lens* items;
    usize length;
    usize capacity;
} Lenses;

Lens Lenses_Get(Lenses* lenses, usize index) {
    if (index < 0 || index >= lenses->length) { raise(SIGTRAP); }
    return lenses->items[index]; 
}

void Lenses_Push(Lenses* lenses, Lens lens) {
    if (lenses->length == lenses->capacity) { raise(SIGTRAP); }
    lenses->items[lenses->length] = lens;
    lenses->length++;
}

void add_point_source(Rays*);
void add_line_source(Rays*, DrawState*);
void add_mirror(Lines* mirrors, DrawState* state);
void add_lens(Lenses* mirrors, DrawState* state);

void test_mirrors(Rays* light_rays, Lines* mirrors);

int main() {
    InitWindow(WIDTH, HEIGHT, "esby is confused");
    SetTargetFPS(60);

    DrawState draw_state = (DrawState) {
        .drawing_line_source = false,
        .drawing_mirror = false,
        .drawing_lens = false
    };

    Ray light_ray_buffer[MAX_LENGTH];
    Rays light_rays = {
        .items = light_ray_buffer,
        .length = 0,
        .capacity = MAX_LENGTH
    };

    Line light_lines_buffer[MAX_LENGTH];
    Lines light_lines = {
        .items = light_lines_buffer,
        .length = 0,
        .capacity = MAX_LENGTH
    };

    Line mirror_buffer[MAX_LENGTH];
    Lines mirrors = {
        .items = mirror_buffer,
        .length = 0,
        .capacity = MAX_LENGTH
    };

    Lens lens_buffer[MAX_LENGTH];
    Lenses lenses = {
        .items = lens_buffer,
        .length = 0,
        .capacity = MAX_LENGTH
    };

    while (!WindowShouldClose()) {
        // add light sources and generate `light_rays` array
        add_point_source(&light_rays);
        add_line_source(&light_rays, &draw_state);

        // add components
        add_mirror(&mirrors, &draw_state);
        add_lens(&lenses, &draw_state);

        // generate `light_lines` every frame based on components
        for (int i = 0; i < light_rays.length; i++) {
            Ray ray = Rays_Get(&light_rays, i);

            usize mirror_index;
            Vector2 intersection = closest_intersection(&ray, &mirrors, &mirror_index);

            while (!isnan(intersection.x) || !isnan(intersection.y)) {
                Lines_Push(&light_lines, (Line) {
                    .start = ray.start,
                    .end = intersection
                });

                Line mirror = mirrors.items[mirror_index];
                Vector2 mirror_delta = Vector2_Subtract(&mirror.end, &mirror.start);

                ray.start = intersection;
                ray.theta = 2 * Vector2_Direction(&mirror_delta) - ray.theta;
                intersection = closest_intersection(&ray, &mirrors, &mirror_index);
            }

            // TODO: implement lenses
            // add in the rest of the ray
            Lines_Push(&light_lines, (Line) {
                .start = ray.start,
                .end = {
                    ray.start.x + LIGHT_RAY_LENGTH * cos(ray.theta),
                    ray.start.y + LIGHT_RAY_LENGTH * sin(ray.theta),
                }
            });
        }

        // draw all `light_lines` and components
        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < light_lines.length; i++) {
            Line line = Lines_Get(&light_lines, i);
            DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, WHITE);
        }

        for (int i = 0; i < mirrors.length; i++) {
            Line line = Lines_Get(&mirrors, i);
            DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, GRAY);
        }

        for (int i = 0; i < lenses.length; i++) {
            Line line = Lenses_Get(&lenses, i).line;
            DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, BLUE);
        }

        DrawText("[1] Add point source", 4, 4, TEXT_HEIGHT, LIGHTGRAY);
        DrawText("[2] Add line source", 4, 4 + 1.5 * TEXT_HEIGHT, TEXT_HEIGHT, LIGHTGRAY);
        DrawText("[3] Add mirror", 4, 4 + 3 * TEXT_HEIGHT, TEXT_HEIGHT, LIGHTGRAY);
        DrawText("[4] Add ideal lens", 4, 4 + 4.5 * TEXT_HEIGHT, TEXT_HEIGHT, LIGHTGRAY);

        EndDrawing();

        // TODO: persistent `light_lines` array
        light_lines.length = 0;
    }

    CloseWindow();
    return 0;
}

void add_point_source(Rays *light_rays) {
    if (!IsKeyPressed(KEY_ONE)) return;
    for (int i = 0; i < POINT_SOURCE_RAY_NUMBER; i++) {
        Rays_Push(light_rays, (Ray) {
            .start = GetMousePosition(),
            .theta = 2 * PI * i / POINT_SOURCE_RAY_NUMBER
        });
    }
}

void add_line_source(Rays* light_rays, DrawState* state) {
    if (!IsKeyPressed(KEY_TWO)) return;
    if (!state->drawing_line_source) {
        state->line_source_start = GetMousePosition();
    } else {
        Vector2 start = state->line_source_start;
        Vector2 end = GetMousePosition();
        Vector2 delta = Vector2_Subtract(&end, &start);

        u32 num_rays = (u32) (Vector2_Length(&delta) / LINE_SOURCE_RAY_DISTANCE);
        f32 theta = Vector2_Direction(&delta) - PI / 2;

        for (int i = 0; i <= num_rays; i++) {
            f32 scalar = (float) i / num_rays;
            Rays_Push(light_rays, (Ray) {
                .start = {
                    start.x + scalar * delta.x,
                    start.y + scalar * delta.y
                },
                .theta = theta
            });
        }
    }

    state->drawing_line_source = !state->drawing_line_source;
}

void add_mirror(Lines* mirrors, DrawState* state) {
    if (!IsKeyPressed(KEY_THREE)) return;
    if (!state->drawing_mirror) {
        state->mirror_start = GetMousePosition();
    } else {
        Lines_Push(mirrors, (Line) {
            .start = state->mirror_start,
            .end = GetMousePosition()
        });
    }

    state->drawing_mirror = !state->drawing_mirror;
}

void add_lens(Lenses* lenses, DrawState* state) {
    if (!IsKeyPressed(KEY_FOUR)) return;
    if (!state->drawing_lens) {
        state->lens_start = GetMousePosition();
    } else {
        Lenses_Push(lenses, (Lens) {
            .line = {
                .start = state->lens_start,
                .end = GetMousePosition()
            },
            .focal_length = 100.0 
        });
    }

    state->drawing_lens = !state->drawing_lens;
}

void test_mirrors(Rays* light_rays, Lines* mirrors) {
    Rays_Push(light_rays, (Ray) {
        .start = { 500, 500 },
        .theta = -1
    });

    Lines_Push(mirrors, (Line) {
        .start = { 700, 100 },
        .end = { 800, 100 }
    });
}
