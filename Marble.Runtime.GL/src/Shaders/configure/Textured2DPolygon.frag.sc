$input v_texcoord0

#include "bgfx_shader.sh"

SAMPLER2D(s_texColor, 0);

uniform mat4 u_transformData;

void main()
{
    vec4 color = texture2D(s_texColor, v_texcoord0.xy);
    gl_FragColor = u_transformData[2] * color;
}
