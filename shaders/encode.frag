// Encoder or Modulator
// This pass converts RGB colors on iChannel0 to
// a YIQ (NTSC) Composite signal.

#define PI   3.14159265358979323846
#define TAU  6.28318530717958647693

// Hue adjustment, I've found that rotating the hue
// by -0.15° yields nicer colors, though its not really
// necessary. Change this value to your liking
#define HUE_ADJUSTMENT (-0.15)

const mat3 rgb_to_yiq = mat3(0.299, 0.596, 0.211,
                             0.587,-0.274,-0.523,
                             0.114,-0.322, 0.312);

// Alternative RGB to YIQ matrix
// const mat3 rgb_to_yiq = mat3(0.299, 0.587, 0.114,
//                              0.596,-0.274,-0.322,
//                              0.211,-0.523, 0.312);
float hash12(vec2 p) {
	vec3 p3  = fract(vec3(p.xyx) * .1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float hash11(float p) {
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash13(vec3 p3) {
	p3  = fract(p3 * .1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    // Chroma encoder oscillator frequency 
    float fc = iResolution.x;

    float t = float(fragCoord.x);
    
    // Tuning error offset (not necessary)
    //float offset = hash12(vec2(fragCoord.y, float(iFrame)));
    vec2 pos = fragCoord.xy / iResolution.xy;

    vec3 rgb = texture(iChannel0, pos).rgb;
    vec3 yiq = rgb_to_yiq * rgb;
    
    // This is a phase offset based on the Y coordinate of the pixel
    // Its not necessary for decoding, it just adds some movement
    // to otherwise completely static images.
    float y_phase_offset = (float((int(fragCoord.y) & 0x1)) * 0.5);
    
    // Final oscillator angle
    float f = fc * t + y_phase_offset + HUE_ADJUSTMENT - (fragCoord.y / 3.0) + (float(iFrame & 0x1) * PI);

    float i = yiq.y * cos(f), // I signal
          q = yiq.z * sin(f); // Q signal

    float c = yiq.x + (i + q); // Composite

    // Return a grayscale representation of the signal
    fragColor = vec4(vec3(c), 1.0);
}