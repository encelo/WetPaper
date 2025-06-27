namespace {

char const * const sprite_blur_fs = R"(
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D uTexture;
uniform vec2 uResolution;
uniform vec2 uDirection;
out vec4 fragColor;

void main()
{
	vec2 uv = vec2(gl_FragCoord.xy / uResolution.xy);
	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.3846153846) * uDirection;
	vec2 off2 = vec2(3.2307692308) * uDirection;

	color += texture(uTexture, uv) * 0.2270270270;
	color += texture(uTexture, uv + (off1 / uResolution)) * 0.3162162162;
	color += texture(uTexture, uv - (off1 / uResolution)) * 0.3162162162;
	color += texture(uTexture, uv + (off2 / uResolution)) * 0.0702702703;
	color += texture(uTexture, uv - (off2 / uResolution)) * 0.0702702703;

	fragColor = color;
}
)";

char const * const sprite_dispersion_vs = R"(
uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;

layout (std140) uniform InstanceBlock
{
	mat4 modelMatrix;
	vec4 color;
	vec4 texRect;
	vec2 spriteSize;
};

out vec2 vPosition;
out vec2 vTexCoords;

void main()
{
	vec2 aPosition = vec2(0.5 - float(gl_VertexID >> 1), -0.5 + float(gl_VertexID % 2));
	vec2 aTexCoords = vec2(1.0 - float(gl_VertexID >> 1), 1.0 - float(gl_VertexID % 2));
	vec4 position = vec4(aPosition.x * spriteSize.x, aPosition.y * spriteSize.y, 0.0, 1.0);

	gl_Position = uProjectionMatrix * uViewMatrix * modelMatrix * position;
	vPosition = aPosition;
	vTexCoords = vec2(aTexCoords.x * texRect.x + texRect.y, aTexCoords.y * texRect.z + texRect.w);
}
)";

char const * const batched_sprite_dispersion_vs = R"(
uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;

struct Instance
{
	mat4 modelMatrix;
	vec4 color;
	vec4 texRect;
	vec2 spriteSize;
};

// Single instances data will be collected in a uniform block called `InstancesBlock`
layout (std140) uniform InstancesBlock
{
#ifndef BATCH_SIZE
	#define BATCH_SIZE (585) // 64 Kb / 112 b
#endif
	Instance[BATCH_SIZE] instances;
} block;

out vec2 vPosition;
out vec2 vTexCoords;

#define i block.instances[gl_VertexID / 6]

void main()
{
	vec2 aPosition = vec2(0.5 - float(gl_VertexID >> 1), -0.5 + float(gl_VertexID % 2));
	vec2 aTexCoords = vec2(1.0 - float(gl_VertexID >> 1), 1.0 - float(gl_VertexID % 2));
	vec4 position = vec4(aPosition.x * i.spriteSize.x, aPosition.y * i.spriteSize.y, 0.0, 1.0);

	gl_Position = uProjectionMatrix * uViewMatrix * i.modelMatrix * position;
	vPosition = aPosition;
	vTexCoords = vec2(aTexCoords.x * i.texRect.x + i.texRect.y, aTexCoords.y * i.texRect.z + i.texRect.w);
}
)";

// Taken from: https://blog.maximeheckel.com/posts/refraction-dispersion-and-other-shader-light-effects/
char const * const sprite_dispersion_fs = R"(
#ifdef GL_ES
precision mediump float;
#endif

const float uIorR = 1.15;
const float uIorY = 1.16;
const float uIorG = 1.18;
const float uIorC = 1.22;
const float uIorB = 1.22;
const float uIorP = 1.22;

const float uSaturation = 1.08;
const float uChromaticAberration = 0.60;
const float uRefractPower = 0.40;
const float uFresnelPower = 8.0;
const float uShininess = 40.0;
const float uDiffuseness = 0.2;
const vec3 uLight = vec3(-1.0, 1.0, 1.0);

uniform vec2 winResolution;
uniform sampler2D uTexture0;
uniform sampler2D uTexture1;
in vec2 vPosition;
in vec2 vTexCoords;
out vec4 fragColor;

vec3 sat(vec3 rgb, float adjustment)
{
	const vec3 W = vec3(0.2125, 0.7154, 0.0721);
	vec3 intensity = vec3(dot(rgb, W));
	return mix(intensity, rgb, adjustment);
}

float fresnel(vec3 eyeVector, vec3 normal, float power)
{
	float fresnelFactor = abs(dot(eyeVector, normal));
	float inversefresnelFactor = 1.0 - fresnelFactor;

	return pow(inversefresnelFactor, power);
}

