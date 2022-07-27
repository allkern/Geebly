// Simple NTSC codec
// Change Buffer A to whatever you want

#define FIR_SIZE 5
#define PI   3.14159265358979323846

float blackman(float n, float N) {
    float a0 = (1.0 - 0.16) / 2.0;
    float a1 = 1.0 / 2.0;
    float a2 = 0.16 / 2.0;
    
    return a0 - (a1 * cos((2.0 * PI * n) / N)) + (a2 * cos((4.0 * PI * n) / N)) * 1.0;
}

void main() {
    vec3 rgb = vec3(0.0);
    
    float counter = 0.0;
    
    for (int y = -3; y < 0; y++) {
        for (int d = -FIR_SIZE; d < FIR_SIZE; d++) {
            vec2 pos = vec2(fragCoord.x + float(d), fragCoord.y + float(y));

            vec3 s = texture(iChannel0, pos / iResolution.xy).rgb;
        
            // Apply Blackman window for smoother colors
            float window = blackman(float(d + FIR_SIZE), float(FIR_SIZE * 2 + 1)); 

            rgb += s * window;

            counter++;
        }
    }
    
    rgb /= counter;
    rgb *= 5.0;

    fragColor = vec4(rgb, 1.0);
}