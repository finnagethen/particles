#pragma once

#include "cglm/struct.h"
#include <stddef.h>
#include <stdint.h>


typedef struct particles {
    size_t num_particles;

    vec3s* positions;
    vec3s* velocities;
    float* lifetimes;
} particles_s;

typedef struct particle_desc {
    vec3s position;
    vec3s velocity;
    float lifetime;
} particle_desc_s;

void particles_init(particles_s* p, size_t max_particles);
void particles_deinit(particles_s* p);
void particles_update(particles_s* p, float dt);
void particles_add(particles_s* p, const particle_desc_s* desc);


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
    size_t max_particles;
    emit_func emit;
} emitter_desc_s;

void emitter_init(emitter_s* e, const emitter_desc_s* desc);
void emitter_deinit(emitter_s* e);
void emitter_update(emitter_s* e, float dt);
void emitter_emit(emitter_s* e, float dt);


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
