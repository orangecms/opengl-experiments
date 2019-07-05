R"(
#version 320 es
layout(location = 0)

in vec2 a_position;
in vec2 a_texCoord;
out vec2 v_texCoord;
out vec4 v_color;

void main() {
  gl_Position = vec4((vec3(a_position, 1)).xy, 0, 1);
  v_texCoord = a_texCoord;
  v_texCoord.y = 1.0 - a_texCoord.y;
  v_color = gl_Position * 0.5 + 0.5;
}
)"
