#include "HairOpacityShaderProgram.h"

HairOpacityShaderProgram::HairOpacityShaderProgram(const char* vertex, const char* fragment, const char* geometry, const char* tessControl, const char* tessEval) : ShaderProgram(vertex, fragment, geometry, tessControl, tessEval)
{
	uniforms.numGroupHairs = 1;
	uniforms.groupSpread = 0.15f;
	uniforms.hairRadius = 0.001f;
	uniforms.splineTension = 0.5f;
	uniforms.tapering = 5.0f;
	uniforms.noiseAmplitude = 0.0f;
	uniforms.colour = glm::vec3(0.6f, 0.4f, 0.3f);
}

void HairOpacityShaderProgram::SetGlobalUniforms()
{
	glUniformMatrix4fv(GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(uniforms.view));
	glUniformMatrix4fv(GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(uniforms.projection));
	glUniformMatrix4fv(GetUniformLocation("dirToLight"), 1, GL_FALSE, glm::value_ptr(uniforms.dirToLight));
	glUniform3fv(GetUniformLocation("lightPosition"), 1, glm::value_ptr(uniforms.lightPosition));
	glUniform3fv(GetUniformLocation("camPosition"), 1, glm::value_ptr(uniforms.camPosition));
	glUniform1i(GetUniformLocation("noiseTexture"), uniforms.noiseTexture);
	glUniform1i(GetUniformLocation("shadowMap"), uniforms.hairShadowMap);
}
