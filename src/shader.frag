varying vec3 color;
varying vec3 norm;
varying vec3 light;

void main(void)
{
	vec3 tri_normal = normalize(cross(dFdx(norm), dFdy(norm)));

	float intensity = max(0.0, dot(tri_normal, normalize(light)));

	gl_FragColor = vec4(color, 1.0);
	gl_FragColor.rgb *= intensity;
}
