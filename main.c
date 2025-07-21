#include <stdint.h>
#include <stdio.h>
#include <raylib.h>
#include <time.h>
#include <stdlib.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 800;

int main() {
    InitWindow(WIDTH, HEIGHT, "esby is confused");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();

        BeginDrawing();
        ClearBackground(BLACK);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

