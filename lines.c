#ifndef LINES
#define LINES

#include <float.h>
#include <math.h>
#include <signal.h>

#define Ray RAYLIB_Ray
#include <raylib.h>
#undef Ray

#include "types.c"
#include "constants.c"
#include "vectors.c"

typedef struct {
    Vector2 start;
    f32 angle;
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

Line Lines_Get(Lines *lines, usize index) {
    if (index < 0 || index >= lines->length) { raise(SIGTRAP); }
    return lines->items[index]; 
}

Ray Rays_Get(Rays *rays, usize index) {
    if (index < 0 || index >= rays->length) { raise(SIGTRAP); }
    return rays->items[index]; 
}

void Lines_Push(Lines *lines, Line line) {
    if (lines->length == lines->capacity) { raise(SIGTRAP); }
    lines->items[lines->length] = line;
    lines->length++;
}

void Rays_Push(Rays *rays, Ray ray) {
    if (rays->length == rays->capacity) { raise(SIGTRAP); }
    rays->items[rays->length] = ray;
    rays->length++;
}

Vector2 Ray_ToVector(Ray *ray) {
    return (Vector2) { cos(ray->angle), sin(ray->angle) };
}

f32 Line_Angle(Line *line) {
    Vector2 delta = Vector2_Subtract(&line->end, &line->start);
    return Vector2_Angle(&delta);
}

// line segment = a + bt where a = start, b = end - start, 0 < t < 1
// t_1 = (a_2 - a_1) X b_2 / (b_1 X b_2), t_2 = (a_2 - a_1) X b_1 / (b_1 X b_2)
Vector2 lines_intersect(Line *line_1, Line *line_2) {
    Vector2 a_1 = line_1->start;
    Vector2 a_2 = line_2->start;
    Vector2 b_1 = Vector2_Subtract(&line_1->end, &line_1->start);
    Vector2 b_2 = Vector2_Subtract(&line_2->end, &line_2->start);

    Vector2 separation = Vector2_Subtract(&a_2, &a_1);
    f32 denominator = Vector2_Cross(&b_1, &b_2);
    if (fabs(denominator) < EPSILON) return (Vector2) { NAN, NAN };

    // intersection when 0 < t_1, t_2 < 1
    f32 t_1 = Vector2_Cross(&separation, &b_2) / denominator;
    f32 t_2 = Vector2_Cross(&separation, &b_1) / denominator;
    if (t_1 > EPSILON && t_1 < 1.0  - EPSILON && t_2 > EPSILON && t_2 < 1.0 - EPSILON) {
        return (Vector2) { a_1.x + b_1.x * t_1, a_1.y + b_1.y * t_1 };
    }

    return (Vector2) { NAN, NAN };
}

Vector2 ray_line_intersect(Ray *ray, Line *line) {
    Vector2 a_1 = ray->start;
    Vector2 a_2 = line->start;
    Vector2 b_1 = Ray_ToVector(ray);
    Vector2 b_2 = Vector2_Subtract(&line->end, &line->start);

    Vector2 separation = Vector2_Subtract(&a_2, &a_1);
    f32 denominator = Vector2_Cross(&b_1, &b_2);
    if (fabs(denominator) < EPSILON) return (Vector2) { NAN, NAN };

    // intersection when t_1 > 0 and 0 < t_2 < 1
    f32 t_1 = Vector2_Cross(&separation, &b_2) / denominator;
    f32 t_2 = Vector2_Cross(&separation, &b_1) / denominator;
    if (t_1 > EPSILON && t_2 > EPSILON && t_2 < 1.0 - EPSILON) {
        return (Vector2) { a_1.x + b_1.x * t_1, a_1.y + b_1.y * t_1 };
    }

    return (Vector2) { NAN, NAN };
}

Vector2 rays_intersect(Ray *ray_1, Ray *ray_2) {
    Vector2 a_1 = ray_1->start;
    Vector2 a_2 = ray_2->start;
    Vector2 b_1 = Ray_ToVector(ray_1);
    Vector2 b_2 = Ray_ToVector(ray_2);

    Vector2 separation = Vector2_Subtract(&a_2, &a_1);
    f32 denominator = Vector2_Cross(&b_1, &b_2);
    if (fabs(denominator) < EPSILON) return (Vector2) { NAN, NAN };

    // intersection when t_1, t_2 > 0
    f32 t_1 = Vector2_Cross(&separation, &b_2) / denominator;
    f32 t_2 = Vector2_Cross(&separation, &b_1) / denominator;
    if (t_1 > EPSILON && t_2 > EPSILON) {
        return (Vector2) { a_1.x + b_1.x * t_1, a_1.y + b_1.y * t_1 };
    }

    return (Vector2) { NAN, NAN };
}

usize closest_intersection(Ray *ray, Lines *lines, Vector2 *intersection, f32 *distance) {
    *intersection = (Vector2) { NAN, NAN };
    *distance = FLT_MAX;
    usize index = (usize) -1;

    for (int i = 0; i < lines->length; i++) {
        Line line = Lines_Get(lines, i);
        Vector2 test_intersection = ray_line_intersect(ray, &line);
        if (isnan(test_intersection.x) || isnan(test_intersection.y)) continue;

        f32 test_distance = Vector2_Distance(&ray->start, &test_intersection);
        if (test_distance < *distance) {
            *distance = test_distance;
            *intersection = test_intersection;
            index = i;
        }
    }

    return index;
}

#endif
