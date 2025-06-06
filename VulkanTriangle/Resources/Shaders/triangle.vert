#version 450

// Per-vertex position & color calculated from gl_VertexIndex
layout(location = 0) out vec3 vColor;

void main()
{
    const vec2  positions[3] = vec2[3](
        vec2( 0.0, -0.5),   // bottom-center
        vec2( 0.5,  0.5),   // top-right
        vec2(-0.5,  0.5)    // top-left
    );

    const vec3  colors[3] = vec3[3](
        vec3(1.0, 0.0, 0.0),   // red
        vec3(0.0, 1.0, 0.0),   // green
        vec3(0.0, 0.0, 1.0)    // blue
    );

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    vColor      = colors[gl_VertexIndex];
}
