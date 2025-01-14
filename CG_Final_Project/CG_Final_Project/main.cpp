
#include "global.h"
#include "InitApp.h"





#define WINDOW_POSITION 100		// 윈도우가 스크린의 어디에서 시작하는지 -> 스크린 좌상단 기준 x,x 에서 윈도우가 열림

#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "winmm.lib")
#include <Mmsystem.h>
#include <Digitalv.h>

GLvoid drawScene(GLvoid);
GLvoid Reshape(int w, int h);
GLuint ShaderProgram;
GLuint lightProgram;

SuperGLuint super;

Wall walls[WALL_COUNT];
Thorn thorns[TRAP_COUNT];
Hole holes[TRAP_COUNT];
Deceleration_Trap deceleration_traps[TRAP_COUNT];
WinItem win_items[WIN_COUNT];


GLfloat xAngle = 0.0f, yAngle = 0.0f;
GLfloat yaw = 0.0f, pitch = 0.0f;		// 오일러 각
GLfloat tempx = 0.0f, tempy = 0.0f;		// 이전의 마우스 값


glm::vec3 EYE = glm::vec3(0.0f, 1.0f, 0.5f);
glm::vec3 AT = glm::vec3(1.0f, 1.0f, 0.5f);
glm::vec3 UP = glm::vec3(0.0f, 1.0f, 0.0f);

glm::vec3 T_EYE = glm::vec3(0.0f, 20.0f, 0.01f);		// 탑뷰기준 카메라 EYE
glm::vec3 T_AT = glm::vec3(0.0f, 0.0f, 0.0f);		// 탑뷰기준 카메라 AT

bool Left = false, Right = false, Up = false, Down = false;		// 키 입력
bool move = false;
bool click = false;
bool JUMP = false;
Pacman *pacman = new Pacman;
Ghost ghosts[GHOST_COUNT];

//HSJ추가사항
int itemID[4];  //실제아이템아이디
int itemCOunt = 4;
//
EViewPoint view_point = E_DEFAULT_VIEW;

MCI_OPEN_PARMS m_mciOpenParms;
MCI_PLAY_PARMS m_mciPlayParms;
DWORD m_dwDeviceID;
MCI_OPEN_PARMS mciOpen;
MCI_PLAY_PARMS mciPlay;


void Mouse(int button, int state, int x, int y);
void MouseMotion(int x, int y);
void PassiveMouse(int x, int y);
void Timer(int a);
void myTimer(int a);
void JumpTimer(int a);
void InputKey(unsigned char key, int x, int y);
void KeyUP(unsigned char key, int x, int y);
bool check_move();								// 이동키가 눌렸나 확인하는 함수
void Set_Cursor();

int dwID;

