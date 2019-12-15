#pragma once
#include "global.h"

class Thorn
{
private:
	glm::vec3 Pos = glm::vec3(1.0f);
	GLfloat height = 0.0f;
	GLfloat MAX_HEIGHT = 0.6f;
	GLfloat size = 1.0f / 6.0f;
	
	bool is_detect_player = false;		// 감지했나 확인하는 변수
	bool is_activated = false;			// 발동했나 확인하는 변수, -> 발동했다면 쿨타임이 있음
	GLfloat cool_time = 5.0f;
	GLfloat time = 0.0f;
	GLfloat detect_range = 1.0f;

public:
	Thorn();
	~Thorn();

	void Set_Pos(GLfloat x, GLfloat z);
	void Draw(GLuint ShaderProgram, GLuint ConVBO, GLuint ConEBO);
	void Update(glm::vec3 Pos);
	void Detect_to_Player(glm::vec3 Pos);		// 플레이어를 감지함
};

