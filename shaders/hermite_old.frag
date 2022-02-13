// Adapted from https://www.shadertoy.com/view/7dyXWG

// This pass applies a simulation of analog overshoot, which
// is the cause for ghosting artifacts around edges (sharp transitions)
// This is not necessary for encoding/decoding.

// These two define the tension and bias parameters
// of the Hermite interpolation function
// Values closer to 0.5 on both parameters yield
// more aggressive ghosting artifacts
#define HERMITE_TENSION 0.25
#define HERMITE_BIAS 0.75

float hermite(float y0, float y1, float y2, float y3, float m, float tension, float bias) {
    float m2 = m*m,
          m3 = m2*m,
          m0 =      (y1-y0)*(1.0+bias)*(1.0-tension)/2.0;
          m0 = m0 + (y2-y1)*(1.0-bias)*(1.0-tension)/2.0;
    float m1 =      (y2-y1)*(1.0+bias)*(1.0-tension)/2.0;
          m1 = m1 + (y3-y2)*(1.0-bias)*(1.0-tension)/2.0;
    float a0 =  2.0*m3 - 3.0*m2 + 1.0,
          a1 =      m3 - 2.0*m2 + m,
          a2 =      m3 -     m2,
          a3 = -2.0*m3 + 3.0*m2;
     
     return (a0*y1+a1*m0+a2*m1+a3*y2);
}

void main()
{
    // Normalized pixel coordinates (from 0 to 1)
    vec2 fcm1 = vec2(fragCoord.x-4.0, fragCoord.y)/iResolution.xy,
         fcm0 = vec2(fragCoord.x-0.0, fragCoord.y)/iResolution.xy,
         fcp1 = vec2(fragCoord.x+4.0, fragCoord.y)/iResolution.xy,
         fcp2 = vec2(fragCoord.x+8.0, fragCoord.y)/iResolution.xy;
    
    vec3 y0 = texture(iChannel0, fcm1).xyz,
         y1 = texture(iChannel0, fcm0).xyz,
         y2 = texture(iChannel0, fcp1).xyz,
         y3 = texture(iChannel0, fcp2).xyz;
    
    vec3 o = y1;
         
    float l = HERMITE_TENSION, y = HERMITE_BIAS;
         
    vec3 f = vec3(
        hermite(y0.x, y1.x, y2.x, y3.x, l, y, y * 50.0),
        hermite(y0.y, y1.y, y2.y, y3.y, l, y, y * 50.0),
        hermite(y0.z, y1.z, y2.z, y3.z, l, y, y * 50.0)
    );

    fragColor = vec4(f, 1.0);
}