float hash11(float p)
{
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

void main()
{
    float start = hash11(float(iFrame)) * (iResolution.x * iResolution.y),
          end = hash11(float(iFrame + 1)) * (iResolution.x * iResolution.y);

    // Normalized pixel coordinates (from 0 to 1)
    float clock = fragCoord.x + (iResolution.x * fragCoord.y);

    if (end < start) {
        float temp = start;
        start = end;
        end = start;
    }
    
    if ((clock >= start) && (clock <= end)) {
        fragColor = vec4(vec3(0.0), 1.0);
    } else {
        fragColor = texture(iChannel0, fragCoord.xy / iResolution.xy);
    }
}