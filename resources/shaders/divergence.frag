#version 400

uniform sampler2D VelocityTex;
uniform vec2 Size;
uniform float HalfrDx;

out vec4 FragOut;

void main(void)
{
    ivec2 pos = ivec2(gl_FragCoord.xy);

    vec4 vL = texelFetch(VelocityTex, pos - ivec2(1, 0), 0);
    vec4 vR = texelFetch(VelocityTex, pos + ivec2(1, 0), 0);
    vec4 vB = texelFetch(VelocityTex, pos - ivec2(0, 1), 0);
    vec4 vT = texelFetch(VelocityTex, pos + ivec2(0, 1), 0);

    float div = HalfrDx * ((vR.x - vL.x) + (vT.y - vB.y));
    FragOut = vec4(div, 0, 0, 0);
}
