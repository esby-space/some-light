//////////////
// OVERVIEW //
//////////////

// 1. When a light source is created or changed, update `light_rays` array.
// 2. When a component (like lenses or mirrors) is created or changed, iterate
//    over `LightRays` and components to generate `light_lines` array.
// 3. Iterate over `light_lines` to render final output.

/////////////////////
// MATH CONVENTION //
/////////////////////

// The origin lies in the top left corner, with positive-x going from left to
// right and positive-y going from top to bottom. All angles are in radians with
// positive angles corresponding to clockwise rotation.
// 
// When a light ray  and optical component form an angle ⍺ and β with the
// horizontal respectively, the light ray forms an angle θ_1 = π/2 + ⍺ - β
// relative to the normal of the component at a displacement x_1 from the center
// of the component. The light ray is then reflected or refracted at an angle
// θ_2 relative to the normal at a displacement x_2 according to the [Ray
// Transfer Matrix Analysis]
// (https://en.wikipedia.org/wiki/Ray_transfer_matrix_analysis). The resulting
// ray forms an angle ⍺' = β + θ_2 - π/2.

#include <float.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <signal.h>

#include "lib/raylib.c"
#include "lib/types.c"
#include "lib/arena.c"
#include "lib/list.c"
#include "lib/vectors.c"

#include "constants.c"
#include "lines.c"
#include "optics.c"
#include "lights.c"
#include "tests.c"

typedef struct {
    bool drawing_line_source;
    bool drawing_mirror;
    bool drawing_lens;
    Vector2 line_source_start;
    Vector2 lens_start;
    Vector2 mirror_start;
} DrawState;

void add_line_source(Rays*, DrawState*, Arena*);
void add_mirror(Lines*, DrawState*, Arena*);
void add_lens(Lenses*, DrawState*, Arena*);

i32 main() {
    printf("hi\n");

    Arena arena = Arena_New(1024 * 1014);

    InitWindow(WIDTH, HEIGHT, "esby is confused");
    SetTargetFPS(60);
    Font font = LoadFontEx("/System/Library/Fonts/Monaco.ttf", 32, 0, 250);

    DrawState draw_state = (DrawState) {
        .drawing_line_source = false,
        .drawing_mirror = false,
        .drawing_lens = false
    };

    Rays light_rays = { 0 };
    Lines light_lines = { 0 };
    Lines mirrors = { 0 };
    Lenses lenses = { 0 };
    PointLights point_lights = { 0 };

    test_update_setup(&light_rays, &point_lights, &lenses, &arena);

    while (!WindowShouldClose()) {
        Vector2 mouse = GetMousePosition();

        // add_point_source(&light_rays, &arena);
        if (IsKeyPressed(KEY_ONE)) PointLights_Add(&point_lights, &light_rays, &mouse, &arena);
        add_line_source(&light_rays, &draw_state, &arena);
        add_mirror(&mirrors, &draw_state, &arena);
        add_lens(&lenses, &draw_state, &arena);

        test_update_main(&light_rays, &point_lights);

        // generate `light_lines` every frame based on components
        for (i32 i = 0; i < light_rays.length; i++) {
            Ray *ray = Rays_Get(&light_rays, i);

            Vector2 mirror_intersection, lens_intersection;
            f32 mirror_distance, lens_distance;
            usize mirror_index = closest_intersection(ray, &mirrors, &mirror_intersection, &mirror_distance);
            usize lens_index = closest_lens(ray, &lenses, &lens_intersection, &lens_distance);

            while (mirror_index != (usize) -1 || lens_index != (usize) -1) {
                // IDEA: take minimum of components, use result to pick intersection and refraction function
                if (mirror_distance < lens_distance) {
                    *List_Push(&light_lines, &arena) = (Line) {
                        .start = ray->start,
                        .end = mirror_intersection
                    };

                    Line *mirror = Lines_Get(&mirrors, mirror_index);
                    ray = &(Ray) {
                        .angle = reflect_mirror(ray, mirror),
                        .start = mirror_intersection
                    };

                    mirror_index = closest_intersection(ray, &mirrors, &mirror_intersection, &mirror_distance);
                    lens_index = closest_lens(ray, &lenses, &lens_intersection, &lens_distance);
                }

                if (lens_distance < mirror_distance) {
                    *List_Push(&light_lines, &arena) = (Line) {
                        .start = ray->start,
                        .end = lens_intersection
                    };

                    Lens *lens = Lenses_Get(&lenses, lens_index);
                    ray = &(Ray) {
                        .angle = refract_lens(ray, lens),
                        .start = lens_intersection
                    };

                    mirror_index = closest_intersection(ray, &mirrors, &mirror_intersection, &mirror_distance);
                    lens_index = closest_lens(ray, &lenses, &lens_intersection, &lens_distance);
                }
            }

            // add in the rest of the ray
            Vector2 light_vector = Ray_ToVector(ray);
            Vector2 scaled_vector = Vector2_Scale(&light_vector, LIGHT_RAY_LENGTH);
            *List_Push(&light_lines, &arena) = (Line) {
                .start = ray->start,
                .end = Vector2_Add(&ray->start, &scaled_vector)
            };
        }

        // draw all `light_lines` and components
        BeginDrawing();
        ClearBackground(BLACK);

        for (i32 i = 0; i < light_lines.length; i++) {
            Line *line = Lines_Get(&light_lines, i);
            DrawLine(line->start.x, line->start.y, line->end.x, line->end.y, WHITE);
        }

        for (i32 i = 0; i < mirrors.length; i++) {
            Line *line = Lines_Get(&mirrors, i);
            DrawLine(line->start.x, line->start.y, line->end.x, line->end.y, GRAY);
        }

        for (i32 i = 0; i < lenses.length; i++) {
            Line line = Lenses_Get(&lenses, i)->line;
            DrawLine(line.start.x, line.start.y, line.end.x, line.end.y, BLUE);
        }

        DrawTextEx(font, "[1] Add point source", (Vector2) { 4, 4 }, TEXT_HEIGHT, TEXT_SPACING, LIGHTGRAY);
        DrawTextEx(font, "[2] Add line source", (Vector2) { 4, 4 + 1.2 * TEXT_HEIGHT }, TEXT_HEIGHT, TEXT_SPACING, LIGHTGRAY);
        DrawTextEx(font,"[3] Add mirror", (Vector2) { 4, 4 + 2.4 * TEXT_HEIGHT }, TEXT_HEIGHT, TEXT_SPACING, LIGHTGRAY);
        DrawTextEx(font, "[4] Add ideal lens", (Vector2) { 4, 4 + 3.6 * TEXT_HEIGHT }, TEXT_HEIGHT, TEXT_SPACING, LIGHTGRAY);

        EndDrawing();

        // TODO: persistent `light_lines` array?
        light_lines.length = 0;
    }

    Arena_Free(&arena);
    CloseWindow();
    return 0;
}