int main(int argc, char** argv)
{
	srand(time(NULL));
	for (int i = 0; i < 4; ++i) {
		itemID[i] = rand() % 4 + i * 4;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(WINDOW_POSITION, WINDOW_POSITION);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Example1");
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Unable to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
		std::cout << "GLEW Initialized\n";
	glEnable(GL_CULL_FACE);

	InitProgram2(lightProgram);
	CreateCon(super.ConEBO, super.ConVBO);
	CreateCube(ShaderProgram, super.EBO, super.VBO);
	CreateSphere(super.SVBO, super.SNVBO);
	Set_Cursor();				// 커서 시작지점 설정
	init_wall(walls, thorns, holes, deceleration_traps, win_items, ghosts);				// 벽 좌표 설정
	
	mciOpen.lpstrElementName = TEXT("title.mp3");
	mciOpen.lpstrDeviceType = TEXT("mpegvideo");

	mciSendCommand(NULL, MCI_OPEN, MCI_OPEN_ELEMENT | MCI_OPEN_TYPE,
		(DWORD)(LPVOID)&mciOpen);
	dwID = mciOpen.wDeviceID;
	mciSendCommand(dwID, MCI_PLAY, MCI_DGV_PLAY_REPEAT, // play & repeat
		(DWORD)(LPVOID)&m_mciPlayParms);

	InitProgram(ShaderProgram);
	



	glutDisplayFunc(drawScene);
	glutKeyboardFunc(InputKey);			// 키보드 입력
	glutKeyboardUpFunc(KeyUP);			// 키보드 떼는 것
	glutMouseFunc(Mouse);				// 마우스 클릭
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(PassiveMouse);
	glutTimerFunc(10, myTimer, 2);
	glutTimerFunc(10, Timer, 1);
	glutTimerFunc(10, JumpTimer, 1);
	glutReshapeFunc(Reshape);
	glutMainLoop();
}

GLvoid drawScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);



	

	glUseProgram(ShaderProgram);


	view(ShaderProgram, view_point, EYE, AT, UP, T_EYE, T_AT);	// 카메라 

	Myprojection(ShaderProgram, view_point);

	draw_map(ShaderProgram, super, walls, thorns, holes, deceleration_traps, win_items, ghosts, pacman);

	// 플레이어 위치
	pacman->Draw(ShaderProgram, super.SVBO, super.SNVBO);

	for (int i = 0; i < GHOST_COUNT; ++i) {
		ghosts[i].Draw(ShaderProgram, super.SVBO, super.SNVBO);
	}


	renderBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, pacman->Get_lifecount(), itemCOunt);
	
	glUseProgram(lightProgram);
	draw_floor(lightProgram, super.VBO, super.EBO, pacman);

	glutSwapBuffers();


}
void myTimer(int a)
{
	if (!pacman->Get_is_lived()) {		// 죽었으면
		pacman->Revive(EYE, AT, yaw, pitch);
	}



	// ---------------------------- 함정 업데이트 부분 -----------------------------

	for (int i = 0; i < TRAP_COUNT; ++i) {
		thorns[i].Update(pacman->Get_Pos());
		holes[i].Update(pacman->Get_Pos());
	}


	// ---------------------------- 함정 업데이트 부분 -----------------------------



	// ------------------------- 함정 충돌체크 부분 -------------------------------------

	// 함정의 좌표를 받을 벡터
	glm::vec3 t_pos;		
	glm::vec3 h_pos;
	glm::vec3 d_pos;

	// 팩맨의 정보
	glm::vec3 pacman_pos = pacman->Get_Pos();
	GLfloat rsize = pacman->Get_Size();
	
	// 함정의 좌표, 사이즈를 담을 변수
	GLfloat x, z;
	GLfloat trap_size;




	// ---------------------------------- 가시 함정 ------------------------------------------------

	for (int i = 0; i < TRAP_COUNT; ++i) {
		t_pos = thorns[i].Get_Pos();
		trap_size = thorns[i].Get_Size() + 0.1f;

		x = t_pos.x;
		z = t_pos.z;

		if (is_Collision(pacman_pos.x, pacman_pos.z, rsize, x, z, trap_size, trap_size) && thorns[i].Get_State() && pacman->Get_is_lived()) {		// 충돌검사


			pacman->Die();		// Die 함수에서 lifeCount 의 감소, 부활이 자동으로 일어남
								// gameover 는 text 출력으로 임시 구현 되어있음
			std::cout << "걸림";

		}
	}


	// ---------------------------------- 가시 함정 ------------------------------------------------




	// ---------------------------------- 구멍 함정 ------------------------------------------------

	for (int i = 0; i < TRAP_COUNT; ++i) {
		h_pos = holes[i].Get_Pos();
		trap_size = thorns[i].Get_Size() - 0.1f;		

		x = h_pos.x;
		z = h_pos.z;

		if (is_Collision(pacman_pos.x, pacman_pos.z, rsize, x, z, trap_size, trap_size) && holes[i].Get_State()) {		// 충돌검사

			if (pacman->is_on_floor) {		// 팩맨에 구멍 함정에 걸렸는데 바닥에 있으면
				
				pacman->Fall_Hole();

			}
		}
	}



	// ---------------------------------- 구멍 함정 ------------------------------------------------



	// ---------------------------------- 감속 함정 ------------------------------------------------



	for (int i = 0; i < TRAP_COUNT; ++i) {
		d_pos = deceleration_traps[i].Get_Pos();
		trap_size = thorns[i].Get_Size() + 0.15f;

		x = d_pos.x;
		z = d_pos.z;

		// 초기 속도값이 30.0f
		// 충돌 && 초기속도값이면 속도를 낮춤
		if (is_Collision(pacman_pos.x, pacman_pos.z, rsize, x, z, trap_size, trap_size) && 			// 충돌검사
			(fabsf(pacman->Get_Speed() - 30.0f) <= FLT_EPSILON) && pacman->is_on_floor)
		{

			pacman->Set_Speed(100.0f);		// 속도 값으로 이동거리를 나누기 때문에 속도값이 크면느려짐
			pacman->slowdown_count = i;		// 몇번 함정에 걸린지를 알려줌

			break;
		}

		// 충돌 X && 느려진 상태면 속도 되돌림
		else if (!is_Collision(pacman_pos.x, pacman_pos.z, rsize, x, z, trap_size, trap_size) &&
			(fabsf(pacman->Get_Speed() - 100.0f) <= FLT_EPSILON) && (i == pacman->slowdown_count))		// 해당 함정에서 빠져나오면 속도 정상화
		{

			pacman->Set_Speed(30.0f);
			
		}

	}

	// ---------------------------------- 감속 함정 ------------------------------------------------



	// ------------------------- 함정 충돌체크 부분 -------------------------------------

	glutPostRedisplay();
	glutTimerFunc(100, myTimer, 2);
}

