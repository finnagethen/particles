#pragma once

#include "cglm/struct.h"
#include <stddef.h>
#include <stdint.h>


typedef struct particles {
    size_t num_particles;

    vec3s* positions;
    vec3s* velocities;
    float* lifetimes;

    vec4s start_color;
    vec4s end_color;
    vec4s* colors;
} particles_s;

typedef struct particle_desc {
    vec3s position;
    vec3s velocity;
    float lifetime;
} particle_desc_s;

typedef struct particles_desc {
    size_t max_particles;
    vec4s start_color;
    vec4s end_color;
} particles_desc_s;


typedef struct emitter emitter_s; // forward declaration
typedef void (*emit_func)(struct emitter* e);

typedef struct emitter {
    float emission_rate; // particles per second
    float emission_accum; // accumulator to track emission timing           
    
    size_t max_particles;
    particles_s particles;

    emit_func emit;
} emitter_s;

typedef struct emitter_desc {
    float emission_rate;
    emit_func emit;

    const particles_desc_s* particles_desc;
} emitter_desc_s;

void emitter_init(emitter_s* e, const emitter_desc_s* desc);
void emitter_deinit(emitter_s* e);
void emitter_update(emitter_s* e, float dt);
void emitter_emit(emitter_s* e, float dt);
void emitter_add_particle(emitter_s* e, const particle_desc_s* desc);


static const float quad_size = 0.05f;
static const float quad_vertices[] = {
    // x, y, z
    -quad_size, -quad_size, 0.0f,  // bottom-left
     quad_size, -quad_size, 0.0f,  // bottom-right
     quad_size,  quad_size, 0.0f,  // top-right
    -quad_size,  quad_size, 0.0f   // top-left
};

static const uint16_t quad_indices[] = {
    0, 1, 2, // first triangle
    0, 2, 3  // second triangle
}; 
