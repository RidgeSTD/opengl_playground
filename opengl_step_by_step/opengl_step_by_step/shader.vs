#version 330  //���߱��������ǵ�Ŀ��GLSL�������汾��3.3

layout (location = 0) in vec3 Position; // �󶨶��������������ԣ���ʽ���������Ժ�shader���Զ�Ӧӳ��

uniform mat4 gWorld;

void main()
{
	gl_Position = gWorld * vec4(Position, 1.0);
}