void Timer(int a)
{
	glm::vec3 pacman_pos;
	GLfloat rsize;

	if (move) {
		pacman->Move(Up, Down, Right, Left, EYE, AT, UP);

		pacman_pos = pacman->Get_Pos();
		rsize = pacman->Get_Size() + 0.1f;			// 팩맨의 반지름

		// -------------------------- 벽과 플레이어 충돌 -----------------------------------

		glm::vec3 w_pos;

		GLfloat x, z;									// 벽의 x, z값
		GLfloat width, depth;							// 벽의 가로 세로 길이

		GLfloat degree = 0.05f;		// 밀어낼때 판단 정도,   값이 너무 작으면 충돌처리 X, 값이 너무 크면 밀어내기 제대로 못함

		for (int i = 0; i < WALL_COUNT; ++i) {
			w_pos = walls[i].Get_Pos();

			x = w_pos.x;
			z = w_pos.z;

			width = walls[i].Get_Width();
			depth = walls[i].Get_Depth();



			if (is_Collision(pacman_pos.x, pacman_pos.z, rsize, x, z, width, depth)) {		// 사각형과 원의 충돌검사

				// 충돌 시 밀어내기 
				if (x + width - degree < pacman_pos.x - rsize) {		// 벽의 오른쪽에서 충돌

					pacman->Set_Pos_x(x + width + rsize);		// 벽의 오른쪽 + 반지름으로 x좌표 설정
				}

				else if (x - width + degree > pacman_pos.x + rsize) {		// 벽의 왼쪽에서 충돌

					pacman->Set_Pos_x(x - width - rsize);		// 벽의 왼쪽 - 반지름으로 x좌표 설정
				}

				if (z + depth - degree < pacman_pos.z - rsize) {		// 벽의 앞쪽에서 충돌

					pacman->Set_Pos_z(z + depth + rsize);		// 벽의 앞쪽 + 반지름으로 z좌표 설정
				}

				else if (z - depth + degree > pacman_pos.z + rsize) {		// 벽의 뒷쪽에서 충돌

					pacman->Set_Pos_z(z - depth - rsize);		// 벽의 뒷쪽 - 반지름으로 z좌표 설정
				}




			}

		}

		// -------------------------- 벽과 플레이어 충돌 -----------------------------------



		

		// -------------------------- 플레이어와 아이템 충돌 -----------------------------------

		glm::vec3 item_pos;

		GLfloat px, pz;		// 팩맨 위치
		GLfloat dx, dz;		// 팩맨 과 아이템 거리
		GLfloat d;		// 반지름의 합

		for (int i = 0; i < WIN_COUNT; ++i) {
			item_pos = win_items[i].Get_Pos();

			x = item_pos.x;		// 아이템 위치
			z = item_pos.z;

			px = pacman_pos.x;
			pz = pacman_pos.z;

			dx = x - px;
			dz = z - pz;
			
			d = win_items[i].Get_Size() + pacman->Get_Size();
			
			if ((dx * dx) + (dz * dz) <= (d * d) && win_items[i].is_activate) {	// 반지름 합 보다 원점끼리의 거리가 작으면 -> 충돌
				
				win_items[i].is_activate = false;
				itemCOunt -= 1;

				if (itemCOunt == 0) {
					std::cout << "게임 승리!" << std::endl;
				}
			}
		}



		// -------------------------- 플레이어와 아이템 충돌 -----------------------------------



	

	}

	// -------------------------- 플레이어와 유령 충돌 -----------------------------------


	glm::vec3 ghost_pos;
	GLfloat gx, gz, p_x, p_z, d_x, d_z, d2;

	for (int i = 0; i < GHOST_COUNT; ++i) {

		ghosts[i].Update();			// 유령 움직임

		ghost_pos = ghosts[i].Get_Pos();
		pacman_pos = pacman->Get_Pos();

		gx = ghost_pos.x;
		gz = ghost_pos.z;

		p_x = pacman_pos.x;
		p_z = pacman_pos.z;

		d_x = gx - p_x;
		d_z = gz - p_z;

		d2 = ghosts[i].Get_Size() + pacman->Get_Size() + 0.07f;

		if ((d_x * d_x) + (d_z * d_z) <= (d2 * d2) && pacman->Get_is_lived()) {	// 충돌 && 팩맨이 살아있다면

			pacman->Die();
		}
	}


	// -------------------------- 플레이어와 유령 충돌 -----------------------------------


	glutPostRedisplay();



	glutTimerFunc(10, Timer, a);
}

