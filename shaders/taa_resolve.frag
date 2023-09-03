#version 450

#include "lib/utils.glsl"


// -------------------------------- I/O --------------------------------

layout(set = 0, binding = 0) uniform sampler2D in_depth;
layout(set = 0, binding = 1) uniform sampler2D in_color;
layout(set = 0, binding = 2) uniform sampler2D in_prev;

layout(set = 0, binding = 3) uniform CurrentCameraData {
    Camera current_cam;
};

layout(set = 0, binding = 4) uniform PrevCameraData {
    Camera prev_cam;
};

layout(set = 0, binding = 5) uniform Settings_Inline {
    uint flags;
    float blending_factor;
};

layout(location = 0) in vec2 in_uv;

layout(location = 0) out vec4 out_color;


// -------------------------------- Utils --------------------------------

struct ClampingInfo {
    vec3 max_color;
    vec3 min_color;
};

void update_clamping(inout ClampingInfo info, vec3 color) {
    info.max_color = max(info.max_color, color);
    info.min_color = min(info.min_color, color);
}

ClampingInfo gather_clamping_info(sampler2D in_color, vec2 uv) {
    ClampingInfo info;
    info.max_color = vec3(0.0);
    info.min_color = vec3(999999.0);

    update_clamping(info, textureOffset(in_color, uv, ivec2(-1, -1)).rgb);
    update_clamping(info, textureOffset(in_color, uv, ivec2(-1,  0)).rgb);
    update_clamping(info, textureOffset(in_color, uv, ivec2(-1,  1)).rgb);

    update_clamping(info, textureOffset(in_color, uv, ivec2( 0, -1)).rgb);
    update_clamping(info, textureOffset(in_color, uv, ivec2( 0,  0)).rgb);
    update_clamping(info, textureOffset(in_color, uv, ivec2( 0,  1)).rgb);

    update_clamping(info, textureOffset(in_color, uv, ivec2( 1, -1)).rgb);
    update_clamping(info, textureOffset(in_color, uv, ivec2( 1,  0)).rgb);
    update_clamping(info, textureOffset(in_color, uv, ivec2( 1,  1)).rgb);

    return info;
}



// -------------------------------- Main --------------------------------

const uint resolve_bit = 0x1;
const uint clamping_bit = 0x2;

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    const vec2 size = vec2(textureSize(in_color, 0).xy);
    const vec2 uv = gl_FragCoord.xy / size;

    bool sample_valid = true;

    vec2 prev_uv = uv;
    if((flags & resolve_bit) != 0) {
        const float depth = texelFetch(in_depth, coord, 0).x;
        const vec3 world_pos = unproject(uv, depth, current_cam.inv_unjittered_view_proj);
        const vec2 prev_uv = project(world_pos, prev_cam.unjittered_view_proj).xy;

        sample_valid = sample_valid && (prev_uv == saturate(prev_uv));
    }

    const vec3 current = texture(in_color, uv).rgb;
    vec3 prev = texture(in_prev, prev_uv).rgb;

    if((flags & clamping_bit) != 0 && sample_valid) {
        const ClampingInfo clamping_info = gather_clamping_info(in_color, uv);
        prev = clamp(prev, clamping_info.min_color, clamping_info.max_color);
    }


    out_color = vec4(mix(current, prev, sample_valid ? blending_factor : 0.0), 1.0);
}
