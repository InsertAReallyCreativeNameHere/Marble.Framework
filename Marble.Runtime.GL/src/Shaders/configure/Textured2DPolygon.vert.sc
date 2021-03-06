$input a_position, a_texcoord0
$output v_texcoord0

#include "bgfx_shader.sh"

uniform mat4 u_transformData;

void main()
{
    vec2 pos = a_position;

    pos.x *= u_transformData[1].x;
    pos.y *= u_transformData[1].y;
    pos.x += u_transformData[0].z;
    pos.y += u_transformData[0].w;

    float s = sin(u_transformData[1].z);
    float c = cos(u_transformData[1].z);

    float x = pos.x;
    float y = pos.y;

    pos.x = x * c + y * s;
    pos.y = y * c - x * s;
    
    pos.x += u_transformData[0].x;
    pos.y += u_transformData[0].y;

	gl_Position = mul(u_modelViewProj, vec4(pos.x, pos.y, u_transformData[1][3], 1.0));

    v_texcoord0 = a_texcoord0;
}
