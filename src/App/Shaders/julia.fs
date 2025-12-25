#version 330 core

in vec2 fragPos;
out vec4 fragCol;

uniform vec2 c;
uniform float radiusThresholdSquared;
uniform int maxIterations;
uniform int superSamplingRatio;

uniform float scale;
uniform vec2 resolution;
uniform vec2 center;

uniform vec3 aColor;
uniform vec3 bColor;
uniform vec3 cColor;
uniform vec3 dColor;

float rand(vec2 x){
	return fract(sin(dot(x, vec2(12.9898, 78.233))) * 43758.5453123);
}

vec3 pallete(float t) {
    return aColor + bColor * cos(6.28318 * (cColor * t + dColor));
}

float get_stability(vec2 z) {
	int iterations;
	for (iterations = 0; iterations < maxIterations; iterations++) {
		if (dot(z, z) > radiusThresholdSquared) break;
		z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
	}
	return float(iterations) / float(maxIterations);
}

void main() {
	vec2 z = center + fragPos * resolution / 2 * scale;
	vec3 color = pallete(get_stability(z));
	if (superSamplingRatio > 1) {
		int i;
		float rseed = 0.;
		for (i = 1; i < superSamplingRatio; i++) {
			vec2 seed = vec2(rseed++, rseed++);
			vec2 rand2 = vec2(rand(seed + 0.342), rand(seed + 0.756)) - 0.5;
			color += pallete(get_stability(z + rand2 * scale));
		}
		color /= superSamplingRatio;
	}
	fragCol = vec4(color, 1.0);
}
