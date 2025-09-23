#include "quad.h"

const vertex_s quad_vertices[4] = {
    { .pos = {{ -QUAD_SIZE, -QUAD_SIZE, 0.0f }}, .uv = {{ 0.0f, 1.0f }} },
    { .pos = {{  QUAD_SIZE, -QUAD_SIZE, 0.0f }}, .uv = {{ 1.0f, 1.0f }} },
    { .pos = {{  QUAD_SIZE,  QUAD_SIZE, 0.0f }}, .uv = {{ 1.0f, 0.0f }} },
    { .pos = {{ -QUAD_SIZE,  QUAD_SIZE, 0.0f }}, .uv = {{ 0.0f, 0.0f }} }
};

const uint16_t quad_indices[6] = {
    0, 1, 2,
    0, 2, 3
};
