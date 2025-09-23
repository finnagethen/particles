@vs vs
layout(binding=0) uniform vs_params {
    mat4 model;
    mat4 view;
    mat4 proj;
};

in vec3 pos;
in vec3 inst_pos;
in vec4 inst_color;

out vec4 color;

void main() {
    vec3 cam_right = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cam_up = vec3(view[0][1], view[1][1], view[2][1]);

    vec3 world_pos = inst_pos 
        + pos.x * cam_right 
        + pos.y * cam_up;

    gl_Position = proj * view * model * vec4(world_pos, 1.0f);
    color = inst_color;
}
@end

@fs fs
in vec4 color;
out vec4 frag_color;
void main() {
    frag_color = color;
}
@end

@program instancing vs fs
