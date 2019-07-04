R"(
#version 320 es
precision mediump float;
in vec2 v_texCoord;
uniform sampler2D u_image;

in vec4 v_color;
out vec4 FragColor;

float speed = .001;
uniform float u_time;

void main()
{
  float t = u_time * speed;
  vec4 texture = texture2D(u_image, v_texCoord);
  vec4 color = texture * v_color * (v_texCoord.x/v_texCoord.y);
  FragColor = color + 0.1 * vec4(.5, sin(t), cos(t), 1);
}
)"
