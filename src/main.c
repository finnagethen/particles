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
#include "quad.h"
#include "texture.h"

#include "instancing.glsl.h"


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

    emitter_add_particle(e, &(particle_desc_s){
        .position = (vec3s){ },
        .velocity = (vec3s){ 
            .x = frand_range(-0.5f, 0.5f), 
            .y = frand_range(1.0f, 3.0f), 
            .z = frand_range(-0.5f, 0.5f) 
        },
        .lifetime = frand_range(1.0f, 5.0f)
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
        .emission_rate = 50.0f,
        .emit = emit_particle, 
        .particles_desc = &(particles_desc_s){
            .max_particles = 1024,
            .start_color = (vec4s){ .r = 1.0f, .g = 0.5f, .b = 0.0f, .a = 1.0f },
            .end_color = (vec4s){ .r = 1.0f, .g = 0.0f, .b = 0.0f, .a = 0.0f }
        }
    });

    // a pass action for the default render pass
    state.pass_action = (sg_pass_action){
        .colors[0] = {
            .load_action = SG_LOADACTION_CLEAR,
            .clear_value = { 0.5f, 0.5f, 0.5f, 1.0f }
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

    // empty, dynamic instance-data vertex buffer, goes into vertex-buffer-slot 1 and 2
    state.bind.vertex_buffers[1] = sg_make_buffer(&(sg_buffer_desc){
        .size = state.emitter.max_particles * sizeof(vec3s),
        .usage.stream_update = true,
        .label = "instance-pos-data"
    });

    state.bind.vertex_buffers[2] = sg_make_buffer(&(sg_buffer_desc){
        .size = state.emitter.max_particles * sizeof(vec4s),
        .usage.stream_update = true,
        .label = "instance-color-data"
    });

    // a texture for the particles
    sg_image img = sg_make_image(&(sg_image_desc){
        .width = TEXTURE_WIDTH,
        .height = TEXTURE_HEIGHT,
        .data.mip_levels[0] = SG_RANGE(texture),
        .label = "particle-image"
    });
    state.bind.views[VIEW_tex] = sg_make_view(&(sg_view_desc){
        .texture = { .image = img }, 
        .label = "particle-texture-view"
    });

    // a sampler for the texture
    state.bind.samplers[SMP_smp] = sg_make_sampler(&(sg_sampler_desc){
        .label = "particle-sampler"
    });

    // a shader
    sg_shader shd = sg_make_shader(instancing_shader_desc(sg_query_backend()));

    // a pipeline object
    state.pip = sg_make_pipeline(&(sg_pipeline_desc){
        // vertex buffer at slot 1 and 2 must step per instance
        .layout = {
            .attrs = {
                [ATTR_instancing_pos] = {
                    .format = SG_VERTEXFORMAT_FLOAT3,
                    .buffer_index = 0, 
                    .offset = offsetof(vertex_s, pos)
                },
                [ATTR_instancing_uv0] = {
                    .format = SG_VERTEXFORMAT_FLOAT2,
                    .buffer_index = 0, 
                    .offset = offsetof(vertex_s, uv)
                },
                [ATTR_instancing_inst_pos] = {
                    .format = SG_VERTEXFORMAT_FLOAT3,
                    .buffer_index = 1
                }, 
                [ATTR_instancing_inst_color] = {
                    .format = SG_VERTEXFORMAT_FLOAT4,
                    .buffer_index = 2
                }
            },
            .buffers[0].stride = sizeof(vertex_s), 
            .buffers[1].step_func = SG_VERTEXSTEP_PER_INSTANCE,
            .buffers[2].step_func = SG_VERTEXSTEP_PER_INSTANCE,
        },
        .shader = shd,
        .index_type = SG_INDEXTYPE_UINT16,
        .cull_mode = SG_CULLMODE_BACK,
        .face_winding = SG_FACEWINDING_CCW,
        .depth = {
            .compare = SG_COMPAREFUNC_LESS_EQUAL,
            .write_enabled = false,
        },
        .colors[0] = {
            .blend = {
                .enabled = true,
                .src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA,
                .dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .src_factor_alpha = SG_BLENDFACTOR_ONE,
                .dst_factor_alpha = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA,
                .op_rgb = SG_BLENDOP_ADD,
                .op_alpha = SG_BLENDOP_ADD,
            }
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

        sg_update_buffer(state.bind.vertex_buffers[2], &(sg_range){
            .ptr = state.emitter.particles.colors,
            .size = state.emitter.particles.num_particles * sizeof(vec4s)
        });
    }

    // model-view-projection matrix
    const mat4s proj = glms_perspective(
        glm_rad(60.0f), 
        sapp_widthf() / sapp_heightf(), 
        0.01f, 50.0f
    );
    
    // rotate the camera around the origin
    const float radius = 5.0f;
    const float cam_x = sinf(sapp_frame_count() * 0.05f) * radius;
    const float cam_z = cosf(sapp_frame_count() * 0.05f) * radius;
    const mat4s view = glms_lookat(
        (vec3s){ .x = cam_x, .y = 1.5f, .z = cam_z },
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
        .window_title = "Particle System",
        .logger.func = slog_func,
    };
}
