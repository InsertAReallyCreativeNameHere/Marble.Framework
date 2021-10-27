#include "bgfx_shader.sh"

uniform mat4 u_transformData;

void main()
{
    gl_FragColor = u_transformData[2];
}
