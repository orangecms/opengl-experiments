R"(
#version 320 es
layout(location = 0)

in vec2 a_position;
in vec2 a_texCoord;
out vec2 v_texCoord;
out vec4 v_color;
uniform mediump float u_time;

mediump float speed = .0007;

void main() {
  mediump float t = u_time * speed;
  mat3 the_matrix = mat3(
    cos(t), -sin(t),  .2,
    sin(t), -cos(t),  .2,
        .5,      .5, 1.0
  );
  gl_Position = vec4((vec3(a_position, 1)).xy, 0, 1);
  gl_Position.xyz += vec3 (cos(t), .4 + sin(t), 1);
  gl_Position.xyz *= the_matrix;
  gl_Position.w = 2.6 + sin(t);
  v_texCoord = a_texCoord;
  v_color = gl_Position * 0.5 + 0.5;
}
)"
