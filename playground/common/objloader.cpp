#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>

#include <glm/glm.hpp>

#include "objloader.hpp"

bool loadOBJ(
	const char *path,
	std::vector<glm::vec3> &out_vertices,
	std::vector<glm::vec2> &out_uvs,
	std::vector<glm::vec3> &out_normals)
{
	printf("Loading OBJ file...\n");

	std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	std::vector<glm::vec3> temp_vertices;
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;

	FILE* file = fopen(path, "r");
	if (file == NULL)
	{
		printf("Cannot open file %s!\n", path);
		getchar();
		return false;
	}

	// 这里没有放在循环里
	char lineHead[128];
	while (true)
	{
		//lineHead = (char*)alloca(sizeof(char) * 128);
		int data = fscanf(file, "%s", lineHead);
		if (data == EOF) {
			break;
		}
		else {
			if (strcmp(lineHead, "v") == 0) {
				glm::vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}
			else if (strcmp(lineHead, "vt") == 0) {
				glm::vec2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back(uv);
			}
			else if (strcmp(lineHead, "vn") == 0) {
				glm::vec3 normal;
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				temp_normals.push_back(normal);
			}
			else if (strcmp(lineHead, "f") == 0) {
				//  这是一个Fragment
				std::string v1, v2, v3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n",
					&vertexIndex[0], &uvIndex[0], &normalIndex[0],
					&vertexIndex[1], &uvIndex[1], &normalIndex[1],
					&vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9) {
					printf("OBJ file bad parsing, fragment line should have 9 elements\n");
					return false;
				}
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
		}
	}

	// 使用索引反向找到真实数据放到输出buffer中
	for (unsigned int i = 0; i < normalIndices.size; i++)
	{
		// 减一是因为OBJ文件下标从1开始
		out_normals.push_back(temp_normals[normalIndices[i] - 1]);
		out_uvs.push_back(temp_uvs[uvIndices[i] - 1]);
		out_vertices.push_back(temp_vertices[vertexIndices[i] - 1]);
	}

	fclose(file);
	printf("OBJ File loaded!\n");
	return true;
}
