
/*
  Tweak DefaultFlagValue to optimal speed. loop speed may vary due to several environmental reasons.

  여러 요인에 따라 실행 시 반복이 돌아가는 속도가 다를 수 있음
  따라서 동작이 너무 빠르거나 느리다면 ClassicGame() 과 DualGame() 의 FlagValue 기본값을 조절해야 함.
*/
#define DefaultFlagValue 200

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <string.h>
#pragma execution_character_set( "utf-8" ) //for better UI

//틀 위치
#define C 21
#define L13x 1
#define LR12y 4
#define L24x 23
#define R13x 75
#define R24x 97

//키
#define ARROW 224
#define UP 72
#define LEFT 75
#define DOWN 80
#define RIGHT 77

//구현을 위한 외부 함수
void SetCur(int X, int Y);
void SetColor(int text, int bg);
void HideCur();
COORD GetCur(void);
void ShowCur();
//사용자 정의 함수
void UpdateTime(double* ElapsedTime);
void ClassicGame(int Mode); //1:40Lines
void DualGame(int Mode)
void Template(int dual); //틀 출력, 1입력시 3D용, 1 입력 시 2인용
void Title(); //타이틀 화면
void PrintTitle();
void DebugMode();
int ClearRows(int fieldnum); //꽉 찬 줄 개수 리턴, 자체적으로 제거
int CanPlaceTetr(int fieldnum, int* pos);
void BasicSpin(int fieldnum, int defaulttype, int dir);
void SpinTetr(int fieldnum, int* Pos, int defaulttype, int dir);
void CopyField(int fieldnum);
void PutTetr(int fieldnum, int* pos, int del); //테트로미노 배치
int PrintField(int field, int raw);
void GenerateTetr(int fieldnum, int* TetrArray, int* Cnt, int type); //테트로미노 정보를 생성,저장, type에 0 입력 시 랜덤
void PrintSingleTetr(int type, int loc); //hold, next 에 직접적으로 출력

struct Tetromino
{
	int type; //종류
	int color; //색
	int Leftend;
	int Bottomend;
	int Rightend;
};
//Global Variables

clock_t LastTime;
int randomvalue = 1; //이전에 생성한 랜덤 값과 중복 방지 - 완전 랜덤 구현
struct Tetromino TetrData[2];
int field[2][24][10]; //[필드][y][x] 초기값: 0, 컬러 값을 넣어 현재 상태를 출력
int temp[2][24][10]; //field 의 임시 버전

void main()
{
	SetConsoleOutputCP(65001); 
	HideCur();

	Title();



	SetCur(90, 70);
}

