uniform vec3 colors[128];

/* vertex shader */
attribute float type;
attribute float blk_light;
varying vec3 color;
varying vec3 norm;
varying vec3 light;

void main(void) {
	/* light from above */
	vec3 lightpos = vec3(0.0, 1.0*blk_light, 0.0);
	norm = (gl_ModelViewMatrix * gl_Vertex).xyz;
	// eye-space light vector
	vec4 v = gl_ModelViewMatrix * gl_Vertex;
	light = lightpos - v.xyz;
	/* color lookup from type + apply light from sun */
	color = colors[int(type)] * (blk_light + 4.0)/5.0;
	gl_Position = ftransform();
}
