#version 330

layout(location = 0) in vec3 inVertexPos_local;

uniform mat4 uModelMat;
uniform mat4 uViewProjMat;

void main ()
{
	vec4 pos = vec4(inVertexPos_local, 1.0);
	gl_Position = uViewProjMat * uModelMat * pos;
}