float specular(vec3 light, vec3 eyeVector, vec3 normal, float shininess, float diffuseness)
{
	vec3 lightVector = normalize(-light);
	vec3 halfVector = normalize(eyeVector + lightVector);

	float NdotL = dot(normal, lightVector);
	float NdotH =  dot(normal, halfVector);
	float kDiffuse = max(0.0, NdotL);
	float NdotH2 = NdotH * NdotH;

	float kSpecular = pow(NdotH2, shininess);
	return  kSpecular + kDiffuse * diffuseness;
}

const int LOOP = 16;

void main()
{
	float iorRatioRed = 1.0 / uIorR;
	float iorRatioGreen = 1.0 / uIorG;
	float iorRatioBlue = 1.0 / uIorB;

	vec2 uv = gl_FragCoord.xy / winResolution.xy;
	uv.y = 1.0 - uv.y;

	vec3 position3 = vec3(vPosition.x, vPosition.y, 0.0);
	vec2 centeredPos = vPosition * 2.0;
	float distSquared = dot(centeredPos, centeredPos);
	if (distSquared > 1.0)
		discard;
	vec3 normal = vec3(centeredPos.x, centeredPos.y, sqrt(1.0 - distSquared));

	vec2 centeredTC = vTexCoords * 2.0 - 1.0;
	vec3 eyeVector = normalize(vec3(centeredTC.x, centeredTC.y, -1.0));

	vec3 color = vec3(0.0);
	for ( int i = 0; i < LOOP; i ++ )
	{
		float slide = float(i) / float(LOOP) * 0.1;

		vec3 refractVecR = refract(eyeVector, normal, (1.0 / uIorR));
		vec3 refractVecY = refract(eyeVector, normal, (1.0 / uIorY));
		vec3 refractVecG = refract(eyeVector, normal, (1.0 / uIorG));
		vec3 refractVecC = refract(eyeVector, normal, (1.0 / uIorC));
		vec3 refractVecB = refract(eyeVector, normal, (1.0 / uIorB));
		vec3 refractVecP = refract(eyeVector, normal, (1.0 / uIorP));

		float r = texture(uTexture0, uv + refractVecR.xy * (uRefractPower + slide * 1.0) * uChromaticAberration).x * 0.5;

		float y = (texture(uTexture0, uv + refractVecY.xy * (uRefractPower + slide * 1.0) * uChromaticAberration).x * 2.0 +
		           texture(uTexture0, uv + refractVecY.xy * (uRefractPower + slide * 1.0) * uChromaticAberration).y * 2.0 -
		           texture(uTexture0, uv + refractVecY.xy * (uRefractPower + slide * 1.0) * uChromaticAberration).z) / 6.0;

		float g = texture(uTexture0, uv + refractVecG.xy * (uRefractPower + slide * 2.0) * uChromaticAberration).y * 0.5;

		float c = (texture(uTexture0, uv + refractVecC.xy * (uRefractPower + slide * 2.5) * uChromaticAberration).y * 2.0 +
		           texture(uTexture0, uv + refractVecC.xy * (uRefractPower + slide * 2.5) * uChromaticAberration).z * 2.0 -
		           texture(uTexture0, uv + refractVecC.xy * (uRefractPower + slide * 2.5) * uChromaticAberration).x) / 6.0;

		float b = texture(uTexture0, uv + refractVecB.xy * (uRefractPower + slide * 3.0) * uChromaticAberration).z * 0.5;

		float p = (texture(uTexture0, uv + refractVecP.xy * (uRefractPower + slide * 1.0) * uChromaticAberration).z * 2.0 +
		           texture(uTexture0, uv + refractVecP.xy * (uRefractPower + slide * 1.0) * uChromaticAberration).x * 2.0 -
		           texture(uTexture0, uv + refractVecP.xy * (uRefractPower + slide * 1.0) * uChromaticAberration).y) / 6.0;

		float R = r + (2.0 * p + 2.0 * y - c) / 3.0;
		float G = g + (2.0 * y + 2.0 * c - p) / 3.0;
		float B = b + (2.0 * c + 2.0 * p - y) / 3.0;

		color.r += R;
		color.g += G;
		color.b += B;

		color = sat(color, uSaturation);
	}

	// Divide by the number of layers to normalize colors (rgb values can be worth up to the value of LOOP)
	color /= float(LOOP);

	// Specular
	float specularLight = specular(uLight, eyeVector, normal, uShininess, uDiffuseness);
	color += specularLight;

	// Fresnel
	float f = fresnel(eyeVector, normal, uFresnelPower);
	color.rgb += f * vec3(1.0);

	vec4 bubbleTex = texture(uTexture1, vTexCoords);
	vec3 blendedRGB = bubbleTex.rgb * bubbleTex.a + color.rgb * (1.0 - bubbleTex.a);
	float blendedA = (bubbleTex.a < 0.1) ? bubbleTex.a : 1.0;

	fragColor = vec4(blendedRGB, blendedA);
}
)";

}
