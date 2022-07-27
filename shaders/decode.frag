// Decoder or Demodulator
// This pass takes the Composite signal generated on Buffer B
// and decodes it

#define PI   3.14159265358979323846
#define TAU  6.28318530717958647693

#define BRIGHTNESS_FACTOR        0.75

// The decoded IQ signals get multiplied by this
// factor. Bigger values yield more color saturation
#define CHROMA_SATURATION_FACTOR 10.0

// Size of the decoding FIR filter. bigger values
// yield more smuggly video and are more expensive
#define CHROMA_DECODER_FIR_SIZE  60
#define LUMA_DECODER_FIR_SIZE  20

// YIQ to RGB matrix
const mat3 yiq_to_rgb = mat3(1.000, 1.000, 1.000,
                             0.956,-0.272,-1.106,
                             0.621,-0.647, 1.703);

float blackman(float n, float N) {
    float a0 = (1.0 - 0.16) / 2.0;
    float a1 = 1.0 / 2.0;
    float a2 = 0.16 / 2.0;
    
    return a0 - (a1 * cos((2.0 * PI * n) / N)) + (a2 * cos((4.0 * PI * n) / N)) * 1.0;
}

float sinc(float x) {
    x *= PI;
    
    return (x == 0.0) ? 1.0 : (sin(x) / x);
}

float sincf(float cutoff, float n) {
    float cut2 = 2.0 * cutoff;
    
    return cut2 * sinc(cut2 * n);
}

float windowed_sinc(float cutoff, float n, float N) {
    return sinc((2.0 * cutoff) * (n - ((N - 1.0) / 2.0))) * blackman(n, N);
}

float lowpass(float cutoff, float n) {
    float cut2 = 2.0 * cutoff;
    
    return cut2 * sinc(cut2 * n);
}

float bandpass(float lf, float hf, float n, float N) {
    return windowed_sinc(hf, n, N) - windowed_sinc(lf, n, N);
}

void main() {
    // Chroma decoder oscillator frequency
    float fc = 400.0;
    
    float counter = 0.0;
    
    // Sum and decode NTSC samples
    // This is essentially a simple averaging filter
    // that happens to be weighted by two cos and sin
    // oscillators at a very specific frequency
    vec3 yiq;
    
    // Decode Luma first
    for (int d = -LUMA_DECODER_FIR_SIZE; d < 0; d++) {
        vec2 pos = vec2(fragCoord.x + float(d), fragCoord.y);

        vec3 s = texture(iChannel0, pos / iResolution.xy).rgb;
        
        float filt = lowpass(0.05, float(d));

        yiq += s * vec3(float(LUMA_DECODER_FIR_SIZE) * 4.5, 0.0, 0.0) * filt;

        counter++;
    }
    
    // Then decode chroma
    for (int d = 0; d < CHROMA_DECODER_FIR_SIZE; d++) {
        vec2 pos = vec2(fragCoord.x + float(d) - float(CHROMA_DECODER_FIR_SIZE / 2), fragCoord.y);

        vec3 s = texture(iChannel0, pos / iResolution.xy).rgb;

        float t = fc * (fragCoord.x + float(d));
        
        // Apply Blackman window for smoother colors
        //float window = blackman(float(d + CHROMA_DECODER_FIR_SIZE), float(CHROMA_DECODER_FIR_SIZE * 2 + 1)); 
        float filt = bandpass(0.9999, 1.0, float(d), float(CHROMA_DECODER_FIR_SIZE));

        yiq += s * vec3(0.0, cos(t), sin(t)) * filt;

        counter++;
    }

    yiq /= counter;
    yiq.yz *= 5000.0;

    // Saturate chroma (IQ)
    yiq.yz *= CHROMA_SATURATION_FACTOR;
    yiq.x *= BRIGHTNESS_FACTOR;

    fragColor = vec4((yiq_to_rgb * yiq), 1.0);
}