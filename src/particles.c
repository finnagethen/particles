#include "particles.h"

#include <stdlib.h>
#include <assert.h>


void particles_init(particles_s* p, size_t max_particles) {
    assert(p);
    
    *p = (particles_s){
        .num_particles = 0,
        .positions = malloc(max_particles * sizeof(vec3s)),
        .velocities = malloc(max_particles * sizeof(vec3s)),
        .lifetimes = malloc(max_particles * sizeof(float))
    };

    assert(p->positions && p->velocities);
}

void particles_deinit(particles_s* p) {
    if (p) {
        free(p->positions);
        free(p->velocities);
        free(p->lifetimes);
        *p = (particles_s){ };
    }
}

void particles_update(particles_s* p, float dt) {
    assert(p && dt >= 0.0f);
    
    size_t i = 0;
    while (i < p->num_particles) {
        p->positions[i] = glms_vec3_add(p->positions[i], glms_vec3_scale(p->velocities[i], dt));
        p->lifetimes[i] -= dt;

        // if lifetime expired, remove particle by swapping with the last one
        if (p->lifetimes[i] <= 0.0f) {
            p->positions[i] = p->positions[p->num_particles - 1];
            p->velocities[i] = p->velocities[p->num_particles - 1];
            p->lifetimes[i] = p->lifetimes[p->num_particles - 1];
            p->num_particles--;
        } else {
            i++;
        }
    }
}

void particles_add(particles_s* p, const particle_desc_s* desc) {
    assert(p && desc);
    
    const size_t idx = p->num_particles++;
    p->positions[idx] = desc->position;
    p->velocities[idx] = desc->velocity;
    p->lifetimes[idx] = desc->lifetime;
}

void emitter_init(emitter_s* e, const emitter_desc_s* desc) {
    assert(e && desc);
    
    *e = (emitter_s){
        .emission_rate = desc->emission_rate,
        .emission_accum = 0.0f,
        .max_particles = desc->max_particles,
        .emit = desc->emit
    };
    particles_init(&e->particles, e->max_particles);
}

void emitter_deinit(emitter_s* e) {
    if (e) {
        particles_deinit(&e->particles);
        *e = (emitter_s){ };
    }
}

void emitter_update(emitter_s* e, float dt) {
    assert(e && dt >= 0.0f);

    particles_update(&e->particles, dt);
}

void emitter_emit(emitter_s* e, float dt) {
    assert(e && e->emit && dt >= 0.0f);
    
    e->emission_accum += e->emission_rate * dt;

    while (e->emission_accum >= 1.0f &&
           e->particles.num_particles < e->max_particles) {
        e->emit(e);
        e->emission_accum -= 1.0f;
    }
}


