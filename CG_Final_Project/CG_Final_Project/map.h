#pragma once
#ifndef __MAP
#define __MAP

#include "Wall.h"
#include "Thorn.h"
#include "Hole.h"
#include "Deceleration_Trap.h"
#include "WinItem.h"

#define WALL_COUNT 42
#define TRAP_COUNT 3
#define ITEM_COUNT 16
#define WIN_COUNT 4

struct TrapPoint {
	float x, z;
};
void draw_floor(GLuint ShaderProgram, GLuint VBO, GLuint EBO);

void init_wall(Wall walls[], Thorn thorns[], Hole holes[], Deceleration_Trap traps[], WinItem items[]);
void draw_map(GLuint ShaderProgram,SuperGLuint super, Wall walls[], Thorn thorns[], Hole holes[], Deceleration_Trap traps[], WinItem items[]);
void draw_WinItem(GLuint ShaderProgram, GLuint SVBO, GLuint SNVBO, TrapPoint p);
#endif