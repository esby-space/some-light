#ifndef LINES
#define LINES

#include <float.h>
#include <math.h>

#define Ray RAYLIB_Ray
#include <raylib.h>
#undef Ray

#include "types.c"
#include "constants.c"
#include "vectors.c"

typedef struct {
    Vector2 start;
    f32 theta;
} Ray;

typedef struct {
    Vector2 start;
    Vector2 end;
} Line;

typedef struct {
    Ray *items;
    usize length;
    usize capacity;
} Rays;

typedef struct {
    Line *items;
    usize length;
    usize capacity;
} Lines;

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

#endif
