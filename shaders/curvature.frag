vec2 curve(vec2 coord, float bend) {
	// put in symmetrical coords
	coord = (coord - 0.5) * 2.0;

	coord *= 1.1;	

	// deform coords
	coord.x *= 1.0 + pow((abs(coord.y) / bend), 2.0);
	coord.y *= 1.0 + pow((abs(coord.x) / bend), 2.0);

	// transform back to 0.0 - 1.0 space
	coord  = (coord / 2.0) + 0.5;

	return coord;
}

void main() {
    vec2 remap = curve(fragCoord.xy / iResolution.xy, 4.0);
    vec4 baseColor = texture(iChannel0, remap);
    
    if (remap.x < 0.0 || remap.y < 0.0 || remap.x > 1.0 || remap.y > 1.0){
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    } else {
        fragColor = baseColor;
    }
}