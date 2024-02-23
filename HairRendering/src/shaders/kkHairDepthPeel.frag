#version 440

//---Opacity---//
uniform sampler2D opacityMap;
uniform sampler2D hairShadowMap;
uniform sampler2DShadow meshShadowMap;
uniform float shadowIntensity;
uniform bool useShadows;

float currentDepth;

const float LAYER_SIZE = 0.0005f;

//Sample opacity maps layers at the given texcoord to get occlusion from other strands
float SampleOcclusion(vec2 coord)
{
	float layerSize = LAYER_SIZE;
	float layerStart = texelFetch(hairShadowMap, ivec2(coord * textureSize(hairShadowMap, 0)), 0).r;
	vec4 opacity = texture(opacityMap, coord);
	float occlusion = 0;

	for (int i = 0; i < 4; i++)
	{
		float t = clamp((currentDepth - layerStart) / layerSize, 0.0f, 1.0f);
		occlusion += mix(0.0f, opacity[i], t);
		layerStart += layerSize;
		layerSize *= 2.0f;
	}
	return occlusion;
}

//Calculate transmittance of the light to a point based on opacity map
float CalculateTransmittance(vec4 point)
{
	vec4 shadowCoord = (point / point.w + 1.0f) / 2.0f;
	currentDepth = shadowCoord.z - 0.0001f;

	vec2 size = textureSize(hairShadowMap, 0);
	vec2 texelSize = vec2(1.0f) / size;

	//Interpolate 4 samples of opacity map
	vec2 fractional = fract(shadowCoord.xy * size);
	float sample1 = SampleOcclusion(shadowCoord.xy + texelSize * ivec2(0, 0));
	float sample2 = SampleOcclusion(shadowCoord.xy + texelSize * ivec2(0, 1));
	float sample3 = SampleOcclusion(shadowCoord.xy + texelSize * ivec2(1, 0));
	float sample4 = SampleOcclusion(shadowCoord.xy + texelSize * ivec2(1, 1));
	float occlusion = mix(mix(sample1, sample2, fractional.y), mix(sample3, sample4, fractional.y), fractional.x);

	return mix(1.0f, exp(-shadowIntensity * occlusion), useShadows);
}

float GetMeshVisibility(vec4 point)
{
	vec4 shadowCoord = (point / point.w + 1.0f) / 2.0f;
	shadowCoord.z -= 0.0003f;

	return mix(1.0f, texture(meshShadowMap, shadowCoord.xyz), useShadows);
}

//---Hair lighting---//
in float tessx_g;

uniform vec3 colour;
uniform float maxColourChange;
uniform float opacity;
uniform mat4 view;
uniform mat4 dirToLight;
uniform vec3 lightPosition;
uniform float lightIntensity;
uniform vec3 lightColour;
uniform vec3 camPosition;

uniform float ambientIntensity;
uniform float specularIntensity;
uniform float kajiyaSpecularExponent;
uniform float diffuseIntensity;

uniform bool useColourGradient;

const vec4 FILL_LIGHT_POSITION = vec4(-2.0f, 1.0f, 1.0f, 1.0f);

const float darkesttColor = 0.5f;
const float darkestGradient = 0.0f;
const float lightestGradient = 0.3f;

vec3 GetColour(vec4 pos, vec3 tangent, vec3 normal, vec4 lightPos, vec4 camPos, float colourChange)
{

	vec3 lightDir = normalize(lightPos.xyz - pos.xyz);
	vec3 viewDir = normalize(camPos.xyz - pos.xyz);
    vec3 halfVec = normalize(lightDir + viewDir);
	
    float NdotL = max(dot(normalize(tangent), lightDir), 0.0);
    float NdotH = max(dot(normalize(tangent), halfVec), 0.0);
    float specular = specularIntensity * pow(NdotH, kajiyaSpecularExponent) ;

	vec3 hairColour =  colour * lightColour; 

	if (useColourGradient) {
		vec3 colourScale = vec3(1.0f + maxColourChange * (2.0f * colourChange - 1.0f));
		colourScale *= mix(darkesttColor, 1.0f, smoothstep(darkestGradient, lightestGradient, tessx_g));

		hairColour =   colour * colourScale * lightColour; 
	}
    vec3 diffuse =  diffuseIntensity * hairColour * NdotL;
    vec3 ambient = ambientIntensity * hairColour ;
	
    vec3 finalColor = ambient + (diffuse + specular);
	return finalColor;
}

vec4 Lighting(vec4 pos, vec3 tangent, vec3 normal, float colourChange)
{
	
	vec4 lightSpacePos = dirToLight * pos;
	
	vec4 colour;
	colour.w = opacity;

	//Key light
	colour.xyz = GetColour(pos, tangent, normal, view * vec4(lightPosition, 1.0f), vec4(camPosition, 1.0f), colourChange);
	colour.xyz *= CalculateTransmittance(lightSpacePos);
	colour.xyz *= GetMeshVisibility(lightSpacePos);

	//Fill light
	colour.xyz += lightIntensity * GetColour(pos, tangent, normal, view * FILL_LIGHT_POSITION, vec4(camPosition, 1.0f), colourChange);

	return colour;
}

//---Depth peel---//
uniform sampler2D depthPeelMap;

const vec4 BACKGROUND_COLOUR = vec4 (0.0f, 0.0f, 0.0f, 1.0f);

void DepthPeel(inout vec4 colour, vec4 point)
{
	vec4 clip = (point / point.w + 1.0f) / 2.0f;
	
	float currentDepth = gl_FragCoord.z - 1e-4;
	float previousDepth = texelFetch(depthPeelMap, ivec2(clip.xy * textureSize(depthPeelMap, 0)), 0).r;

	float remove = step(currentDepth, previousDepth);

	colour = mix(colour, BACKGROUND_COLOUR, remove);
	gl_FragDepth = mix(gl_FragCoord.z, 1.0f, remove);
}

//---Main---//
in vec4 position_g;
in vec3 tangent_g;
in vec3 normal_g;
in float colourChange_g;

uniform mat4 projection;

out vec4 fragColour;

void main()
{
	fragColour = Lighting(position_g, tangent_g, normal_g, colourChange_g);
	DepthPeel(fragColour, projection * position_g);
}