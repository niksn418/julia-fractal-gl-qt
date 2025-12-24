#version 330 core

in vec2 fragPos;
out vec4 fragCol;

uniform vec2 c;
uniform float radiusThresholdSquared;
uniform int maxIterations;

uniform vec2 posScale;
uniform vec2 center;

uniform vec3 aColor;
uniform vec3 bColor;
uniform vec3 cColor;
uniform vec3 dColor;

vec3 pallete(float t) {
    return aColor + bColor * cos(6.28318 * (cColor * t + dColor));
}

void main() {
	vec2 z = center + fragPos * posScale;
	int iterations;
	for (iterations = 0; iterations < maxIterations; iterations++) {
		if (dot(z, z) > radiusThresholdSquared) break;
		z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;
	}

	float stability = float(iterations) / float(maxIterations);
	fragCol = vec4(pallete(stability), 1.0);
}