void JumpTimer(int a)
{
	/// ----------------------------- 업데이트 --------------------

	glm::vec3 pacman_pos = pacman->Get_Pos();

	pacman->Update(JUMP, EYE, AT, UP);

	if ((pacman_pos.y < 1.0f) && !pacman->is_fall && !pacman->is_on_floor) {		// y좌표, 팩맨이 함정에 빠지지않았는가, 바닥위에 있지않은가
		pacman->Set_Pos_y();
		pacman->Set_Jump();
		JUMP = false;				// 점프 후 바닥에 닿았다면 점프 false
	}



	/// ----------------------------- 업데이트 --------------------

	glutPostRedisplay();

	glutTimerFunc(15, JumpTimer, a);
}



GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);

}

void InputKey(unsigned char key, int x, int y)
{
	static bool culling = false;
	switch (key)
	{
	case '1':
		view_point = E_DEFAULT_VIEW;
		break;
	case '3':
		view_point = E_TOP_VIEW;
		break;

	case 'w':
		Up = true;

		if (!move) {				// 이동중이면 timer 함수를 호출 X
			move = true;

		}

		break;

	case 'a':
		Left = true;

		if (!move) {				// 이동중이면 timer 함수를 호출 X
			move = true;
		}

		break;

	case 's':
		Down = true;

		if (!move) {				// 이동중이면 timer 함수를 호출 X
			move = true;
		}

		break;

	case 'd':
		Right = true;

		if (!move) {				// 이동중이면 timer 함수를 호출 X
			move = true;
		}

		break;

	case 'k':

		break;


	case 'q':

		glutLeaveMainLoop();
		break;

	default:
		break;
	}

	glutPostRedisplay();
}

void KeyUP(unsigned char key, int x, int y)
{
	switch (key) {
	case 'w':
		Up = false;

		if (!check_move()) {		// 이동하지 않으면 move 함수를 false로 만듬
			move = false;
		}

		break;

	case 'a':
		Left = false;

		if (!check_move()) {
			move = false;
		}

		break;

	case 's':
		Down = false;

		if (!check_move()) {
			move = false;
		}

		break;

	case 'd':
		Right = false;

		if (!check_move()) {
			move = false;
		}

		break;

	case 32:			// 스페이스바
		if (!JUMP) {
			JUMP = true;

		}
		break;

	}

	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		click = true;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		click = false;
		Set_Cursor();
	}
	glutPostRedisplay();
}

void MouseMotion(int x, int y)
{
	if (click) {
		int width = WINDOW_WIDTH / 2;
		int height = WINDOW_HEIGHT / 2;

		GLfloat mx = (GLfloat)(x - width) / (GLfloat)width;			// 마우스 이동한 좌표
		GLfloat my = (GLfloat)(height - y) / (GLfloat)height;


		// 현재 프레임과 이전프레임의 마우스 좌표 차이
		xAngle = (my - tempy) * 100;				// 위아래 이동 시 x축 기준 회전		뒤의 숫자는 배율
		yAngle = (mx - tempx) * 100;				// 좌우 이동시 y축 기준 회전

		yaw += yAngle;
		pitch += xAngle;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;


		AT.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		AT.y = sin(glm::radians(pitch));
		AT.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		// AT 의 EYE 기준 회전
		AT = AT - EYE;		// EYE를 원점이동

		AT.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		AT.y = sin(glm::radians(pitch));
		AT.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

		AT = AT + EYE;		// 이동했던 만큼 다시 역이동

		tempx = mx;
		tempy = my;

		glutPostRedisplay();
	}
}

void PassiveMouse(int x, int y)
{
	Set_Cursor();
	glutPostRedisplay();
}

bool check_move()		// 이동키가 눌려있으면 true, 아니면 false
{
	if (Left || Right || Up || Down)
		return true;
	else
		return false;
}

void Set_Cursor()
{
	int x = WINDOW_WIDTH / 2 + WINDOW_POSITION + 11;
	int y = WINDOW_HEIGHT / 2 + WINDOW_POSITION + 31;

	SetCursorPos(x, y);		// 시작 마우스 커서 위치 설정

	tempx = 0.0f;
	tempy = 0.0f;
}

