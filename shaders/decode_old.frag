// Decoder or Demodulator
// This pass takes the Composite signal generated on Buffer B
// and decodes it

// Also applies some noise and generates black bars on the sides
// of the video frame.
// These two are post-processing effects, and thus, not necessary
// for decoding/encoding.

#define PI   3.14159265358979323846
#define TAU  6.28318530717958647693

// Size of the black bars on the sides, as a fraction
// of the entire horizontal resolution
#define BARS_SIZE                (1.0 / 500.0)

// Noise visibility factor
// Values between 1.0 and 2.0 are the most realistic
#define LUMA_NOISE_FACTOR        1.0
#define LUMA_NOISE_DENSITY       0.15
#define CHROMA_NOISE_FACTOR      0.0
#define CHROMA_NOISE_DENSITY     0.0
#define BRIGHTNESS_FACTOR        1.0

// The decoded IQ signals get multiplied by this
// factor. Bigger values yield more color saturation
#define CHROMA_SATURATION_FACTOR 2.0

// Size of the decoding FIR filter. bigger values
// yield more smuggly video and are more expensive
#define DECODER_FIR_SIZE         1

const mat3 yiq_to_rgb = mat3(1.000, 1.000, 1.000,
                             0.956,-0.272,-1.106,
                             0.621,-0.647, 1.703);

// Noise (not required)
float hash13(vec3 p3) {
	p3  = fract(p3 * .1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    // Tuning error (not necessary)
    // Chroma decoder oscillator frequency
    float fc = iResolution.x;
    
    float counter = 1.0;
    
    // Sum and decode NTSC samples
    // This is essentially a simple averaging filter
    // that happens to be weighted by two cos and sin
    // oscillators at a very specific frequency
    vec3 yiq;
    
    for (int d = -DECODER_FIR_SIZE; d < DECODER_FIR_SIZE; d++) {
        vec2 pos = vec2(fragCoord.x + float(d), fragCoord.y);

        vec3 s = texture(iChannel0, pos / iResolution.xy).rgb;

        float t = fc * (fragCoord.x + float(d)) - (fragCoord.y / 3.0) + (float(iFrame & 0x1) * PI);
        
        yiq += s * vec3(BRIGHTNESS_FACTOR, cos(t), sin(t));

        counter++;
    }

    yiq /= counter;
    
    // Everything below this line (excluding the YIQ to RGB conversion)
    // isn't absolutely necessary. The core of the decoder is above
    // this line.

    yiq.x += 0.1;

    // Saturate chroma (IQ)
    yiq.yz *= CHROMA_SATURATION_FACTOR;
    
    // Noise
    float f = hash13(vec3(fragCoord.xy, iFrame)),
          af = (f - 0.5) * 0.5;
    yiq.xyz *= (f > (1.0 - LUMA_NOISE_DENSITY)) ? (f * LUMA_NOISE_FACTOR) : 1.0;
    
    if (abs(af) > (1.0 - CHROMA_NOISE_DENSITY)) yiq.yz += af * CHROMA_NOISE_FACTOR;
    
    // Black bars
    float u = fragCoord.x / iResolution.x;
    
    if (!((u >= BARS_SIZE) && (u <= (1.0 - BARS_SIZE)))) {
        yiq.yz = vec2(0.0);
        yiq.x = 0.01;
    }

    fragColor = vec4((yiq_to_rgb * yiq), 1.0);
}