#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define Ray RAYLIB_Ray
#include <raylib.h>
#undef Ray

typedef float     f32;
typedef double    f64;
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef uintptr_t usize;
typedef intptr_t  isize;

const u32 MAX_LENGTH = 2048;
const f32 EPSILON = 1e-4;

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

Vector2 Vector2Add(Vector2* v, Vector2* w);
Vector2 Vector2Subtract(Vector2* v, Vector2* w);
Vector2 Vector2Scale(Vector2* v, f32 a);
f32 Vector2Length(Vector2* v);
f32 Vector2Direction(Vector2* v);
f32 Vector2Dot(Vector2* a, Vector2* b);
f32 Vector2Cross(Vector2* a, Vector2* b);

Vector2 lines_intersect(Line* line_1, Line* line_2);
Vector2 ray_line_intersect(Ray* ray, Line* line);
Vector2 rays_intersect(Ray* ray_1, Ray* ray_2);
Vector2 closest_intersection(Ray* ray, Lines* lines, usize* index);

void add_point_source(Rays*);
void add_line_source(Rays*, DrawState*);
void add_mirror(Lines* mirrors, DrawState* state);
void add_lens(Lines* mirrors, DrawState* state);

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

Vector2 Vector2Add(Vector2* v, Vector2* w) {
    return (Vector2) { v->x + w->x, v->y + w->y };
}

Vector2 Vector2Subtract(Vector2* v, Vector2* w) {
    return (Vector2) { v->x - w->x, v->y - w->y };
}

Vector2 Vector2Scale(Vector2* v, f32 a) {
    return (Vector2) { a * v->x, a * v->y };
}

f32 Vector2Length(Vector2* v) {
    return sqrt(pow(v->x, 2) + pow(v->y, 2));
}

f32 Vector2Direction(Vector2* v) {
    return atan2(v->y, v->x);
}

f32 Vector2Dot(Vector2* a, Vector2* b) {
    return a->x * b->x + a->y * b->y;
}

f32 Vector2Cross(Vector2* a, Vector2* b) {
    return a->x * b->y - a->y * b->x;
}

// line segment = a + bt where a = start, b = end - start, 0 < t < 1
// t_1 = (a_2 - a_1) X b_2 / (b_1 X b_2), t_2 = (a_2 - a_1) X b_1 / (b_1 X b_2)
Vector2 lines_intersect(Line* line_1, Line* line_2) {
    Vector2 a_1 = line_1->start;
    Vector2 a_2 = line_2->start;
    Vector2 b_1 = Vector2Subtract(&line_1->end, &line_1->start);
    Vector2 b_2 = Vector2Subtract(&line_2->end, &line_2->start);

    Vector2 separation = Vector2Subtract(&a_2, &a_1);
    f32 denominator = Vector2Cross(&b_1, &b_2);
    if (fabs(denominator) < EPSILON) return (Vector2) { NAN, NAN };

    // intersection when 0 < t_1, t_2 < 1
    f32 t_1 = Vector2Cross(&separation, &b_2) / denominator;
    f32 t_2 = Vector2Cross(&separation, &b_1) / denominator;
    if (t_1 > EPSILON && t_1 < 1.0  - EPSILON && t_2 > EPSILON && t_2 < 1.0 - EPSILON) {
        return (Vector2) { a_1.x + b_1.x * t_1, a_1.y + b_1.y * t_1 };
    }

    return (Vector2) { NAN, NAN };
}

Vector2 ray_line_intersect(Ray* ray, Line* line) {
    Vector2 a_1 = ray->start;
    Vector2 a_2 = line->start;
    Vector2 b_1 = (Vector2) { cos(ray->theta), sin(ray->theta) };
    Vector2 b_2 = Vector2Subtract(&line->end, &line->start);

    Vector2 separation = Vector2Subtract(&a_2, &a_1);
    f32 denominator = Vector2Cross(&b_1, &b_2);
    if (fabs(denominator) < EPSILON) return (Vector2) { NAN, NAN };

    // intersection when t_1 > 0 and 0 < t_2 < 1
    f32 t_1 = Vector2Cross(&separation, &b_2) / denominator;
    f32 t_2 = Vector2Cross(&separation, &b_1) / denominator;
    if (t_1 > EPSILON && t_2 > EPSILON && t_2 < 1.0 - EPSILON) {
        return (Vector2) { a_1.x + b_1.x * t_1, a_1.y + b_1.y * t_1 };
    }

    return (Vector2) { NAN, NAN };
}

Vector2 rays_intersect(Ray* ray_1, Ray* ray_2) {
    Vector2 a_1 = ray_1->start;
    Vector2 a_2 = ray_2->start;
    Vector2 b_1 = (Vector2) { cos(ray_1->theta), sin(ray_1->theta) };
    Vector2 b_2 = (Vector2) { cos(ray_2->theta), sin(ray_2->theta) };

    Vector2 separation = Vector2Subtract(&a_2, &a_1);
    f32 denominator = Vector2Cross(&b_1, &b_2);
    if (fabs(denominator) < EPSILON) return (Vector2) { NAN, NAN };

    // intersection when t_1, t_2 > 0
    f32 t_1 = Vector2Cross(&separation, &b_2) / denominator;
    f32 t_2 = Vector2Cross(&separation, &b_1) / denominator;
    if (t_1 > EPSILON && t_2 > EPSILON) {
        return (Vector2) { a_1.x + b_1.x * t_1, a_1.y + b_1.y * t_1 };
    }

    return (Vector2) { NAN, NAN };
}

Vector2 closest_intersection(Ray* ray, Lines* lines, usize* index) {
    Vector2 closest_intersection = (Vector2) { NAN, NAN };
    f32 closest_distance = FLT_MAX;
    *index = (usize) - 1;

    for (int i = 0; i < lines->length; i++) {
        Line line = lines->items[i];
        Vector2 intersection = ray_line_intersect(ray, &line);
        if (isnan(intersection.x) || isnan(intersection.y)) continue;

        Vector2 delta = Vector2Subtract(&ray->start, &intersection);
        f32 distance = Vector2Length(&delta);
        if (distance < closest_distance) {
            closest_distance = distance;
            closest_intersection = intersection;
            *index = i;
        }
    }

    return closest_intersection;
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
