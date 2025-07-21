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

const u32 LINE_LENGTH = 4096;
const u32 NUM_POINT_SOURCE_RAYS = 32;
const u32 LINE_SOURCE_RAY_DISTANCE = 32;

typedef struct {
    Vector2 start;
    f32 theta;
} LightRay;

typedef struct {
    LightRay *items;
    u32 length;
    u32 capacity;
} LightRays;

int main() {
    InitWindow(WIDTH, HEIGHT, "esby is confused");
    SetTargetFPS(60);

    LightRay ray_buffer[MAX_LENGTH];
    LightRays rays = {
        .items = ray_buffer,
        .length = 0,
        .capacity = MAX_LENGTH
    };

    bool drawing_line_source = false;
    Vector2 line_source_start;

    while (!WindowShouldClose()) {
        f32 dt = GetFrameTime();

        // Add point source
        if (IsKeyPressed(KEY_ONE)) {
            printf("creating point source\n");
            for (int i = 0; i < NUM_POINT_SOURCE_RAYS; i++) {
                rays.items[rays.length] = (LightRay) {
                    .start = GetMousePosition(),
                    .theta = 2 * PI * i / NUM_POINT_SOURCE_RAYS
                };
                rays.length++;
            }
        }

        // Add line source
        if (IsKeyPressed(KEY_TWO)) {
            if (!drawing_line_source) {
                printf("starting line source\n");
                line_source_start = GetMousePosition();
            } else {
                printf("creating line source\n");
                Vector2 start = line_source_start;
                Vector2 end = GetMousePosition();

                f32 distance = sqrt(pow(end.y - start.y, 2) + pow(end.x - start.x, 2));
                u32 num_rays = (u32) (distance / LINE_SOURCE_RAY_DISTANCE);
                f32 theta = atan2(end.y - start.y, end.x - start.x) - PI / 2;

                for (int i = 0; i <= num_rays; i++) {
                    f32 scalar = (float) i / num_rays;
                    rays.items[rays.length]= (LightRay) {
                        .start = (Vector2) {
                            .x = start.x + scalar * (end.x - start.x),
                            .y = start.y + scalar * (end.y - start.y)
                        },
                        .theta = theta
                    };
                    rays.length++;
                }
            }

            drawing_line_source = !drawing_line_source;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < rays.length; i++) {
            LightRay line = rays.items[i];
            DrawLine(
                line.start.x,
                line.start.y,
                line.start.x + LINE_LENGTH * cos(line.theta),
                line.start.y + LINE_LENGTH * sin(line.theta),
                WHITE
            );
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

