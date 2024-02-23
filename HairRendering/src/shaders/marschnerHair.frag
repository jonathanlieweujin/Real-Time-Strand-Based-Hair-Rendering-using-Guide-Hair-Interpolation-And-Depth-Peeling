#version 440

//Opacity
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
uniform mat4 view;
uniform mat4 dirToLight;
uniform vec3 lightPosition;
uniform vec3 camPosition;
uniform vec3 lightColour;

uniform float ambientIntensity;
uniform float specularIntensity;
uniform float diffuseIntensity;

uniform float opacity;
uniform float maxColourChange;
uniform bool useColourGradient;

uniform float alphaR_;
uniform float alphaTRT;
uniform float alphaTT;

uniform float expR_;
uniform float expTRT;
uniform float expTT;

uniform float lightIntensity;
const vec4 FILL_LIGHT_POSITION = vec4(-2.0f, 1.0f, 1.0f, 1.0f);
const float darkesttColor = 0.5f;
const float darkestGradient = 0.0f;
const float lightestGradient = 0.3f;

vec4 shiftedTangent(vec3 T, vec3 N, float shift){
    vec3 shiftedT = T + shift * N;
    vec3 normalizedShiftedT = normalize(shiftedT);
    return vec4(normalizedShiftedT, 0.0);
}

float rand( vec2 p )
{
    return fract(sin(dot(p,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 GetColour(vec4 pos, vec3 tangent, vec3 normal, vec4 lightPos, vec4 camPos, float colourChange)
{
	vec4 shiftedNormal = shiftedTangent(tangent,normal,rand(pos.xy));

	vec4 lightDir = normalize(lightPos - pos);
	vec4 viewDir = normalize(camPos - pos);

	vec3 N = shiftedNormal.xyz;  
    vec3 V = normalize(viewDir.xyz);
    vec3 L = normalize(lightDir.xyz);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
	
	vec3 hairColour =  colour * lightColour; 

	if (useColourGradient) {
		vec3 colourScale = vec3(1.0f + maxColourChange * (2.0f * colourChange - 1.0f));
		colourScale *= mix(darkesttColor, 1.0f, smoothstep(darkestGradient, lightestGradient, tessx_g));

		hairColour =   colour * colourScale * lightColour; 
	}

    vec3 diffuse = hairColour * NdotL ;

	float theta = asin(NdotL) + asin(NdotV);
	
	vec3 term1 = normalize(L-NdotL*N);
	vec3 term2 = normalize(V-N*NdotV);

	float cosPhi = dot(term1,term2);

	vec3 R_ = vec3(clamp(pow( abs(cos(theta - alphaR_)) , expR_) , 0.0, 1.0))  ;
	vec3 TRT = vec3(clamp(pow( abs(cos(theta - alphaTRT)), expTRT),0.0, 1.0)) ;
	vec3 TT = vec3(clamp(max(0.0, cosPhi) * pow( cos(theta - alphaTT) , expTT), 0.0, 1.0)) ;

	vec3 specular =  R_ + TRT + TT;

	vec3 ambient = hairColour;

	vec3 color = ambient * ambientIntensity + diffuse * diffuseIntensity + specular * specularIntensity;
	return clamp(color,0.0,1.0);

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

// Main
in vec4 position_g;
in vec3 tangent_g;
in vec3 normal_g;
in float colourChange_g;
uniform bool discardLowAlpha;
//uniform mat4 projection;
//uniform sampler2D depthBufferMap;

//in float isControlPoint;

out vec4 fragColour;

void main()
{
	vec4 colour = Lighting(position_g, tangent_g, normal_g, colourChange_g);
	fragColour = colour;
}
