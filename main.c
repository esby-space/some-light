#include <float.h>
#include <math.h>
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
            Ray ray = light_rays.items[i];

            usize mirror_index;
            Vector2 intersection = closest_intersection(&ray, &mirrors, &mirror_index);

            while (!isnan(intersection.x) || !isnan(intersection.y)) {
                Line mirror = mirrors.items[mirror_index];
                Vector2 mirror_delta = Vector2Subtract(&mirror.end, &mirror.start);

                light_lines.items[light_lines.length] = (Line) {
                    .start = ray.start,
                    .end = intersection
                };

                light_lines.length++;

                ray.start = intersection;
                ray.theta = 2 * Vector2Direction(&mirror_delta) - ray.theta;
                intersection = closest_intersection(&ray, &mirrors, &mirror_index);
            }

            // TODO: implement lenses

            // add in the rest of the ray
            light_lines.items[light_lines.length] = (Line) {
                .start = ray.start,
                .end = {
                    ray.start.x + LIGHT_RAY_LENGTH * cos(ray.theta),
                    ray.start.y + LIGHT_RAY_LENGTH * sin(ray.theta),
                }
            };

            light_lines.length++;
        }

        // draw all `light_lines` and components
        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < light_lines.length; i++) {
            Line line = light_lines.items[i];
            DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, WHITE);
        }

        for (int i = 0; i < mirrors.length; i++) {
            Line line = mirrors.items[i];
            DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, GRAY);
        }

        for (int i = 0; i < lenses.length; i++) {
            Line line = lenses.items[i].line;
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
        light_rays->items[light_rays->length] = (Ray) {
            .start = GetMousePosition(),
            .theta = 2 * PI * i / POINT_SOURCE_RAY_NUMBER
        };
        light_rays->length++;
    }
}

void add_line_source(Rays* light_rays, DrawState* state) {
    if (!IsKeyPressed(KEY_TWO)) return;
    if (!state->drawing_line_source) {
        state->line_source_start = GetMousePosition();
    } else {
        Vector2 start = state->line_source_start;
        Vector2 end = GetMousePosition();
        Vector2 delta = Vector2Subtract(&end, &start);

        u32 num_rays = (u32) (Vector2Length(&delta) / LINE_SOURCE_RAY_DISTANCE);
        f32 theta = Vector2Direction(&delta) - PI / 2;

        for (int i = 0; i <= num_rays; i++) {
            f32 scalar = (float) i / num_rays;
            light_rays->items[light_rays->length] = (Ray) {
                .start = {
                    start.x + scalar * delta.x,
                    start.y + scalar * delta.y
                },
                .theta = theta
            };
            light_rays->length++;
        }
    }

    state->drawing_line_source = !state->drawing_line_source;
}

void add_mirror(Lines* mirrors, DrawState* state) {
    if (!IsKeyPressed(KEY_THREE)) return;
    if (!state->drawing_mirror) {
        state->mirror_start = GetMousePosition();
    } else {
        mirrors->items[mirrors->length] = (Line) {
            .start = state->mirror_start,
            .end = GetMousePosition()
        };
        mirrors->length++;
    }

    state->drawing_mirror = !state->drawing_mirror;
}

void add_lens(Lenses* lenses, DrawState* state) {
    if (!IsKeyPressed(KEY_FOUR)) return;
    if (!state->drawing_lens) {
        state->lens_start = GetMousePosition();
    } else {
        // FIX: focal length hardcoded
        lenses->items[lenses->length] = (Lens) {
            .line = {
                .start = state->lens_start,
                .end = GetMousePosition()
            },
            .focal_length = 100.0 
        };
        lenses->length++;
    }

    state->drawing_lens = !state->drawing_lens;
}

void test_mirrors(Rays* light_rays, Lines* mirrors) {
    light_rays->items[light_rays->length] = (Ray) {
        .start = { 500, 500 },
        .theta = -1
    };

    light_rays->length++;

    mirrors->items[mirrors->length] = (Line) {
        .start = { 700, 100 },
        .end = { 800, 100 }
    };

    mirrors->length++;

}
