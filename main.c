#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <raylib.h>
#include <time.h>
#include <stdlib.h>

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

const u32 LIGHT_RAY_LENGTH = 4096;
const u32 POINT_SOURCE_RAY_NUMBER = 32;
const u32 LINE_SOURCE_RAY_DISTANCE = 32;

const u32 TEXT_HEIGHT = 16;
const u32 TEXT_PADDING = 4;

typedef struct {
    Vector2 start;
    f32 theta;
} LightRay;

typedef struct {
    LightRay *items;
    u32 length;
    u32 capacity;
} LightRays;

typedef struct {
    bool drawing_line_source;
    Vector2 line_source_start;
} LineState;

void add_point_source(LightRays*);
void add_line_source(LightRays*, LineState*);

int main() {
    InitWindow(WIDTH, HEIGHT, "esby is confused");
    SetTargetFPS(60);

    LightRay ray_buffer[MAX_LENGTH];
    LightRays rays = {
        .items = ray_buffer,
        .length = 0,
        .capacity = MAX_LENGTH
    };

    LineState line_state = (LineState) {
        .drawing_line_source = false,
    };

    bool drawing_line_source = false;
    Vector2 line_source_start;

    while (!WindowShouldClose()) {
        add_point_source(&rays);
        add_line_source(&rays, &line_state);

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < rays.length; i++) {
            LightRay line = rays.items[i];
            DrawLine(
                line.start.x,
                line.start.y,
                line.start.x + LIGHT_RAY_LENGTH * cos(line.theta),
                line.start.y + LIGHT_RAY_LENGTH * sin(line.theta),
                WHITE
            );
        }

        DrawText("[1] Add point source", 4, 4, TEXT_HEIGHT, LIGHTGRAY);
        DrawText("[2] Add line source", 4, 4 + 1.5 * TEXT_HEIGHT, TEXT_HEIGHT, LIGHTGRAY);
        DrawText("[3] Add ideal lens", 4, 4 + 3 * TEXT_HEIGHT, TEXT_HEIGHT, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

void add_point_source(LightRays *rays) {
    if (!IsKeyPressed(KEY_ONE)) return;
    printf("creating point source\n");
    for (int i = 0; i < POINT_SOURCE_RAY_NUMBER; i++) {
        rays->items[rays->length] = (LightRay) {
            .start = GetMousePosition(),
            .theta = 2 * PI * i / POINT_SOURCE_RAY_NUMBER
        };
        rays->length++;
    }
}

void add_line_source(LightRays* rays, LineState* state) {
    if (!IsKeyPressed(KEY_TWO)) return;
    if (!state->drawing_line_source) {
        printf("starting line source\n");
        state->line_source_start = GetMousePosition();
    } else {
        printf("creating line source\n");
        Vector2 start = state->line_source_start;
        Vector2 end = GetMousePosition();

        f32 distance = sqrt(pow(end.y - start.y, 2) + pow(end.x - start.x, 2));
        u32 num_rays = (u32) (distance / LINE_SOURCE_RAY_DISTANCE);
        f32 theta = atan2(end.y - start.y, end.x - start.x) - PI / 2;

        for (int i = 0; i <= num_rays; i++) {
            f32 scalar = (float) i / num_rays;
            rays->items[rays->length]= (LightRay) {
                .start = (Vector2) {
                    .x = start.x + scalar * (end.x - start.x),
                    .y = start.y + scalar * (end.y - start.y)
                },
                .theta = theta
            };
            rays->length++;
        }
    }

    state->drawing_line_source = !state->drawing_line_source;
}

