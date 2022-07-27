void main() {
    vec2 flipped = vec2(fragCoord.x, iResolution.y - fragCoord.y) / iResolution.xy;
    vec2 scaled = flipped * (iResolution.xy / iTargetResolution.xy);

    fragColor = texture(iChannel0, scaled);
}