#version 400

uniform sampler2D VelocityTex;
uniform sampler2D PressureTex;
uniform sampler2D FrontierTex;
uniform vec2 Size;

out vec4 Velocity;
out vec4 Pressure;


void main(void)
{
    ivec2 pos = ivec2(gl_FragCoord.xy);

    float alpha = texelFetch(FrontierTex, pos, 0).x;


    if(alpha == 1.0)
    {
        float curr;
        float accum = 0.0;
        vec4 moyVelocity = vec4(0.0);
        vec4 moyPressure = vec4(0.0);

        ivec2 dir[4];
        dir[0] = ivec2(-1,  0);
        dir[1] = ivec2( 1,  0);
        dir[2] = ivec2( 0, -1);
        dir[3] = ivec2( 0,  1);

        for(int i=0; i<4; ++i)
        {
            curr   = 1.0 - texelFetchOffset(FrontierTex, pos, 0, dir[i]).x;
            moyVelocity += texelFetchOffset(VelocityTex, pos, 0, dir[i]) * curr;
            moyPressure += texelFetchOffset(PressureTex, pos, 0, dir[i]) * curr;
            accum += curr;
        }

        if(accum != 0.0)
        {
            Velocity = -moyVelocity / accum;
            Pressure = moyPressure / accum;
        }
        else
        {
            Velocity = vec4(0.0);
            Pressure = texelFetch(PressureTex, pos, 0);
        }
    }
    else
    {
        Velocity = texelFetch(VelocityTex, pos, 0);
        Pressure = texelFetch(PressureTex, pos, 0);
    }
}
