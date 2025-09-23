#include "particles.h"

#include <stdlib.h>
#include <assert.h>


static void particles_init(particles_s* p, const particles_desc_s* desc) {
    assert(p && desc);
    
    *p = (particles_s){
        .num_particles = 0,
        .positions = malloc(desc->max_particles * sizeof(vec3s)),
        .velocities = malloc(desc->max_particles * sizeof(vec3s)),
        .lifetimes = malloc(desc->max_particles * sizeof(float)), 
        .colors = malloc(desc->max_particles * sizeof(vec4s)), 
        .start_color = desc->start_color,
        .end_color = desc->end_color
    };

    assert(
        p->positions && 
        p->velocities && 
        p->lifetimes && 
        p->colors
    );
}

static void particles_deinit(particles_s* p) {
    if (p) {
        free(p->positions);
        free(p->velocities);
        free(p->lifetimes);
        free(p->colors);
        *p = (particles_s){ };
    }
}

static void particles_update(particles_s* p, float dt) {
    assert(p && dt >= 0.0f);
    
    size_t i = 0;
    while (i < p->num_particles) {
        p->lifetimes[i] -= dt;

        // if lifetime expired, remove particle by swapping with the last one
        if (p->lifetimes[i] <= 0.0f) {
            p->positions[i] = p->positions[p->num_particles - 1];
            p->velocities[i] = p->velocities[p->num_particles - 1];
            p->lifetimes[i] = p->lifetimes[p->num_particles - 1];
            p->colors[i] = p->colors[p->num_particles - 1];
            p->num_particles--;
            continue;
        }

        p->positions[i] = glms_vec3_add(p->positions[i], glms_vec3_scale(p->velocities[i], dt));

        assert(p->lifetimes[i] > 0.0f);
        vec4s delta_color = glms_vec4_divs(
            glms_vec4_sub(p->end_color, p->start_color), 
            p->lifetimes[i]
        );

        p->colors[i] = glms_vec4_add(
            p->colors[i], 
            glms_vec4_scale(delta_color, dt)
        );

        i++;
    }
}

static void particles_add(particles_s* p, const particle_desc_s* desc) {
    assert(p && desc);
    
    const size_t idx = p->num_particles++;
    p->positions[idx] = desc->position;
    p->velocities[idx] = desc->velocity;
    p->lifetimes[idx] = desc->lifetime;
    p->colors[idx] = p->start_color;
}

void emitter_init(emitter_s* e, const emitter_desc_s* desc) {
    assert(e && desc);
    
    *e = (emitter_s){
        .emission_rate = desc->emission_rate,
        .emission_accum = 0.0f,
        .max_particles = desc->particles_desc->max_particles,
        .emit = desc->emit
    };
    particles_init(&e->particles, desc->particles_desc);
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

void emitter_add_particle(emitter_s* e, const particle_desc_s* desc) {
    assert(e && desc);
    particles_add(&e->particles, desc);
}
