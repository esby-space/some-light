#define Ray RAYLIB_Ray
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <raylib.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#undef Ray

typedef float    f32;
typedef double   f64;
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef size_t   usize;
typedef ssize_t  isize;

const u32 MAX_LENGTH = 256;

const u32 WIDTH = 1200;
const u32 HEIGHT = 800;
const u32 TEXT_HEIGHT = 16;
const u32 TEXT_PADDING = 4;

const u32 LIGHT_RAY_LENGTH = 4096;
const u32 POINT_SOURCE_RAY_NUMBER = 32;
const u32 LINE_SOURCE_RAY_DISTANCE = 32;

//////////////
// OVERVIEW //
//////////////

// 1. When a light source is created or changed, update `light_rays` array.
// 2. When a component (like lenses or mirrors) is created or changed, iterate
//    over `LightRays` and components to generate `light_lines` array.
// 3. Iterate over `light_lines` to render final output.

// structs
typedef struct {
    Vector2 start;
    f32 theta;
} Ray;

typedef struct {
    Vector2 start;
    Vector2 end;
} Line;

typedef struct {
    bool drawing_line_source;
    bool drawing_mirror;
    bool drawing_lens;
    Vector2 line_source_start;
    Vector2 lens_start;
    Vector2 mirror_start;
} DrawState;

// arrays
typedef struct {
    Ray *items;
    u32 length;
    u32 capacity;
} Rays;

typedef struct {
    Line *items;
    u32 length;
    u32 capacity;
} Lines;

void add_point_source(Rays*);
void add_line_source(Rays*, DrawState*);
void add_mirror(Lines* mirrors, DrawState* state);
void add_lens(Lines* mirrors, DrawState* state);

bool line_intersects(Line* line_1, Line* line_2);
bool intersects(Ray* Ray, Line* Line);

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

    Line lens_buffer[MAX_LENGTH];
    Lines lenses = {
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
            // TODO: implelement the important stuff
            Ray ray = light_rays.items[i];
            light_lines.items[light_lines.length] = (Line) {
                .start = { .x = ray.start.x, .y = ray.start.y },
                .end = {
                    .x = ray.start.x + LIGHT_RAY_LENGTH * cos(ray.theta),
                    .y = ray.start.y + LIGHT_RAY_LENGTH * sin(ray.theta),
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
            Line line = lenses.items[i];
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

        f32 distance = sqrt(pow(end.y - start.y, 2) + pow(end.x - start.x, 2));
        u32 num_rays = (u32) (distance / LINE_SOURCE_RAY_DISTANCE);
        f32 theta = atan2(end.y - start.y, end.x - start.x) - PI / 2;

        for (int i = 0; i <= num_rays; i++) {
            f32 scalar = (float) i / num_rays;
            light_rays->items[light_rays->length]= (Ray) {
                .start = (Vector2) {
                    .x = start.x + scalar * (end.x - start.x),
                    .y = start.y + scalar * (end.y - start.y)
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

void add_lens(Lines* lenses, DrawState* state) {
    if (!IsKeyPressed(KEY_FOUR)) return;
    if (!state->drawing_lens) {
        state->lens_start = GetMousePosition();
    } else {
        lenses->items[lenses->length] = (Line) {
            .start = state->lens_start,
            .end = GetMousePosition()
        };
        lenses->length++;
    }

    state->drawing_lens = !state->drawing_lens;
}

bool line_intersects(Line* line_1, Line* line_2) {
    return NULL;
}

bool intersects(Ray* ray, Line* line) {
    return NULL;
}

