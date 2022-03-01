// Decoder or Demodulator
// This pass takes the Composite signal generated on Buffer B
// and decodes it

// Also applies some noise and generates black bars on the sides
// of the video frame.
// These two are post-processing effects, and thus, not necessary
// for decoding/encoding.

#define DECODE_LUMA_CHROMA_SEPARATELY

#define PI   3.14159265358979323846
#define TAU  6.28318530717958647693

// Size of the black bars on the sides, as a fraction
// of the entire horizontal resolution
#define BARS_SIZE                (1.0 / 500.0)

// Noise visibility factor
// Values between 1.0 and 2.0 are the most realistic
#define LUMA_NOISE_FACTOR        0.0
#define LUMA_NOISE_DENSITY       0.0
#define CHROMA_NOISE_FACTOR      0.0
#define CHROMA_NOISE_DENSITY     0.0

#define BRIGHTNESS_FACTOR        34.0

// The decoded IQ signals get multiplied by this
// factor. Bigger values yield more color saturation
#define CHROMA_SATURATION_FACTOR 64.0

// Size of the decoding FIR filter. bigger values
// yield more smuggly video and are more expensive
#define CHROMA_DECODER_FIR_SIZE         20
#define LUMA_DECODER_FIR_SIZE           1

const mat3 yiq_to_rgb = mat3(1.000, 1.000, 1.000,
                             0.956,-0.272,-1.106,
                             0.621,-0.647, 1.703);

float hann(float i, int size, float phase) {
    return pow(sin((PI * (i + phase)) / float(size)), 2);
}

// Noise (not required)
float hash13(vec3 p3) {
	p3  = fract(p3 * .1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    // Chroma decoder oscillator frequency
    float fc = iResolution.x;
    
    float counter = 1.0;
    
    // Sum and decode NTSC samples
    // This is essentially a simple averaging filter
    // that happens to be weighted by two cos and sin
    // oscillators at a very specific frequency
    vec3 yiq;
    
    for (int d = -CHROMA_DECODER_FIR_SIZE; d < CHROMA_DECODER_FIR_SIZE; d++) {
        vec2 pos = vec2(fragCoord.x + float(d), fragCoord.y);

        vec3 s = texture(iChannel0, pos / iResolution.xy).rgb;

        float t = fc * (fragCoord.x + float(d)) - (fragCoord.y / 3.0) + (float(iFrame & 0x1) * PI);

#ifdef DECODE_LUMA_CHROMA_SEPARATELY
        yiq += s * vec3(0.0, cos(t) * 2.0, sin(t) * 2.0);
#else
        yiq += s * vec3(BRIGHTNESS_FACTOR, cos(t), sin(t));
#endif
        counter++;
    }

    yiq /= counter;
    
#ifdef DECODE_LUMA_CHROMA_SEPARATELY
    for (int d = -LUMA_DECODER_FIR_SIZE; d < LUMA_DECODER_FIR_SIZE; d++) {
        vec2 pos = vec2(fragCoord.x + float(d), fragCoord.y);

        vec3 s = texture(iChannel0, pos / iResolution.xy).rgb;

        float t = fc * (fragCoord.x + float(d)) - (fragCoord.y / 3.0) + (float(iFrame & 0x1) * PI);
        
        yiq += s * vec3(BRIGHTNESS_FACTOR, cos(t) * 0.25, sin(t) * 0.25) * hann(d, LUMA_DECODER_FIR_SIZE * 2, 0.0);

        counter++;
    }

    yiq /= counter;
#endif
    
    // Everything below this line (excluding the YIQ to RGB conversion)
    // isn't absolutely necessary. The core of the decoder is above
    // this line.

    // Saturate chroma (IQ)
    yiq.yz *= CHROMA_SATURATION_FACTOR;

    yiq.x += 0.05;
    
    // Noise
    // float f = hash13(vec3(fragCoord.xy, iTime)),
    //       af = (f - 0.5) * 0.5;
    // yiq.xyz *= (f > (1.0 - LUMA_NOISE_DENSITY)) ? (f * LUMA_NOISE_FACTOR) : 1.0;

    // yiq.x += f * 0.1;
    
    // if (abs(af) > (1.0 - CHROMA_NOISE_DENSITY)) yiq.yz += af * CHROMA_NOISE_FACTOR;
    
    // Black bars
    float u = fragCoord.x / iResolution.x;
    
    if (!((u >= BARS_SIZE) && (u <= (1.0 - BARS_SIZE)))) {
        yiq.yz = vec2(0.0);
        yiq.x = 0.01;
    }

    fragColor = vec4((yiq_to_rgb * yiq), 1.0);
}