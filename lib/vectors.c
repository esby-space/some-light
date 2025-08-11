#pragma once

#include <math.h>

#include "raylib.c"
#include "types.c"

f32 Vector2_Length(Vector2 *v) {
    return sqrt(pow(v->x, 2) + pow(v->y, 2));
}

f32 Vector2_Angle(Vector2 *v) {
    return atan2(v->y, v->x);
}

Vector2 Vector2_Add(Vector2 *v, Vector2 *w) {
    return (Vector2) { v->x + w->x, v->y + w->y };
}

Vector2 Vector2_Subtract(Vector2 *v, Vector2 *w) {
    return (Vector2) { v->x - w->x, v->y - w->y };
}

Vector2 Vector2_Scale(Vector2 *v, f32 a) {
    return (Vector2) { a * v->x, a * v->y };
}

Vector2 Vector2_Average(Vector2 *v, Vector2 *w) {
    Vector2 sum = Vector2_Add(v, w);
    return Vector2_Scale(&sum, 0.5);
}

f32 Vector2_Distance(Vector2 *v, Vector2 *w) {
    Vector2 difference = Vector2_Subtract(w, v);
    return Vector2_Length(&difference);
}

f32 Vector2_Dot(Vector2 *a, Vector2 *b) {
    return a->x * b->x + a->y * b->y;
}

f32 Vector2_Cross(Vector2 *a, Vector2 *b) {
    return a->x * b->y - a->y * b->x;
}

