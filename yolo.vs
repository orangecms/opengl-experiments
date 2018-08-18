R"(
#version 320 es
layout(location = 0) in vec3 vertexPosition_modelspace;

mediump float speed = .001;
uniform mediump float u_time;

void main() {
  mediump float t = u_time * speed;
  mat3 the_matrix = mat3(
    cos(t), -sin(t),  .2,
    sin(t), -cos(t),  .2,
        .5,      .5, 1.0
  );
  gl_Position.xyz = vertexPosition_modelspace;
  gl_Position.xyz += vec3 (cos(t), .4 + sin(t), 1);
  gl_Position.xyz *= the_matrix;
  gl_Position.w = 2.6 + sin(t);
}
)"
