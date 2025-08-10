#ifndef OPTICS
#define OPTICS

#include "lib/types.c"
#include "lines.c"

typedef struct {
    Line line;
    f32 focal_length;
} Lens;

typedef struct {
    Lens *data;
    usize length;
    usize capacity;
} Lenses;

Lens Lenses_Get(Lenses *lenses, usize index) {
    if (index < 0 || index >= lenses->length) { raise(SIGTRAP); }
    return lenses->data[index]; 
}

// void Lenses_Push(Lenses *lenses, Lens lens) {
//     if (lenses->length == lenses->capacity) { raise(SIGTRAP); }
//     lenses->data[lenses->length] = lens;
//     lenses->length++;
// }

usize closest_lens(Ray *ray, Lenses *lenses, Vector2 *intersection, f32 *distance) {
    *intersection = (Vector2) { NAN, NAN };
    *distance = FLT_MAX;
    usize index = (usize) -1;

    for (i32 i = 0; i < lenses->length; i++) {
        Lens lens = Lenses_Get(lenses, i);
        Vector2 test_intersection = ray_line_intersect(ray, &lens.line);
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

// TODO: maybe make functions more general to any optical device?
f32 reflect_mirror(Ray *light_ray, Line *mirror) {
    f32 alpha = light_ray->angle;
    f32 beta = Line_Angle(mirror);

    f32 theta_1 = alpha - beta + PI/2;
    f32 theta_2 = PI - theta_1;
    return theta_2 + beta - PI/2;
}

f32 refract_lens(Ray *light_ray, Lens *lens) {
    f32 alpha = light_ray->angle;
    f32 beta = Line_Angle(&lens->line);

    Vector2 intersection = ray_line_intersect(light_ray, &lens->line);
    Vector2 center = Vector2_Average(&lens->line.start, &lens->line.end);
    Vector2 displacement = Vector2_Subtract(&center, &intersection);
    Vector2 light_vector = Ray_ToVector(light_ray);
    f32 x_1 = Vector2_Cross(&displacement, &light_vector);

    f32 theta_1 = alpha - beta + PI/2;
    f32 theta_2 = -x_1 / lens->focal_length + theta_1;
    return theta_2 + beta - PI/2;
}

#endif

