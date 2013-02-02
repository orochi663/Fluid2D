#version 400

uniform sampler2D VelocityTex;
uniform sampler2D HeatTex;
uniform vec2 Size;
uniform float HalfrDx;
uniform vec2 MousePos;

out vec4 Velocity;
out vec4 Heat;

void main(void)
{
    ivec2 pos = ivec2(gl_FragCoord.xy);

    vec4  hC = texelFetch(HeatTex, pos, 0);
    float hL = texelFetch(HeatTex, pos - ivec2(1, 0), 0).x;
    float hR = texelFetch(HeatTex, pos + ivec2(1, 0), 0).x;
    float hB = texelFetch(HeatTex, pos - ivec2(0, 1), 0).x;
    float hT = texelFetch(HeatTex, pos + ivec2(0, 1), 0).x;

    vec4 v = texelFetch(VelocityTex, pos, 0);
    v.y += HalfrDx * ((hL + hR + hB + hT) - hC.x) * 0.05;

    Velocity = v;


    if(distance(MousePos, gl_FragCoord.xy) < 10.0)
    {
        Heat =  vec4(1.0, 0, 0, 0);
    }
    else
    {
        Heat = hC;
    }
}
