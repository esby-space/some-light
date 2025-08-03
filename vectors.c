#ifndef VECTORS
#define VECTORS

#include <raylib.h>

#include "types.c"

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

#endif