void add_line_source(Rays *light_rays, DrawState *state, Arena *arena) {
    if (!IsKeyPressed(KEY_TWO)) return;
    if (!state->drawing_line_source) {
        state->line_source_start = GetMousePosition();
    } else {
        Vector2 start = state->line_source_start;
        Vector2 end = GetMousePosition();
        Vector2 delta = Vector2_Subtract(&end, &start);

        u32 num_rays = (u32) (Vector2_Length(&delta) / LINE_SOURCE_RAY_DISTANCE);
        f32 theta = Vector2_Angle(&delta) - PI / 2;

        for (i32 i = 0; i <= num_rays; i++) {
            f32 scalar = (float) i / num_rays;
            *List_Push(light_rays, arena) = (Ray) {
                .start = {
                    start.x + scalar * delta.x,
                    start.y + scalar * delta.y
                },
                .angle = theta
            };
        }
    }

    state->drawing_line_source = !state->drawing_line_source;
}

void add_mirror(Lines *mirrors, DrawState *state, Arena *arena) {
    if (!IsKeyPressed(KEY_THREE)) return;
    if (!state->drawing_mirror) {
        state->mirror_start = GetMousePosition();
    } else {
        *List_Push(mirrors, arena) = (Line) {
            .start = state->mirror_start,
            .end = GetMousePosition()
        };
    }

    state->drawing_mirror = !state->drawing_mirror;
}

// TODO: make focal length adjustable
void add_lens(Lenses *lenses, DrawState *state, Arena *arena) {
    if (!IsKeyPressed(KEY_FOUR)) return;
    if (!state->drawing_lens) {
        state->lens_start = GetMousePosition();
    } else {
        *List_Push(lenses, arena) = (Lens) {
            .line = {
                .start = state->lens_start,
                .end = GetMousePosition()
            },
            .focal_length = 300.0
        };
    }

    state->drawing_lens = !state->drawing_lens;
}

