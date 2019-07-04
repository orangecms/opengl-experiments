R"(
#version 320 es
out mediump vec3 color;

mediump float speed = .001;
uniform mediump float u_time;
uniform sampler2D u_image;

void main()
{
  mediump float t = u_time * speed;
  mediump vec2 coords = vec2(1.0, 1.0);
  mediump vec4 texture = texture2D(u_image, coords);
  color = texture.rgb + 0.4 * vec3(.5,sin(t),cos(t));
}
)"
