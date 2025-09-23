#pragma once

#include "cglm/struct.h"
#include <stdint.h>

#define QUAD_SIZE 0.05f

typedef struct vertex {
    vec3s pos;
    vec2s uv;
} vertex_s;

extern const vertex_s quad_vertices[4];
extern const uint16_t quad_indices[6];