void ClassicGame(int Mode)
{
	int Flag = 1; //블록이 내려오는 플래그
	double ElapsedTime = 0;
	double* ElapsedTimePtr = &ElapsedTime;
	int FlagValue = DefaultFlagValue;
	int TimePause = 1;
	int Input;
	int Pos[4] = { 4, 3, 4, 3 }; //칸 왼쪽 아래를 0, 0 이라 할때 출력하는 칸, == 중 왼쪽을 의미 (max : (10, 20)), 뒤 2개는 prev
	int* PosPtr = Pos;
	int WallPushStack = 0; //벽에서 계속 밀면 안 떨어지는 현상 방지
	int new = 1; //새로운 조각 출력 여부, 이전 조각이 바닥에 닿았거나 시작시 1
	int holdable = 1; //hold 번복 안 되게 함
	int holdtetr = 0; //hold 중인 테트로미노
	int nexttetr; //next
	int GameOver = 0; //반복 탈출 조건
	int ClearedLines = 0;
	int CurrentClearedLines = 0;
	int TetrCnt[7]; //테트로미노가 나왔는지 저장 - 7-bag rule 적용
	int* TetrCntPtr = TetrCnt;
	int TetrominoArray[2]; //현재-next
	int* TetrArrayPtr = TetrominoArray;
	int defaulttype;
	int swaptemp; //교체에 사용
	int update = 0;
	double besttime;
	int bestline;
	char temp[10];
	int j;
	int i;
	for (i = 0; i < 10; i++)
		temp[i] = NULL;
	FILE* file = fopen("records.txt", "r");
	if (file == NULL)
		fprintf(file, "%d\n%lf", 0, 0.0);
	fscanf(file, "%s", temp);
	bestline = atoi(temp);
	for (i = 0; i < 10; i++)
		temp[i] = NULL;
	fscanf(file, "%s", temp);
	besttime = atof(temp);
	LastTime = clock();


	system("cls");
	Template(0);

	//변수 초기화
	for (i = 0; i < 7; i++)
		TetrCnt[i] = 0;
	swaptemp = rand() % 7;
	TetrominoArray[1] = swaptemp * 4 + 1;
	for (i = 0; i < 24; i++)
	{
		for (j = 0; j < 10; j++)
			field[0][i][j] = 0;
	}

	while (!GameOver)
	{
		if (new) //새로운 조각 출력
		{
			if (holdable == 2)
			{
				holdable = 0;
				GenerateTetr(0, TetrArrayPtr, TetrCntPtr, holdtetr);
			}

			else
			{
				holdable = 1;
				GenerateTetr(0, TetrArrayPtr, TetrCntPtr, 0);
			}
			CurrentClearedLines = ClearRows(0); //꽉 찬 줄 개수 리턴, 자체적으로 제거
			if (CurrentClearedLines)
			{
				ClearedLines += CurrentClearedLines;
				if (!Mode)
				{
					FlagValue -= 2 * CurrentClearedLines;
				}
				CurrentClearedLines = 0;
			}
			Pos[0] = 4;
			Pos[1] = 3;
			Pos[2] = 4;
			Pos[3] = 3;

			nexttetr = TetrominoArray[1];
			defaulttype = TetrData[0].type;
			new = 0;

			if (Mode && !CanPlaceTetr(0, PosPtr)) //종료 조건
			{
				GameOver = 1;
			}
			else if ((Mode && ClearedLines >= 40))
			{
				GameOver = 1;
			}
			else if (!Mode && !CanPlaceTetr(0, PosPtr))
			{
				GameOver = 1;
			}
			PrintSingleTetr(nexttetr, 0);
		}
		if (!(Flag % FlagValue))
		{
			if (Pos[1] > 3)
				TimePause = 0;
			Pos[1]++;
			PutTetr(0, PosPtr, !new);
			if (CanPlaceTetr(0, PosPtr))
			{
				PutTetr(0, PosPtr, 0);

				Pos[3] = Pos[1]; //y 업데이트
				WallPushStack = 0;
			}
			else
			{
				new = 1;
				Pos[1] = Pos[3];
				PutTetr(0, PosPtr, 0);
			}

		}
		if (_kbhit())
		{
			Input = _getch(); //키 입력 확인
			switch (Input)
			{
			case ARROW: //arrow
				switch (_getch())
				{
				case UP:
					SpinTetr(0, PosPtr, defaulttype, 1);
					PutTetr(0, PosPtr, 0);
					WallPushStack = 0;
					break;
				case DOWN:
					Pos[1]++;
					WallPushStack = 0;
					break;
				case LEFT:
					WallPushStack++;
					if (WallPushStack >= 5)
					{
						Pos[1]++;
						WallPushStack = 0;
					}
					else
						Pos[0]--;
					break;
				case RIGHT:
					WallPushStack++;
					if (WallPushStack >= 5)
					{
						Pos[1]++;
						WallPushStack = 0;
					}
					else
						Pos[0]++;
					break;
				}
				PutTetr(0, PosPtr, 1);
				if (CanPlaceTetr(0, PosPtr))
				{
					PutTetr(0, PosPtr, 0);
					Pos[3] = Pos[1];
					Pos[2] = Pos[0];
				}
				else
				{
					Pos[1] = Pos[3];
					Pos[0] = Pos[2];
					PutTetr(0, PosPtr, 0);
				}
				break;
			case 32: //space
				PutTetr(0, PosPtr, 1);
				for (i = 0; CanPlaceTetr(0, PosPtr); i++)
				{
					Pos[1]++;
					Pos[3] = Pos[1];
				}
				Pos[1]--;
				Pos[3]--;
				new = 1;
				PutTetr(0, PosPtr, 0);
				WallPushStack = 0;
				break;
			case 'z': //left spin
			case 'Z':
				SpinTetr(0, PosPtr, defaulttype, 0);
				PutTetr(0, PosPtr, 0);
				WallPushStack = 0;
				break;
			case 'x': //right spin
			case 'X':
				SpinTetr(0, PosPtr, defaulttype, 1);
				PutTetr(0, PosPtr, 0);
				WallPushStack = 0;
				break;
			case 'c': //HOLD
			case 'C':
				PutTetr(0, PosPtr, 1);

				if (holdable)
				{
					if (!holdtetr) //처음 hold
					{
						holdtetr = TetrominoArray[0];
						TetrominoArray[0] = TetrominoArray[1];
						while (1)
						{
							swaptemp = (rand() % 7);
							if (!TetrCnt[swaptemp])
								break;
						}
						TetrCnt[swaptemp] = 1;
						TetrominoArray[1] = swaptemp * 4 + 1;

					}
					else
					{
						TetrominoArray[0] = holdtetr;
					}
					holdtetr = defaulttype;
					PrintSingleTetr(holdtetr, 1);
					new = 1;
					holdable = 2;
					continue;
				}
				break;
			case '0':
				/*for (i = 0; i < 24; i++)
				{
					for (j = 0; j < 10; j++)
					{
						field[0][i][j] = 0;
					}
				}
				DebugMode();*/
				break;
			default:
				break;
			}
		}
		Sleep(1);
		Flag++;
		if (!TimePause)
			UpdateTime(ElapsedTimePtr);

		SetCur(24, 21);
		printf("Time : %.2lf", ElapsedTime);
		SetCur(24, 23);
		printf("Lines : %d", ClearedLines);
		SetCur(24, 24);
		if (Mode)
			printf("Best : %.2lf", besttime);
		else
			printf("Best : %d", bestline);
	}
	system("cls");
	Sleep(10);
	SetCur(55, 11);

	file = fopen("records.txt", "w");
	if (Mode)
	{
		printf("Time : %.2lf", ElapsedTime);
		if (ClearedLines >= 40 && (besttime > ElapsedTime || besttime == 0.0))
		{
			SetCur(55, 12);
			printf("NEW RECORD!");
			besttime = ElapsedTime;

		}
	}
	else
	{
		printf("Lines : %d", ClearedLines);
		if ((int)bestline < ClearedLines)
		{
			SetCur(55, 12);
			printf("NEW RECORD!");
			bestline = ClearedLines;
		}
		Sleep(1000);
	}
	/*for (i = 0; i < 24; i++)
	{
		for (j = 0; j < 10; j++)
			temp[0][i][j] = 0;
	}*/
	SetCur(47, 14);
	printf("press any key to continue");
	while (!_kbhit());
	fprintf(file, "%d\n%f", bestline, besttime);
	Input = _getch();
	fclose(file);
}
void DualGame(int Mode)
{
	//Flag, ElapsedTime, Input, swaptemp제외 모두 2 붙인 변수로 존재
	int Flag = 1; //블록이 내려오는 플래그
	double ElapsedTime = 0;
	double* ElapsedTimePtr = &ElapsedTime;
	int FlagValue = DefaultFlagValue;
	int FlagValue2 = DefaultFlagValue;
	int TimePause = 1;
	int Input;
	int Pos[4] = { 4, 3, 4, 3 }; //칸 왼쪽 아래를 0, 0 이라 할때 출력하는 칸, == 중 왼쪽을 의미 (max : (10, 20)), 뒤 2개는 prev
	int Pos2[4] = { 4, 3, 4, 3 };
	int* PosPtr = Pos;
	int* Pos2Ptr = Pos2;
	int WallPushStack = 0; //벽에서 계속 밀면 안 떨어지는 현상 방지
	int WallPushStack2 = 0;
	int new = 1; //새로운 조각 출력 여부, 이전 조각이 바닥에 닿았거나 시작시 1
	int new2 = 1;
	int holdable = 1; //hold 번복 안 되게 함
	int holdable2 = 1;
	int holdtetr = 0; //hold 중인 테트로미노
	int holdtetr2 = 0;
	int nexttetr; //next
	int nexttetr2;
	int GameOver = 0; //반복 탈출 조건
	int GameOver2 = 0;
	int ClearedLines = 0;
	int ClearedLines2 = 0;
	int CurrentClearedLines = 0;
	int CurrentClearedLines2 = 0;
	int TetrCnt[7]; //테트로미노가 나왔는지 저장 - 7-bag rule 적용
	int TetrCnt2[7];
	int* TetrCntPtr = TetrCnt;
	int* TetrCnt2Ptr = TetrCnt2;
	int TetrominoArray[2]; //현재-next
	int TetrominoArray2[2];
	int* TetrArrayPtr = TetrominoArray;
	int* TetrArray2Ptr = TetrominoArray2;
	int defaulttype;
	int defaulttype2;
	int swaptemp; //교체에 사용
	int winner = 0; //기본값

	int j;
	int i;
	system("cls");
	Template(1);

	//변수 초기화
	for (i = 0; i < 7; i++)
	{
		TetrCnt2[i] = 0;
		TetrCnt[i] = 0;
	}
	swaptemp = rand() % 7;
	TetrominoArray[1] = swaptemp * 4 + 1;
	for (i = 0; i < 24; i++)
	{
		for (j = 0; j < 10; j++)
			field[0][i][j] = 0;
	}
	swaptemp = rand() % 7;
	TetrominoArray2[1] = swaptemp * 4 + 1;
	for (i = 0; i < 24; i++)
	{
		for (j = 0; j < 10; j++)
			field[1][i][j] = 0;
	}

	while (!GameOver && !GameOver2)
	{
		if (new) //새로운 조각 출력
		{
			if (holdable == 2)
			{
				holdable = 0;
				GenerateTetr(0, TetrArrayPtr, TetrCntPtr, holdtetr);
			}

			else
			{
				holdable = 1;
				GenerateTetr(0, TetrArrayPtr, TetrCntPtr, 0);
			}
			CurrentClearedLines = ClearRows(0); //꽉 찬 줄 개수 리턴, 자체적으로 제거
			if (CurrentClearedLines)
			{
				ClearedLines += CurrentClearedLines;
				if (!Mode)
				{
					FlagValue -= 2 * CurrentClearedLines;
				}
				CurrentClearedLines = 0;
			}
			Pos[0] = 4;
			Pos[1] = 3;
			Pos[2] = 4;
			Pos[3] = 3;

			nexttetr = TetrominoArray[1];
			defaulttype = TetrData[0].type;
			new = 0;

			if (Mode && !CanPlaceTetr(0, PosPtr)) //종료 조건
			{
				GameOver = 2; //패배
				winner = 2;
			}
			else if ((Mode && ClearedLines >= 40))
			{
				GameOver = 3; //승리
				winner = 1;
			}
			else if (!Mode && !CanPlaceTetr(0, PosPtr))
			{
				GameOver = 1; //라인 비교
			}
			PrintSingleTetr(nexttetr, 0);
		}
		if (new2) //새로운 조각 출력
		{
			if (holdable2 == 2)
			{
				holdable2 = 0;
				GenerateTetr(1, TetrArray2Ptr, TetrCnt2Ptr, holdtetr2);
			}

			else
			{
				holdable2 = 1;
				GenerateTetr(1, TetrArray2Ptr, TetrCnt2Ptr, 0);
			}
			CurrentClearedLines2 = ClearRows(1); //꽉 찬 줄 개수 리턴, 자체적으로 제거
			if (CurrentClearedLines2)
			{
				ClearedLines2 += CurrentClearedLines2;
				if (!Mode)
				{
					FlagValue2 -= 2 * CurrentClearedLines2;
				}
				CurrentClearedLines2 = 0;
			}
			Pos2[0] = 4;
			Pos2[1] = 3;
			Pos2[2] = 4;
			Pos2[3] = 3;

			nexttetr2 = TetrominoArray2[1];
			defaulttype2 = TetrData[1].type;
			new2 = 0;

			if (Mode && !CanPlaceTetr(1, Pos2Ptr)) //종료 조건
			{
				GameOver2 = 2; //패배
				winner = 1;
			}
			else if ((Mode && ClearedLines2 >= 40))
			{
				GameOver2 = 3; //승리
				winner = 2;
			}
			else if (!Mode && !CanPlaceTetr(1, Pos2Ptr))
			{
				GameOver2 = 1; //라인 비교
			}
			PrintSingleTetr(nexttetr2, 2);
		}
		if (!(Flag % FlagValue))
		{
			if (Pos[1] > 3)
				TimePause = 0;
			Pos[1]++;
			PutTetr(0, PosPtr, !new);
			if (CanPlaceTetr(0, PosPtr))
			{
				PutTetr(0, PosPtr, 0);

				Pos[3] = Pos[1]; //y 업데이트
				WallPushStack = 0;
			}
			else
			{
				new = 1;
				Pos[1] = Pos[3];
				PutTetr(0, PosPtr, 0);
			}

		}
		if (!(Flag % FlagValue2))
		{
			if (Pos2[1] > 3)
				TimePause = 0;
			Pos2[1]++;
			PutTetr(1, Pos2Ptr, !new2);
			if (CanPlaceTetr(1, Pos2Ptr))
			{
				PutTetr(1, Pos2Ptr, 0);

				Pos2[3] = Pos2[1]; //y 업데이트
				WallPushStack2 = 0;
			}
			else
			{
				new2 = 1;
				Pos2[1] = Pos2[3];
				PutTetr(1, Pos2Ptr, 0);
			}

		}
		if (_kbhit())
		{
			Input = _getch(); //키 입력 확인
			switch (Input)
			{
			case ARROW: //arrow
				switch (_getch())
				{
				case UP:
					SpinTetr(1, Pos2Ptr, defaulttype2, 1);
					PutTetr(1, Pos2Ptr, 0);
					WallPushStack2 = 0;
					break;
				case DOWN:
					Pos2[1]++;
					WallPushStack2 = 0;
					break;
				case LEFT:
					WallPushStack2++;
					if (WallPushStack2 >= 5)
					{
						Pos2[1]++;
						WallPushStack2 = 0;
					}
					else
						Pos2[0]--;
					break;
				case RIGHT:
					WallPushStack2++;
					if (WallPushStack2 >= 5)
					{
						Pos2[1]++;
						WallPushStack2 = 0;
					}
					else
						Pos2[0]++;
					break;
				}
				PutTetr(1, Pos2Ptr, 1);
				if (CanPlaceTetr(1, Pos2Ptr))
				{
					PutTetr(1, Pos2Ptr, 0);
					Pos2[3] = Pos2[1];
					Pos2[2] = Pos2[0];
				}
				else
				{
					Pos2[1] = Pos2[3];
					Pos2[0] = Pos2[2];
					PutTetr(1, Pos2Ptr, 0);
				}
				break;
			case 'w':
			case 'W':
				SpinTetr(0, PosPtr, defaulttype, 1);
				PutTetr(0, PosPtr, 0);
				WallPushStack = 0;
				break;
			case 's':
			case 'S':
				Pos[1]++;
				WallPushStack = 0;
				PutTetr(0, PosPtr, 1);
				if (CanPlaceTetr(0, PosPtr))
				{
					PutTetr(0, PosPtr, 0);
					Pos[3] = Pos[1];
				}
				else
				{
					Pos[1] = Pos[3];
					PutTetr(0, PosPtr, 0);
				}
				break;
			case 'a':
			case 'A':
				WallPushStack++;
				if (WallPushStack >= 5)
				{
					Pos[1]++;
					WallPushStack = 0;
				}
				else
					Pos[0]--;
				PutTetr(0, PosPtr, 1);
				if (CanPlaceTetr(0, PosPtr))
				{
					PutTetr(0, PosPtr, 0);
					Pos[3] = Pos[1];
					Pos[2] = Pos[0];
				}
				else
				{
					Pos[1] = Pos[3];
					Pos[0] = Pos[2];
					PutTetr(0, PosPtr, 0);
				}
				break;
			case 'd':
			case 'D':
				WallPushStack++;
				if (WallPushStack >= 5)
				{
					Pos[1]++;
					WallPushStack = 0;
				}
				else
					Pos[0]++;
				PutTetr(0, PosPtr, 1);
				if (CanPlaceTetr(0, PosPtr))
				{
					PutTetr(0, PosPtr, 0);
					Pos[3] = Pos[1];
					Pos[2] = Pos[0];
				}
				else
				{
					Pos[1] = Pos[3];
					Pos[0] = Pos[2];
					PutTetr(0, PosPtr, 0);
				}
				break;
			case 'c': //1p hard drop
			case 'C':
				PutTetr(0, PosPtr, 1);
				for (i = 0; CanPlaceTetr(0, PosPtr); i++)
				{
					Pos[1]++;
					Pos[3] = Pos[1];
				}
				Pos[1]--;
				Pos[3]--;
				new = 1;
				PutTetr(0, PosPtr, 0);
				WallPushStack = 0;
				break;
			case 32: //2p hard drop
				PutTetr(1, Pos2Ptr, 1);
				for (i = 0; CanPlaceTetr(1, Pos2Ptr); i++)
				{
					Pos2[1]++;
					Pos2[3] = Pos2[1];
				}
				Pos2[1]--;
				Pos2[3]--;
				new2 = 1;
				PutTetr(1, Pos2Ptr, 0);
				WallPushStack2 = 0;
				break;
			case 'q': //1p left spin
			case 'Q':
				SpinTetr(0, PosPtr, defaulttype, 0);
				PutTetr(0, PosPtr, 0);
				WallPushStack = 0;
				break;
			case ',': //2p left spin
			case '<':
				SpinTetr(1, Pos2Ptr, defaulttype2, 0);
				PutTetr(1, Pos2Ptr, 0);
				WallPushStack2 = 0;
				break;
			case '.':
			case '>': //2p right spin
				SpinTetr(1, Pos2Ptr, defaulttype2, 1);
				PutTetr(1, Pos2Ptr, 0);
				WallPushStack2 = 0;
				break;
			case 'e': //1p HOLD
			case 'E':
				PutTetr(0, PosPtr, 1);

				if (holdable)
				{
					if (!holdtetr) //처음 hold
					{
						holdtetr = TetrominoArray[0];
						TetrominoArray[0] = TetrominoArray[1];
						while (1)
						{
							swaptemp = (rand() % 7);
							if (!TetrCnt[swaptemp])
								break;
						}
						TetrCnt[swaptemp] = 1;
						TetrominoArray[1] = swaptemp * 4 + 1;

					}
					else
					{
						TetrominoArray[0] = holdtetr;
					}
					holdtetr = defaulttype;
					PrintSingleTetr(holdtetr, 1);
					new = 1;
					holdable = 2;
					continue;
				}
				break;
			case '/': //2p HOLD
			case '?':
				PutTetr(1, Pos2Ptr, 1);

				if (holdable2)
				{
					if (!holdtetr2) //처음 hold
					{
						holdtetr2 = TetrominoArray2[0];
						TetrominoArray2[0] = TetrominoArray2[1];
						while (1)
						{
							swaptemp = (rand() % 7);
							if (!TetrCnt2[swaptemp])
								break;
						}
						TetrCnt2[swaptemp] = 1;
						TetrominoArray2[1] = swaptemp * 4 + 1;

					}
					else
					{
						TetrominoArray2[0] = holdtetr2;
					}
					holdtetr2 = defaulttype2;
					PrintSingleTetr(holdtetr2, 3);
					new2 = 1;
					holdable2 = 2;
					continue;
				}
				break;
			default:
				break;
			}
		}
		Sleep(1);
		Flag++;
		if (!TimePause)
			UpdateTime(ElapsedTimePtr);

		SetCur(24, 21);
		printf("Time : %.2lf", ElapsedTime);
		SetCur(24, 23);
		printf("Lines : %d", ClearedLines);
		SetCur(98, 21);
		printf("Time : %.2lf", ElapsedTime);
		SetCur(98, 23);
		printf("Lines : %d", ClearedLines2);
	}

	if (!winner)
	{
		if (ClearedLines > ClearedLines2)
			winner = 1;
		else if (ClearedLines == ClearedLines2)
		{
			if (GameOver)
				winner = 2;
			else
				winner = 1;
		}
		else
			winner = 2;
	}
	system("cls");
	Sleep(10);
	SetCur(55, 12);
	printf("%dp won !", winner);
	SetCur(47, 14);
	Sleep(1000);
	printf("press any key to continue");
	while (!_kbhit());
	Input = _getch();
}
void PutTetr(int fieldnum, int* pos, int del)
{
	//입력을 편하게 하고자 1글자 local variable 로 변경
	int f = fieldnum;
	int c;
	int x;
	int y;

	if (del)
	{
		c = 0;
		x = pos[2];
		y = pos[3];
	}
	else
	{
		c = TetrData[fieldnum].color;
		x = pos[0];
		y = pos[1];
	}

	switch (TetrData[fieldnum].type)
	{
		//O
	case 1: //O
	case 2:
	case 3:
	case 4:
		//	* * * *
		//	* # # *
		//	* # # *
		//	* * * *
		field[f][y - 2][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		field[f][y - 2][x + 2] = c;
		break;
		//Z
	case 5: // Z
		// * * * *
		// # # * *
		// * # # *
		// * * * *
		field[f][y - 2][x] = c;
		field[f][y - 2][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		break;
	case 6: // Z 90도
		// * * * *
		// * * # *
		// * # # *
		// * # * *
		field[f][y][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		field[f][y - 2][x + 2] = c;
		break;
	case 7: //Z 180도
		/** * * *
		* * * *
		# # * *
		* # # **/
		field[f][y - 1][x] = c;
		field[f][y][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y][x + 2] = c;
		break;
	case 8: //Z 270도
		// * * * *
		// * # * *
		// # # * *
		// # * * *
		field[f][y][x] = c;
		field[f][y - 1][x] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 2][x + 1] = c;
		break;
		//L
	case 9: // L
		// * * * *
		// * * # *
		// # # # *
		// * * * *
		field[f][y - 1][x] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		field[f][y - 2][x + 2] = c;
		break;
	case 10: // L 90도
		// * * * *
		// * # * *
		// * # * *
		// * # # *
		field[f][y][x + 1] = c;
		field[f][y][x + 2] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 2][x + 1] = c;
		break;
	case 11: // L 180도
		// * * * *
		// * * * *
		// # # # *
		// # * * *
		field[f][y][x] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		field[f][y - 1][x] = c;
		break;
	case 12: //L 270도
		// * * * *
		// # # * *
		// * # * *
		// * # * *
		field[f][y - 2][x] = c;
		field[f][y - 2][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y][x + 1] = c;
		break;
		//T
	case 13: // T
		// * * * *
		// * # * *
		// # # # *
		// * * * *
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x] = c;
		field[f][y - 2][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		break;
	case 14: //T 90도
		// * * * *
		// * # * *
		// * # # *
		// * # * *
		field[f][y][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		field[f][y - 2][x + 1] = c;
		break;
	case 15: //T 180도
		// * * * *
		// * * * *
		// # # # *
		// * # * *
		field[f][y - 1][x] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		break;
	case 16: //T 270
		// * * * *
		// * # * *
		// # # * *
		// * # * *
		field[f][y - 1][x] = c;
		field[f][y][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 2][x + 1] = c;
		break;
		//I
	case 17: // I
		// * * * *
		// # # # #
		// * * * *
		// * * * *
		field[f][y - 2][x] = c;
		field[f][y - 2][x + 1] = c;
		field[f][y - 2][x + 2] = c;
		field[f][y - 2][x + 3] = c;
		break;
	case 18: //I 90
		// * * # *
		// * * # *
		// * * # *
		// * * # *
		field[f][y][x + 2] = c;
		field[f][y - 1][x + 2] = c;
		field[f][y - 2][x + 2] = c;
		field[f][y - 3][x + 2] = c;
		break;
		//J
	case 19: //I 180
		// * * * *
		// * * * *
		// # # # #
		// * * * *
		field[f][y - 1][x] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		field[f][y - 1][x + 3] = c;
		break;
	case 20: //I 270
		// * # * *
		// * # * *
		// * # * *
		// * # * *
		field[f][y][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 2][x + 1] = c;
		field[f][y - 3][x + 1] = c;
		break;
	case 21: // J
		// * * * *
		// # * * *
		// # # # *
		// * * * *
		field[f][y - 1][x] = c;
		field[f][y - 2][x] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		break;
	case 22: // J 90
		// * * * *
		// * # # *
		// * # * *
		// * # * *
		field[f][y - 2][x + 1] = c;
		field[f][y - 2][x + 2] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y][x + 1] = c;
		break;
	case 23: //J 180
		// * * * *
		// * * * *
		// # # # *
		// * * # *
		field[f][y - 1][x] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		field[f][y][x + 2] = c;
		break;
	case 24: //J 270
		// * * * *
		// * # * *
		// * # * *
		// # # * *
		field[f][y][x + 1] = c;
		field[f][y][x] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 2][x + 1] = c;
		break;
		//S
	case 25: // S
		// * * * *
		// * # # *
		// # # * *
		// * * * *
		field[f][y - 2][x + 1] = c;
		field[f][y - 2][x + 2] = c;
		field[f][y - 1][x] = c;
		field[f][y - 1][x + 1] = c;
		break;
	case 26: // S 90도
		// * * * *
		// * # * *
		// * # # *
		// * * # *
		field[f][y - 2][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		field[f][y][x + 2] = c;
		break;
	case 27: //S 180
		// * * * *
		// * * * *
		// * # # *
		// # # * *
		field[f][y][x] = c;
		field[f][y][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		field[f][y - 1][x + 2] = c;
		break;
	case 28: //S 270
		// * * * *
		// # * * *
		// # # * *
		// * # * *
		field[f][y - 1][x] = c;
		field[f][y - 2][x] = c;
		field[f][y][x + 1] = c;
		field[f][y - 1][x + 1] = c;
		break;
	}
	PrintField(fieldnum, 0);
}
int CanPlaceTetr(int fieldnum, int* pos)
{
	//입력을 편하게 하고자 1글자 local variable 로 변경
	int f = fieldnum;
	int c;
	int x;
	int y;
	CopyField(fieldnum);

	if (pos[0] + TetrData[fieldnum].Leftend < 0)
		return 0;
	if (pos[0] + TetrData[fieldnum].Rightend > 9)
		return 0;
	if (pos[1] - TetrData[fieldnum].Bottomend > 23)
		return 0;
	c = TetrData[fieldnum].color;
	x = pos[0];
	y = pos[1];

	switch (TetrData[fieldnum].type) {
		//O
	case 1: // O Tetromino
	case 2:
	case 3:
	case 4:
		//   * * * *
		//   * # # *
		//   * # # *
		//   * * * *
		if (temp[f][y - 2][x + 1] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y - 1][x + 2] != 0 || temp[f][y - 2][x + 2] != 0)
			return 0;
		temp[f][y - 2][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		temp[f][y - 2][x + 2] = c;
		break;

	case 5: // Z Tetromino
		//   * * * *
		//   # # * *
		//   * # # *
		//   * * * *
		if (temp[f][y - 2][x] != 0 || temp[f][y - 2][x + 1] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y - 1][x + 2] != 0)
			return 0;
		temp[f][y - 2][x] = c;
		temp[f][y - 2][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		break;

	case 6: // Z Tetromino
		//   * * * *
		//   * * # *
		//   * # # *
		//   * # * *
		if (temp[f][y][x + 1] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y - 1][x + 2] != 0 || temp[f][y - 2][x + 2] != 0)
			return 0;
		temp[f][y][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		temp[f][y - 2][x + 2] = c;
		break;

	case 7: // Z Tetromino
		//   * * * *
		//   * * * *
		//   # # * *
		//   * # # *
		if (temp[f][y - 1][x] != 0 || temp[f][y][x + 1] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y][x + 2] != 0)
			return 0;
		temp[f][y - 1][x] = c;
		temp[f][y][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y][x + 2] = c;
		break;

	case 8: // Z Tetromino
		//   * * * *
		//   * # * *
		//   # # * *
		//   # * * *
		if (temp[f][y][x] != 0 || temp[f][y - 1][x] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y - 2][x + 1] != 0)
			return 0;
		temp[f][y][x] = c;
		temp[f][y - 1][x] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 2][x + 1] = c;
		break;

	case 9: // L Tetromino
		//   * * * *
		//   * * # *
		//   # # # *
		//   * * * *
		if (temp[f][y - 1][x] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y - 1][x + 2] != 0 || temp[f][y - 2][x + 2] != 0)
			return 0;
		temp[f][y - 1][x] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		temp[f][y - 2][x + 2] = c;
		break;

	case 10: // L Tetromino
		//   * * * *
		//   * # * *
		//   * # * *
		//   * # # *
		if (temp[f][y][x + 1] != 0 || temp[f][y][x + 2] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y - 2][x + 1] != 0)
			return 0;
		temp[f][y][x + 1] = c;
		temp[f][y][x + 2] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 2][x + 1] = c;
		break;

	case 11: // L Tetromino
		//   * * * *
		//   * * * *
		//   # # # *
		//   # * * *
		if (temp[f][y][x] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y - 1][x + 2] != 0 || temp[f][y - 1][x] != 0)
			return 0;
		temp[f][y][x] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		temp[f][y - 1][x] = c;
		break;

	case 12: // L Tetromino
		//   * * * *
		//   # # * *
		//   * # * *
		//   * # * *
		if (temp[f][y - 2][x] != 0 || temp[f][y - 2][x + 1] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y][x + 1] != 0)
			return 0;
		temp[f][y - 2][x] = c;
		temp[f][y - 2][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y][x + 1] = c;
		break;
	case 13: // T
		// * * * *
		// * # * *
		// # # # *
		// * * * *
		if (temp[f][y - 1][x + 1] != 0 || temp[f][y - 1][x] != 0 ||
			temp[f][y - 2][x + 1] != 0 || temp[f][y - 1][x + 2] != 0)
			return 0;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x] = c;
		temp[f][y - 2][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		break;
	case 14: // T 90도
		// * * * *
		// * # * *
		// * # # *
		// * # * *
		if (temp[f][y][x + 1] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y - 1][x + 2] != 0 || temp[f][y - 2][x + 1] != 0)
			return 0;
		temp[f][y][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		temp[f][y - 2][x + 1] = c;
		break;
	case 15: // T 180도
		// * * * *
		// * * * *
		// # # # *
		// * # * *
		if (temp[f][y - 1][x] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y][x + 1] != 0 || temp[f][y - 1][x + 2] != 0)
			return 0;
		temp[f][y - 1][x] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		break;
	case 16: // T 270도
		// * * * *
		// * # * *
		// # # * *
		// * # * *
		if (temp[f][y - 1][x] != 0 || temp[f][y][x + 1] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y - 2][x + 1] != 0)
			return 0;
		temp[f][y - 1][x] = c;
		temp[f][y][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 2][x + 1] = c;
		break;

		// I
	case 17: // I
		// * * * *
		// # # # #
		// * * * *
		// * * * *
		if (temp[f][y - 2][x] != 0 || temp[f][y - 2][x + 1] != 0 ||
			temp[f][y - 2][x + 2] != 0 || temp[f][y - 2][x + 3] != 0)
			return 0;
		temp[f][y - 2][x] = c;
		temp[f][y - 2][x + 1] = c;
		temp[f][y - 2][x + 2] = c;
		temp[f][y - 2][x + 3] = c;
		break;
	case 18: // I 90
		// * * # *
		// * * # *
		// * * # *
		// * * # *
		if (temp[f][y][x + 2] != 0 || temp[f][y - 1][x + 2] != 0 ||
			temp[f][y - 2][x + 2] != 0 || temp[f][y - 3][x + 2] != 0)
			return 0;
		temp[f][y][x + 2] = c;
		temp[f][y - 1][x + 2] = c;
		temp[f][y - 2][x + 2] = c;
		temp[f][y - 3][x + 2] = c;
		break;
	case 19: // I 180
		// * * * *
		// * * * *
		// # # # #
		// * * * *
		if (temp[f][y - 1][x] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y - 1][x + 2] != 0 || temp[f][y - 1][x + 3] != 0)
			return 0;
		temp[f][y - 1][x] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		temp[f][y - 1][x + 3] = c;
		break;
	case 20: // I 270
		// * # * *
		// * # * *
		// * # * *
		// * # * *
		if (temp[f][y][x + 1] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y - 2][x + 1] != 0 || temp[f][y - 3][x + 1] != 0)
			return 0;
		temp[f][y][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 2][x + 1] = c;
		temp[f][y - 3][x + 1] = c;
		break;
	case 21: // J
		// * * * *
		// # * * *
		// # # # *
		// * * * *
		if (temp[f][y - 1][x] != 0 || temp[f][y - 2][x] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y - 1][x + 2] != 0)
			return 0;
		temp[f][y - 1][x] = c;
		temp[f][y - 2][x] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		break;
	case 22: // J 90
		// * * * *
		// * # # *
		// * # * *
		// * # * *
		if (temp[f][y - 2][x + 1] != 0 || temp[f][y - 2][x + 2] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y][x + 1] != 0)
			return 0;
		temp[f][y - 2][x + 1] = c;
		temp[f][y - 2][x + 2] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y][x + 1] = c;
		break;
	case 23: // J 180
		// * * * *
		// * * * *
		// # # # *
		// * * # *
		if (temp[f][y - 1][x] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y - 1][x + 2] != 0 || temp[f][y][x + 2] != 0)
			return 0;
		temp[f][y - 1][x] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		temp[f][y][x + 2] = c;
		break;
	case 24: // J 270
		// * * * *
		// * # * *
		// * # * *
		// # # * *
		if (temp[f][y][x + 1] != 0 || temp[f][y][x] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y - 2][x + 1] != 0)
			return 0;
		temp[f][y][x + 1] = c;
		temp[f][y][x] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 2][x + 1] = c;
		break;
	case 25: // S
		// * * * *
		// * # # *
		// # # * *
		// * * * *
		if (temp[f][y - 2][x + 1] != 0 || temp[f][y - 2][x + 2] != 0 ||
			temp[f][y - 1][x] != 0 || temp[f][y - 1][x + 1] != 0)
			return 0;
		temp[f][y - 2][x + 1] = c;
		temp[f][y - 2][x + 2] = c;
		temp[f][y - 1][x] = c;
		temp[f][y - 1][x + 1] = c;
		break;
	case 26: // S 90도
		// * * * *
		// * # * *
		// * # # *
		// * * # *
		if (temp[f][y - 2][x + 1] != 0 || temp[f][y - 1][x + 1] != 0 ||
			temp[f][y - 1][x + 2] != 0 || temp[f][y][x + 2] != 0)
			return 0;
		temp[f][y - 2][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		temp[f][y][x + 2] = c;
		break;
	case 27: // S 180
		// * * * *
		// * * * *
		// * # # *
		// # # * *
		if (temp[f][y][x] != 0 || temp[f][y][x + 1] != 0 ||
			temp[f][y - 1][x + 1] != 0 || temp[f][y - 1][x + 2] != 0)
			return 0;
		temp[f][y][x] = c;
		temp[f][y][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		temp[f][y - 1][x + 2] = c;
		break;
	case 28: // S 270
		// * * * *
		// # * * *
		// # # * *
		// * # * *
		if (temp[f][y - 1][x] != 0 || temp[f][y - 2][x] != 0 ||
			temp[f][y][x + 1] != 0 || temp[f][y - 1][x + 1] != 0)
			return 0;
		temp[f][y - 1][x] = c;
		temp[f][y - 2][x] = c;
		temp[f][y][x + 1] = c;
		temp[f][y - 1][x + 1] = c;
		break;
	}

	return 1;
}
int ClearRows(int fieldnum) // 꽉 찬 줄 지우고 지운 개수 리턴
{
	int i, j, r, c;
	int clearrow;
	int fullrows = 0;

	for (i = 23; i >= 0; i--)
	{
		clearrow = 1;

		for (j = 0; j < 10; j++)
		{
			if (!field[fieldnum][i][j])
			{
				clearrow = 0;
				break;
			}
		}

		if (clearrow)
		{
			fullrows++;

			//한 줄씩 내림
			for (r = i; r > 0; r--)
			{
				for (c = 0; c < 10; c++)
				{
					field[fieldnum][r][c] = field[fieldnum][r - 1][c];
				}
			}


			for (c = 0; c < 10; c++)
			{
				field[fieldnum][0][c] = 0;
			}

			i++;
		}
	}

	return fullrows;
}
int PrintField(int fieldno, int raw) //
{
	int x;
	int y;
	int fieldnum = fieldno + 1;
	int i, j;

	if (raw) //raw 면 오른쪽에 추가로 출력
	{
		x = R13x + 1;
		y = LR12y - 3;
		for (i = 0; i < 24; i++)
		{
			for (j = 0; j < 10; j++)
			{
				SetCur(x + 2 * j, y + i);
				SetColor(15, 16);
				printf("%d ", field[fieldno][i][j]);
			}
		}
	}
	if (fieldnum == 1) //Game Fields
	{
		y = LR12y - 3;
		x = L13x + 1;

	}
	else
	{
		x = R13x + 1;
		y = LR12y - 3;
	}
	for (i = 0; i < 24; i++)
	{
		for (j = 0; j < 10; j++)
		{
			SetCur(x + 2 * j, y + i);
			SetColor(field[fieldno][i][j], field[fieldno][i][j]);
			printf("||");
		}
	}
	SetColor(15, 16);
	return 1;
}
void PrintSingleTetr(int type, int loc) //0:1p next, 1:1p hold, 2:2p next, 3: 2p hold
{
	int x, y;
	int i, j;

	if (loc <= 1)
		x = 24;
	else
		x = 98;
	if (loc == 0 || loc == 2)
		y = 8;
	else
		y = 18;

	for (i = 0; i < 4; i++)
	{
		SetCur(x, y - i);
		printf("        ");
	}

	switch (type)
	{
	case 1:
		SetColor(6, 6);
		SetCur(x + 2, y - 2);
		printf("////");
		SetCur(x + 2, y - 1);
		printf("////");
		break;
	case 5:
		SetColor(10, 10);
		SetCur(x, y - 2);
		printf("////");
		SetCur(x + 2, y - 1);
		printf("////");
		break;
	case 9:
		SetColor(12, 12);
		//   * * * *
		//   * * # *
		//   # # # *
		//   * * * *
		SetCur(x + 4, y - 2);
		printf("//");
		SetCur(x, y - 1);
		printf("//////");
		break;
	case 13:
		SetColor(13, 13);
		// * * * *
		// * # * *
		// # # # *
		// * * * *
		SetCur(x + 2, y - 2);
		printf("//");
		SetCur(x, y - 1);
		printf("//////");
		break;
	case 17:
		SetColor(11, 11);
		SetCur(x, y - 2);
		printf("////////");
		break;
	case 21:
		SetColor(1, 1);
		// * * * *
		// # * * *
		// # # # *
		// * * * *
		SetCur(x, y - 2);
		printf("//");
		SetCur(x, y - 1);
		printf("//////");
		break;
	case 25:
		SetColor(4, 4);
		// * * * *
		// * # # *
		// # # * *
		// * * * *
		SetCur(x + 2, y - 2);
		printf("////");
		SetCur(x, y - 1);
		printf("////");
		break;
	}
	SetColor(15, 16);
}
void CopyField(int fieldnum) //field를 temp로 복사
{
	int i, j;
	for (i = 0; i < 24; i++)
	{
		for (j = 0; j < 10; j++)
		{
			temp[fieldnum][i][j] = field[fieldnum][i][j];
		}
	}
}
void BasicSpin(int fieldnum, int defaulttype, int dir)
{
	if (dir)
	{
		if (TetrData[fieldnum].type == defaulttype + 3)
		{
			TetrData[fieldnum].type = defaulttype;
		}
		else
		{
			TetrData[fieldnum].type++;
		}
	}
	else
	{
		if (TetrData[fieldnum].type == defaulttype)
		{
			TetrData[fieldnum].type = defaulttype + 3;
		}
		else
		{
			TetrData[fieldnum].type--;
		}
	}
	//왼쪽,아래,오른쪽 끝 좌표
	switch (TetrData[fieldnum].type)
	{
		//O
	case 1: case 2: case 3: case 4:
		TetrData[fieldnum].Leftend = 1;
		TetrData[fieldnum].Bottomend = 1;
		TetrData[fieldnum].Rightend = 2;
		break;
		//I
	case 17:
		TetrData[fieldnum].Leftend = 0;
		TetrData[fieldnum].Bottomend = 2;
		TetrData[fieldnum].Rightend = 3;
		break;
	case 18:
		TetrData[fieldnum].Leftend = 2;
		TetrData[fieldnum].Bottomend = 0;
		TetrData[fieldnum].Rightend = 2;
		break;
	case 19:
		TetrData[fieldnum].Leftend = 0;
		TetrData[fieldnum].Bottomend = 1;
		TetrData[fieldnum].Rightend = 3;
		break;
	case 20:
		TetrData[fieldnum].Leftend = 0;
		TetrData[fieldnum].Bottomend = 0;
		TetrData[fieldnum].Rightend = 1;
		break;
	default:
		if (TetrData[fieldnum].type % 4 == 1)
		{
			TetrData[fieldnum].Leftend = 0;
			TetrData[fieldnum].Bottomend = 1;
			TetrData[fieldnum].Rightend = 2;
		}
		else if (TetrData[fieldnum].type % 4 == 2)
		{
			TetrData[fieldnum].Leftend = 1;
			TetrData[fieldnum].Bottomend = 0;
			TetrData[fieldnum].Rightend = 2;
		}
		else if (TetrData[fieldnum].type % 4 == 3)
		{
			TetrData[fieldnum].Leftend = 0;
			TetrData[fieldnum].Bottomend = 0;
			TetrData[fieldnum].Rightend = 2;
		}
		else if (TetrData[fieldnum].type % 4 == 0)
		{
			TetrData[fieldnum].Leftend = 0;
			TetrData[fieldnum].Bottomend = 0;
			TetrData[fieldnum].Rightend = 1;
		}
	}
}
void SpinTetr(int fieldnum, int* Pos, int defaulttype, int dir)
{
	int clear = 1;
	int x = Pos[0];
	int y = Pos[1];
	PutTetr(fieldnum, Pos, 1); //이전 출력 지움

	if (dir) //ClockWise
	{
		BasicSpin(fieldnum, defaulttype, 1);
		//일단 회전

		if (!CanPlaceTetr(fieldnum, Pos)) //안되면 Wall Kick : 총 5단계
		{
			if (TetrData[fieldnum].type >= 17 && TetrData[fieldnum].type <= 20) //I
			{
				switch (TetrData[fieldnum].type % 4)
				{
				case 1: //0 (기본상태)
					Pos[0] = x - 2;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x + 1;
						Pos[1] = y;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x - 2;
							Pos[1] = y - 1;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x + 1;
								Pos[1] = y + 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 2: //R //90도
					Pos[0] = x - 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x + 2;
						Pos[1] = y;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x - 1;
							Pos[1] = y + 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x + 2;
								Pos[1] = y - 1;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 3: //2 //180도
					Pos[0] = x + 2;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x - 1;
						Pos[1] = y;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x + 2;
							Pos[1] = y + 1;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x - 1;
								Pos[1] = y - 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 4: //L //270도
					Pos[0] = x + 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x - 2;
						Pos[1] = y;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x + 1;
							Pos[1] = y - 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x - 2;
								Pos[1] = y + 1;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				}
			}
			else if (TetrData[fieldnum].type >= 5)
			{
				switch (TetrData[fieldnum].type % 4)
				{
				case 1: //0 (기본상태)
					Pos[0] = x - 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x - 1;
						Pos[1] = y + 1;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x;
							Pos[1] = y - 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x - 1;
								Pos[1] = y - 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 2: //R //90도
					Pos[0] = x + 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x + 1;
						Pos[1] = y - 1;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x;
							Pos[1] = y + 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x + 1;
								Pos[1] = y + 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 3: //2 //180도
					Pos[0] = x + 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x + 1;
						Pos[1] = y + 1;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x;
							Pos[1] = y - 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x + 1;
								Pos[1] = y - 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 4: //L //270도
					Pos[0] = x - 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x - 1;
						Pos[1] = y - 1;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x;
							Pos[1] = y + 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x - 1;
								Pos[1] = y + 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				}
			}
		}
	}

	else
	{
		BasicSpin(fieldnum, defaulttype, 0);

		if (!CanPlaceTetr(fieldnum, Pos)) //안되면 Wall Kick : 총 5단계
		{
			if (TetrData[fieldnum].type >= 17 && TetrData[fieldnum].type <= 20) //I
			{
				switch (TetrData[fieldnum].type % 4)
				{
				case 1: //0 (기본상태)
					Pos[0] = x - 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x + 2;
						Pos[1] = y;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x - 1;
							Pos[1] = y + 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x + 2;
								Pos[1] = y - 1;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 2: //R //90도
					Pos[0] = x + 2;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x - 1;
						Pos[1] = y;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x + 2;
							Pos[1] = y + 1;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x - 1;
								Pos[1] = y - 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 3: //2 //180도
					Pos[0] = x + 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x - 2;
						Pos[1] = y;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x + 1;
							Pos[1] = y - 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x - 2;
								Pos[1] = y + 1;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 4: //L //270도
					Pos[0] = x - 2;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x + 1;
						Pos[1] = y;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x - 2;
							Pos[1] = y - 1;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x + 1;
								Pos[1] = y + 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				}
			}
			else if (TetrData[fieldnum].type >= 5)
			{
				switch (TetrData[fieldnum].type % 4)
				{
				case 1: //0 (기본상태)
					Pos[0] = x + 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x + 1;
						Pos[1] = y + 1;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x;
							Pos[1] = y - 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x + 1;
								Pos[1] = y - 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 2: //R //90도
					Pos[0] = x + 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x + 1;
						Pos[1] = y - 1;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x;
							Pos[1] = y + 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x + 1;
								Pos[1] = y + 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 3: //2 //180도
					Pos[0] = x - 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x - 1;
						Pos[1] = y + 1;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x;
							Pos[1] = y - 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x - 1;
								Pos[1] = y - 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				case 4: //L //270도
					Pos[0] = x - 1;
					Pos[1] = y;
					if (!CanPlaceTetr(fieldnum, Pos))
					{
						Pos[0] = x - 1;
						Pos[1] = y - 1;
						if (!CanPlaceTetr(fieldnum, Pos))
						{
							Pos[0] = x;
							Pos[1] = y + 2;
							if (!CanPlaceTetr(fieldnum, Pos))
							{
								Pos[0] = x - 1;
								Pos[1] = y + 2;
								if (!CanPlaceTetr(fieldnum, Pos))
								{
									clear = 0;
									Pos[0] = x;
									Pos[1] = y;
									break;
								}
								else
									break;
							}
							else
								break;
						}
						else
							break;
					}
					else
						break;
				}
			}
		}
	}

	if (clear)
	{
		Pos[2] = Pos[0];
		Pos[3] = Pos[1];
	}
	else
	{
		BasicSpin(fieldnum, defaulttype, !dir);
		Pos[0] = Pos[2];
		Pos[1] = Pos[3];
	}



}
void GenerateTetr(int fieldnum, int* TetrArray, int* Cnt, int type) //타입을 랜덤으로 . 가로 길이, 컬러 설정
{
	int randnum;
	int overlap = 0;
	int i, j;
	int FullCnt = 1;
	srand(time(NULL));


	for (i = 0; i < 7; i++)
	{
		if (!Cnt[i])
			FullCnt = 0;
	}
	if (FullCnt)
	{
		for (i = 0; i < 7; i++)
			Cnt[i] = 0;
	}

	if (!type)
	{
		TetrArray[0] = TetrArray[1];
		while (1)
		{
			randnum = (rand() % 7);
			if (!Cnt[randnum] && randnum != randomvalue)
				break;
		}
		Cnt[randnum] = 1;
		TetrArray[1] = randnum * 4 + 1;
		randomvalue = randnum;
	}

	TetrData[fieldnum].type = TetrArray[0];
	//color (type 기준) 
	if (TetrData[fieldnum].type <= 4)
		TetrData[fieldnum].color = 6;
	else if (TetrData[fieldnum].type <= 8)
		TetrData[fieldnum].color = 10;
	else if (TetrData[fieldnum].type <= 12)
		TetrData[fieldnum].color = 12;
	else if (TetrData[fieldnum].type <= 16)
		TetrData[fieldnum].color = 13;
	else if (TetrData[fieldnum].type <= 20)
		TetrData[fieldnum].color = 11;
	else if (TetrData[fieldnum].type <= 24)
		TetrData[fieldnum].color = 1;
	else
		TetrData[fieldnum].color = 4;

	//왼쪽,아래,오른쪽 끝 좌표
	switch (TetrData[fieldnum].type)
	{
		//O
	case 1:
		TetrData[fieldnum].Leftend = 1;
		TetrData[fieldnum].Bottomend = 1;
		TetrData[fieldnum].Rightend = 2;
		break;
		//I
	case 17:
		TetrData[fieldnum].Leftend = 0;
		TetrData[fieldnum].Bottomend = 2;
		TetrData[fieldnum].Rightend = 3;
		break;
	default:
		TetrData[fieldnum].Leftend = 0;
		TetrData[fieldnum].Bottomend = 1;
		TetrData[fieldnum].Rightend = 2;
	}
}
void Title()
{
	int running = 1;
	char input;

	PrintTitle();

	while (running)
	{

		if (_kbhit() == 1) {
			input = _getch();
			switch (input) {
			case 'a':
			case 'A':
				ClassicGame(0);

				break;
			case 's':
			case 'S':
				ClassicGame(1);

				break;
			case 'd':
			case 'D':
				DualGame(0);
				break;
			case 'f':
			case 'F':
				DualGame(1);
				break;
			case 'q':
			case 'Q':
				running = 0;
				break;
			}

			PrintTitle();
		}
	}
}
void PrintTitle()
{
	system("cls");
	char arr[5][100] = {
		"========    ========    ========    =======      ======     ======  ",
		"   ==       ==             ==       ==    ==       ==      ===      ",
		"   ==       =======        ==       =======        ==         ==    ",
		"   ==       ==             ==       ==    ==       ==           === ",
		"   ==       ========       ==       ==    ==     ======     ======  " };
	int i, j;
	int textY = 4;
	SetColor(13, 16);

	SetCur(C, 1);
	printf("┏");
	for (i = 0; i < 76; i++)
		printf("━");
	printf("┓");
	for (i = 0; i < 12; i++)
	{
		SetCur(98, 2 + i);
		printf("┃");

	}
	SetCur(C, 2);
	for (i = 0; i < 12; i++)
	{
		SetCur(C, 2 + i);
		printf("┃");

	}
	SetCur(C, 13);
	printf("┖");
	for (i = 0; i < 25; i++)
		printf("━");
	printf("┓");
	for (i = 0; i < 11; i++)
	{
		SetCur(47, 14 + i);
		printf("┃");

	}
	SetCur(47, 24);
	printf("┖");
	for (i = 0; i < 25; i++)
		printf("━");
	printf("┛");
	for (i = 0; i < 10; i++)
	{
		SetCur(73, 14 + i);
		printf("┃");

	}
	SetCur(73, 13);
	printf("┏");
	for (i = 0; i < 24; i++)
		printf("━");
	printf("┛");
	SetCur(27, textY);

	for (i = 0; i < 5; i++)
	{
		for (j = 0; j < strlen(arr[i]); j++)
		{
			SetCur(26 + j, textY + i);
			if (arr[i][j] != ' ')
			{
				SetColor(13, 13);
			}
			else
			{
				SetColor(13, 16);
			}
			printf("%c", arr[i][j]);
		}
		Sleep(80);
	}

	SetColor(13, 16);
	SetCur(14, 17);
	printf("[A] 1p Classic Mode");
	SetCur(14, 19);
	printf("[S] 1p 40 Lines");
	SetCur(90, 17);
	printf("[D] 2p Classic Mode");
	SetCur(90, 19);
	printf("[F] 2p 40 Lines");
	SetCur(90, 21);
	printf("[Q] to quit");
	SetCur(55, 13);
	printf("On Console");

	SetColor(15, 16);
}
void UpdateTime(double* ElapsedTime)
{
	clock_t CurrentTime = clock();
	*ElapsedTime += (double)(CurrentTime - LastTime) / CLOCKS_PER_SEC;
	LastTime = CurrentTime;
}
void DebugMode()
{
	ShowCur();
	int running = 1;
	int Input;
	COORD pos;


	while (running)
	{
		pos = GetCur();
		if (_kbhit() == 1) {
			Input = _getch();
			switch (Input) {
			case 'a':
			case 'A':
				SetCur(pos.X - 1, pos.Y);
				break;
			case 'w':
			case 'W':
				SetCur(pos.X, pos.Y - 1);
				break;
			case 's':
			case 'S':
				SetCur(pos.X, pos.Y + 1);
				break;
			case 'd':
			case 'D':
				SetCur(pos.X + 1, pos.Y);
				break;
			case 'e':
			case 'E':
				SetCur(pos.X - 4, pos.Y + 1);
				printf("(%d, %d)", pos.X, pos.Y);
				SetCur(pos.X, pos.Y);
				break;
			case 'q':
			case 'Q':
				running = 0;
				break;
			}
		}
	}
	HideCur();
}
void Template(int dual)
{
#define Horizontal 20
#define Vertical 20
	int i, j;

	SetColor(15, 16);

	//Left1
	SetCur(L13x, LR12y);
	//printf("┏━━━━━━━━━━━━━━━━━━━━┓");
	for (i = 0; i < 20; i++)
	{
		SetCur(L13x, LR12y + 1 + i);
		printf("┃                    ┃");
	}
	SetCur(L13x, LR12y + Vertical + 1);
	printf("┖━━━━━━━━━━━━━━━━━━━━┛");

	//Left_Next
	SetCur(L24x, LR12y);
	printf("┏━ NEXT ━┓");
	for (i = 0; i < 4; i++)
	{
		SetCur(L24x, LR12y + 1 + i);
		printf("┃        ┃");
	}
	SetCur(L24x, LR12y + 1 + 4);
	printf("┖━━━━━━━━┛");

	//Left_Hold
	SetCur(L24x, LR12y + 10);
	printf("┏━ HOLD ━┓");
	for (i = 0; i < 4; i++)
	{
		SetCur(L24x, LR12y + 1 + i + 10);
		printf("┃        ┃");
	}
	SetCur(L24x, LR12y + 1 + 4 + 10);
	printf("┖━━━━━━━━┛");

	if (dual) //양쪽 출력
	{
		//Right1
		SetCur(R13x, LR12y);
		//printf("┏━━━━━━━━━━━━━━━━━━━━┓");
		for (i = 0; i < 20; i++)
		{
			SetCur(R13x, LR12y + 1 + i);
			printf("┃                    ┃");
		}
		SetCur(R13x, LR12y + Vertical + 1);
		printf("┖━━━━━━━━━━━━━━━━━━━━┛");

		//Right_Next
		SetCur(R24x, LR12y);
		printf("┏━ NEXT ━┓");
		for (i = 0; i < 4; i++)
		{
			SetCur(R24x, LR12y + 1 + i);
			printf("┃        ┃");
		}
		SetCur(R24x, LR12y + 1 + 4);
		printf("┖━━━━━━━━┛");

		//Right_Hold
		SetCur(R24x, LR12y + 10);
		printf("┏━ HOLD ━┓");
		for (i = 0; i < 4; i++)
		{
			SetCur(R24x, LR12y + 1 + i + 10);
			printf("┃        ┃");
		}
		SetCur(R24x, LR12y + 1 + 4 + 10);
		printf("┖━━━━━━━━┛");
	}


	SetCur(100, 100);


}

void SetCur(int X, int Y)
{
	HANDLE Screen;
	Screen = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD Position = { X, Y };

	SetConsoleCursorPosition(Screen, Position);
}
COORD GetCur(void) { //현재 커서 좌표를 COORD 로 리턴
	COORD curPoint;
	CONSOLE_SCREEN_BUFFER_INFO pos;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &pos);
	curPoint.X = pos.dwCursorPosition.X;
	curPoint.Y = pos.dwCursorPosition.Y;
	return curPoint;
}
void SetColor(int text, int bg) { //출력 색 변경 (텍스트, 배경)
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), text | (bg << 4));
}
void HideCur() { //커서 표시 숨김 (토글)
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = FALSE;
	SetConsoleCursorInfo(consoleHandle, &info);
}
void ShowCur() {   //커서 표시 보이게 함 (토글)
	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO info;
	info.dwSize = 100;
	info.bVisible = TRUE;
	SetConsoleCursorInfo(consoleHandle, &info);
}
