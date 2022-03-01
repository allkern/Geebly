// Input on Buffer A or Buffer A's iChannel0

// This final pass lowpasses (blurs) the image some.
// Also applies some small sync noise

// Developer's note:
// I found that most NTSC codecs/decoders on this site are pretty
// hard to understand.
//
// I made this filter to document a lot on how these filters work
// as information on the matter is scarce even on the internet.
// That is why this code is littered with comments everywhere.
//
// As for actual documents or specifications:
// The ITU-R BT.1700 recommendation goes into a lot of detail on how
// an encoder can be implemented both in Hardware and Software.
// (its "usefulness" for software implementations was probably unintentional)
//
// Here's a link to that recommendation's papers download page:
// - https://www.itu.int/rec/R-REC-BT.1700-0-200502-I/en
//
// I hope all of this can help you implement your own NTSC/VHS
// filter!

// To-do list:
// - Filter out chroma from luma, decode them separately
// - Improve lowpass filters overall using window functions

// Strength of the 2D low-pass (blur) filter
#define FIR_SIZE_H 1
#define FIR_SIZE_V 1

float hash12(vec2 p) {
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    vec3 s;
    
    float counter = 0.0;
    
    //float xoffset = hash12(vec2(fragCoord.y, float(iFrame))) * 1.0;

    for (int i = -FIR_SIZE_H; i <= FIR_SIZE_H; i++) {
        for (int y = -FIR_SIZE_V; y <= FIR_SIZE_V; y++) {
            vec2 uv = vec2(fragCoord.x + float(i), fragCoord.y + float(y)) / iResolution.xy;

            s = s + texture(iChannel0, uv).xyz;
            counter += 1.0;
        }
    }
    
    //s /= (float(FIR_SIZE) * 16.0) + 1.0;
    s /= counter;
    fragColor = vec4(s, 1.0);
}