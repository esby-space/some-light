#include <math.h>
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

const u32 LINE_LENGTH = 1024;
const u32 NUM_POINT_LIGHT_RAYS = 32;

typedef struct {
    Vector2 start;
    float theta;
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

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        // Add point source
        if (IsKeyPressed(KEY_ONE)) {
            printf("creating point light\n");
            for (int i = 0; i < NUM_POINT_LIGHT_RAYS; i++) {
                rays.items[rays.length] = (LightRay) {
                    .start = GetMousePosition(),
                    .theta = 2 * PI * i / NUM_POINT_LIGHT_RAYS
                };
                rays.length++;
            }
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

