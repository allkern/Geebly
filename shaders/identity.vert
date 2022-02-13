void main() {
	gl_Position = gl_Vertex;
	vec2 coord = ((gl_Position.xy * iResolution.xy) + (iResolution.xy)) / 2;
	fragCoord = vec2(coord.x, coord.y);
}