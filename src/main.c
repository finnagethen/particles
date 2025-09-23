#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define SOKOL_IMPL
#define SOKOL_GLCORE

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_log.h"

#include "cglm/struct.h"

#include "particles.h"

#include "instancing.glsl.h"


#define MAX_PARTICLES 1024

static struct {
    sg_pass_action pass_action;
    sg_pipeline pip;
    sg_bindings bind;

    emitter_s emitter;
} state;

static float frand_range(float min, float max) {
    const float r = (float)rand() / (float)RAND_MAX;
    return min + r * (max - min);
}

static void emit_particle(emitter_s* e) {
    assert(e);

    particles_add(&e->particles, &(particle_desc_s){
        .position = (vec3s){ .x = 0.0f, .y = 0.0f, .z = 0.0f },
        .velocity = (vec3s){ 
            .x = frand_range(-0.5f, 0.5f), 
            .y = frand_range(1.0f, 3.0f), 
            .z = frand_range(-0.5f, 0.5f) 
        },
        .lifetime = frand_range(1.0f, 3.0f)
    });
}

static void init(void) {
    sg_setup(&(sg_desc){
        .environment = sglue_environment(),
        .logger.func = slog_func,
    });

    // seed random number generator
    srand((unsigned int)time(nullptr));

    // initialize the emitter
    emitter_init(&state.emitter, &(emitter_desc_s){
        .emission_rate = 20.0f,
        .max_particles = MAX_PARTICLES,
        .emit = emit_particle
    });

    // a pass action for the default render pass
    state.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.0f, 0.0f, 0.0f, 1.0f }
        }
    };

    // vertex buffer for static geometry, goes into vertex-buffer-slot 0
    state.bind.vertex_buffers[0] = sg_make_buffer(&(sg_buffer_desc){
        .data = SG_RANGE(quad_vertices), 
        .label = "geometry-vertices"
    });

    // index buffer for static geometry
    state.bind.index_buffer = sg_make_buffer(&(sg_buffer_desc){
        .usage.index_buffer = true,
        .data = SG_RANGE(quad_indices),
        .label = "geometry-indices"
    });

    // empty, dynamic instance-data vertex buffer, goes into vertex-buffer-slot 1
    state.bind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
        .size = state.emitter.max_particles * sizeof(vec3s),
        .usage.stream_update = true,
        .label = "instance-data"
    });

    // a shader
    sg_shader shd = sg_make_shader(instancing_shader_desc(sg_query_backend()));

    // a pipeline object
    state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        // vertex buffer at slot 1 must step per instance
        .layout = {
            .buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE,
            .attrs = {
                [ATTR_instancing_pos] = {
                    .format = SG_VERTEXFORMAT_FLOAT3,
                    .buffer_index = 0
                },
                [ATTR_instancing_inst_pos] = {
                    .format = SG_VERTEXFORMAT_FLOAT3,
                    .buffer_index = 1
                }
            }
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_BACK,
        .face_winding = SG_FACEWINDING_CCW,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = true,
        },
        .label = "instancing-pipeline"
    });
}

static void frame(void) {
    const float dt = (float)(sapp_frame_duration());

    // emit new particles
    emitter_emit(&state.emitter, dt);

    // update emitter (which updates the particles)
    emitter_update(&state.emitter, dt);

    // update instance data
    if (state.emitter.particles.num_particles > 0) {
        sg_update_buffer(state.bind.vertex_buffers[1], &(sg_range){
            .ptr = state.emitter.particles.positions,
            .size = state.emitter.particles.num_particles * sizeof(vec3s)
        });
    }

    // model-view-projection matrix
    const mat4s proj = glms_perspective(
        glm_rad(60.0f), 
        sapp_widthf() / sapp_heightf(), 
        0.01f, 50.0f
    );
    const mat4s view = glms_lookat(
        (vec3s){ .x = 0.0f, .y = 1.5f, .z = 8.0f },
        (vec3s){ .x = 0.0f, .y = 0.0f, .z = 0.0f },
        (vec3s){ .x = 0.0f, .y = 1.0f, .z = 0.0f }
    );

    vs_params_t vs_params;
    memcpy(&vs_params.model, glms_mat4_identity().raw, sizeof(mat4s)); 
    memcpy(&vs_params.view, view.raw, sizeof(mat4s));
    memcpy(&vs_params.proj, proj.raw, sizeof(mat4s));

    // ...and draw
    sg_begin_pass(&(sg_pass){
        .action = state.pass_action,
        .swapchain = sglue_swapchain()
    });
    sg_apply_pipeline(state.pip);
    sg_apply_bindings(&state.bind);
    sg_apply_uniforms(UB_vs_params, &SG_RANGE(vs_params));
    sg_draw(0, 6, state.emitter.particles.num_particles);
    sg_end_pass();
    sg_commit();
}

static void cleanup(void) { 
    emitter_deinit(&state.emitter);
    sg_shutdown(); 
}

static void event(const sapp_event *e) {
    if (e->type == SAPP_EVENTTYPE_KEY_DOWN) {
        if (e->key_code == SAPP_KEYCODE_ESCAPE) {
            sapp_request_quit();
        }
    }
}

sapp_desc sokol_main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    return (sapp_desc){
        .init_cb = init,
        .frame_cb = frame,
        .cleanup_cb = cleanup,
        .event_cb = event,
        .width = 800,
        .height = 600,
        .sample_count = 4,
        .window_title = "Instancing (sokol-app)",
        .icon.sokol_default = true,
        .logger.func = slog_func,
    };
}
