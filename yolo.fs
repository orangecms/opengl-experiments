R"(
#version 320 es
out mediump vec3 color;

mediump float speed = .001;
uniform mediump float u_time;

void main()
{
  mediump float t = u_time * speed;
  color = vec3(.5,sin(t),cos(t));
}
)"
