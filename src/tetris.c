// 俄罗斯方块的C语言版本

#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>
#include "resource.h"
#pragma warning(disable : 6386)
#define SPEED 1100
#define TIMER_ID 73

// 全局变量
const char title[] = "俄罗斯方块";
BOOL blocks[20][10];
int blockPos[4][2];
BOOL nextBlockShow[6];
int score;
int maxScore;
int nextBlock; // 下一个方块的号码
HPEN whitePen, blackPen;
HBRUSH whiteBrush, grayBrush;
HFONT font;

// 函数声明
LRESULT __stdcall WndProc(HWND, UINT, WPARAM, LPARAM); // 窗口响应
void paint(HWND); // 绘制（BeginPaint()）
void rePaint(HWND); // 绘制（GetDC()）
void newBlock(); // 新增方块
void setNextBlock(int); // 设置nextBlockShow
void move(HWND); // 移动并消行
int saveScore(int); // 获取并保存最高记录
void toLeft(); // 左移
void toRight(); // 右移
void toReSet(); // 旋转
void clear(); // 清屏

int __stdcall WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR szCmdLine, _In_ int iCmdShow)
{
	const char szAppName[] = "TetrisByTalkDe";
	HWND hwnd;
	MSG msg;
	WNDCLASSEX wndclass;
	
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, (LPSTR)IDI_MYICON);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(hInstance, (LPSTR)IDI_MYICON);
	
	if (!RegisterClassEx(&wndclass))
	{
		MessageBox(NULL, "窗口生成错误！", title, MB_OK | MB_ICONERROR);
		return -1;
	}
	
	hwnd = CreateWindowEx(
		0,
		szAppName,
		title,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPSIBLINGS,
		100,
		100,
		440, // 5*11 + 20*10 + 5 + 150 + 30
		545, // 5*21 + 20*20 + 40
		NULL,
		NULL,
		hInstance,
		NULL
	);
	
	if (hwnd == NULL)
	{
		MessageBox(NULL, "窗口生成错误！", title, MB_OK | MB_ICONERROR);
		return -1;
	}
	
	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);
	
	while (GetMessage(&msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return (int)msg.wParam;
}

LRESULT __stdcall WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		srand((unsigned int)time(NULL)); // 初始化随机数生成器
		nextBlock = (rand() / 93) % 9; // 初始化第一个方块
		setNextBlock(nextBlock);
		whitePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
		blackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
		grayBrush = CreateSolidBrush(RGB(127, 127, 127));
		// 画笔、画刷初始化
		font = CreateFont(
			16,
			0,
			0,
			0,
			FW_NORMAL,
			FALSE,
			FALSE,
			FALSE,
			DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE,
			"宋体"
		);
		maxScore = saveScore(0); // 获取最高记录
		newBlock(); // 生成第一个方块
		SetTimer(hwnd, TIMER_ID, SPEED, NULL); // 初始化计时器
		return 0;
		
		case WM_PAINT:
		paint(hwnd);
		return 0;
		
		case WM_TIMER:
		move(hwnd);
		rePaint(hwnd);
		return 0;
		
		case WM_KEYDOWN:
		switch (wParam)
		{
			case 'A':
			case VK_LEFT:
			toLeft();
			rePaint(hwnd);
			break;
			
			case 'D':
			case VK_RIGHT:
			toRight();
			rePaint(hwnd);
			break;
			
			case 'W':
			case VK_UP:
			toReSet();
			rePaint(hwnd);
			break;
			
			case 'S':
			case VK_DOWN:
			move(hwnd);
			rePaint(hwnd);
			break;
		}
		return 0;
		
		case WM_DESTROY:
		DeleteObject(whitePen);
		DeleteObject(blackPen);
		DeleteObject(whiteBrush);
		DeleteObject(grayBrush);
		PostQuitMessage(0);
		return 0;
	}
	
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void paint(HWND hwnd)
{
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect;
	TEXTMETRIC tm;
	
	hdc = BeginPaint(hwnd, &ps);
	GetClientRect(hwnd, &rect);
	GetTextMetrics(hdc, &tm);
	
	SelectObject(hdc, font);
	SelectObject(hdc, whitePen);
	SelectObject(hdc, whiteBrush);
	Rectangle(hdc, 0, 0, 420, 515); // 刷底色
	SelectObject(hdc, blackPen);
	MoveToEx(hdc, 255, 0, NULL);
	LineTo(hdc, 255, 505); // 绘制平分线
	for (int i = 0; i < 20; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			if (blocks[i][j])
			{
				SelectObject(hdc, grayBrush);
			}
			else
			{
				SelectObject(hdc, whiteBrush);
			}
			Rectangle(hdc, 5 + 25 * j, 5 + 25 * i, 25 * (j + 1), 25 * (i + 1)); // 绘制方块
		}
	}

	char b[16];
	TextOut(hdc, 260, 30, "分数", (int)strlen("分数"));
	sprintf_s(b, 16, "%06d", score);
	TextOut(hdc, 260, 30 + tm.tmHeight, b, 6);
	TextOut(hdc, 260, 30 + 2 * tm.tmHeight, "最高分", (int)strlen("最高分"));
	sprintf_s(b, 16, "%06d", maxScore);
	TextOut(hdc, 260, 30 + 3 * tm.tmHeight, b, 6); // 绘制文字信息
	TextOut(hdc, 260, 30 + 5 * tm.tmHeight, "TalkDe出品，", (int)strlen("TalkDe出品，"));
	TextOut(hdc, 260, 30 + 6 * tm.tmHeight, "必属精品。", (int)strlen("必属精品。"));
	TextOut(hdc, 260, 30 + 8 * tm.tmHeight, "初稿完成时间：", (int)strlen("初稿完成时间："));
	TextOut(hdc, 260, 30 + 9 * tm.tmHeight, "2021-07-21", (int)strlen("2021-07-21"));
	TextOut(hdc, 260, 30 + 11 * tm.tmHeight, "使用方法：", (int)strlen("使用方法"));
	TextOut(hdc, 260, 30 + 12 * tm.tmHeight, "用“WASD”四键控制，", (int)strlen("用“WASD”四键控制，"));
	TextOut(hdc, 260, 30 + 13 * tm.tmHeight, "需要再英文输入模式下", (int)strlen("需要再英文输入模式下"));
	TextOut(hdc, 260, 30 + 14 * tm.tmHeight, "控制；当然，也可以使", (int)strlen("控制。当然，也可以使"));
	TextOut(hdc, 260, 30 + 15 * tm.tmHeight, "用方向键控制。", (int)strlen("用方向键控制。"));

	// 绘制下一个方块
	for (int i = 0; i < 2; ++i)
	{
		if (nextBlockShow[i])
		{
			SelectObject(hdc, grayBrush);
		}
		else
		{
			SelectObject(hdc, whiteBrush);
		}
		Rectangle(hdc, 285 + 25 * i, 305, 305 + 25 * i, 325); // 绘制方块
	}
	for (int i = 0; i < 4; ++i)
	{
		if (nextBlockShow[i + 2])
		{
			SelectObject(hdc, grayBrush);
		}
		else
		{
			SelectObject(hdc, whiteBrush);
		}
		Rectangle(hdc, 260 + 25 * i, 330, 280 + 25 * i, 350); // 绘制方块
	}

	EndPaint(hwnd, &ps);
}

void rePaint(HWND hwnd)
{
	HDC hdc;
	TEXTMETRIC tm;
	
	hdc = GetDC(hwnd);
	GetTextMetrics(hdc, &tm);
	
	SelectObject(hdc, font);
	SelectObject(hdc, blackPen);
	for (int i = 0; i < 20; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			if (blocks[i][j])
			{
				SelectObject(hdc, grayBrush);
			}
			else
			{
				SelectObject(hdc, whiteBrush);
			}
			Rectangle(hdc, 5 + 25 * j, 5 + 25 * i, 25 * (j + 1), 25 * (i + 1));
		}
	}
	char b[16];
	sprintf_s(b, 16, "%06d", score);
	TextOut(hdc, 260, 30 + tm.tmHeight, b, 6);
	sprintf_s(b, 16, "%06d", maxScore);
	TextOut(hdc, 260, 30 + 3 * tm.tmHeight, b, 6);

	// 绘制下一个方块
	for (int i = 0; i < 2; ++i)
	{
		if (nextBlockShow[i])
		{
			SelectObject(hdc, grayBrush);
		}
		else
		{
			SelectObject(hdc, whiteBrush);
		}
		Rectangle(hdc, 285 + 25 * i, 305, 305 + 25 * i, 325); // 绘制方块
	}
	for (int i = 0; i < 4; ++i)
	{
		if (nextBlockShow[i + 2])
		{
			SelectObject(hdc, grayBrush);
		}
		else
		{
			SelectObject(hdc, whiteBrush);
		}
		Rectangle(hdc, 260 + 25 * i, 330, 280 + 25 * i, 350); // 绘制方块
	}
	
	ReleaseDC(hwnd, hdc);
}

void newBlock()
{
	int thisBlock = nextBlock;
	nextBlock = (rand() / 93) % 9;
	if (nextBlock == thisBlock) // 如果重复就重算
	{
		nextBlock = (rand() / 93) % 9;
	}

	switch (thisBlock)
	{
	case 0:
		blockPos[0][0] = -1;
		blockPos[0][1] = 3;
		blockPos[1][0] = -1;
		blockPos[1][1] = 4;
		blockPos[2][0] = -1;
		blockPos[2][1] = 5;
		blockPos[3][0] = -1;
		blockPos[3][1] = 6;
		break;
		
	case 1:
		blockPos[0][0] = -2;
		blockPos[0][1] = 3;
		blockPos[1][0] = -1;
		blockPos[1][1] = 3;
		blockPos[2][0] = -1;
		blockPos[2][1] = 4;
		blockPos[3][0] = -1;
		blockPos[3][1] = 5;
		break;
		
	case 2:
		blockPos[0][0] = -1;
		blockPos[0][1] = 3;
		blockPos[1][0] = -1;
		blockPos[1][1] = 4;
		blockPos[2][0] = -1;
		blockPos[2][1] = 5;
		blockPos[3][0] = -2;
		blockPos[3][1] = 5;
		break;
		
	case 3:
	case 4:
		blockPos[0][0] = -2;
		blockPos[0][1] = 4;
		blockPos[1][0] = -1;
		blockPos[1][1] = 4;
		blockPos[2][0] = -2;
		blockPos[2][1] = 5;
		blockPos[3][0] = -1;
		blockPos[3][1] = 5;
		break;
		
	case 5:
		blockPos[0][0] = -2;
		blockPos[0][1] = 3;
		blockPos[1][0] = -2;
		blockPos[1][1] = 4;
		blockPos[2][0] = -1;
		blockPos[2][1] = 4;
		blockPos[3][0] = -1;
		blockPos[3][1] = 5;
		break;
		
	case 6:
		blockPos[0][0] = -1;
		blockPos[0][1] = 3;
		blockPos[1][0] = -2;
		blockPos[1][1] = 4;
		blockPos[2][0] = -1;
		blockPos[2][1] = 4;
		blockPos[3][0] = -2;
		blockPos[3][1] = 5;
		break;
		
	case 7:
	case 8:
		blockPos[0][0] = -1;
		blockPos[0][1] = 3;
		blockPos[1][0] = -2;
		blockPos[1][1] = 4;
		blockPos[2][0] = -1;
		blockPos[2][1] = 4;
		blockPos[3][0] = -1;
		blockPos[3][1] = 5;
	}

	setNextBlock(nextBlock);
}

void setNextBlock(int nextBlock)
{
	switch (nextBlock)
	{
	case 0:
		nextBlockShow[0] = FALSE;
		nextBlockShow[1] = FALSE;
		nextBlockShow[2] = TRUE;
		nextBlockShow[3] = TRUE;
		nextBlockShow[4] = TRUE;
		nextBlockShow[5] = TRUE;
		break;

	case 1:
		nextBlockShow[0] = TRUE;
		nextBlockShow[1] = FALSE;
		nextBlockShow[2] = FALSE;
		nextBlockShow[3] = TRUE;
		nextBlockShow[4] = TRUE;
		nextBlockShow[5] = TRUE;
		break;

	case 2:
		nextBlockShow[0] = FALSE;
		nextBlockShow[1] = TRUE;
		nextBlockShow[2] = TRUE;
		nextBlockShow[3] = TRUE;
		nextBlockShow[4] = TRUE;
		nextBlockShow[5] = FALSE;
		break;

	case 3:
	case 4:
		nextBlockShow[0] = TRUE;
		nextBlockShow[1] = TRUE;
		nextBlockShow[2] = FALSE;
		nextBlockShow[3] = TRUE;
		nextBlockShow[4] = TRUE;
		nextBlockShow[5] = FALSE;
		break;

	case 5:
		nextBlockShow[0] = TRUE;
		nextBlockShow[1] = TRUE;
		nextBlockShow[2] = FALSE;
		nextBlockShow[3] = FALSE;
		nextBlockShow[4] = TRUE;
		nextBlockShow[5] = TRUE;
		break;

	case 6:
		nextBlockShow[0] = TRUE;
		nextBlockShow[1] = TRUE;
		nextBlockShow[2] = TRUE;
		nextBlockShow[3] = TRUE;
		nextBlockShow[4] = FALSE;
		nextBlockShow[5] = FALSE;
		break;

	case 7:
	case 8:
		nextBlockShow[0] = TRUE;
		nextBlockShow[1] = FALSE;
		nextBlockShow[2] = TRUE;
		nextBlockShow[3] = TRUE;
		nextBlockShow[4] = TRUE;
		nextBlockShow[5] = FALSE;
	}
}

void move(HWND hwnd)
{
	BOOL flag1 = FALSE;
	for (int i = 0; i < 4; ++i)
	{
		if (blockPos[i][0] < 0)
		{
			flag1 = TRUE;
			break;
		}
	} // 是否全露检测
	if (flag1) // 如果没有完全露出
	{
		for (int i = 0; i < 4; ++i)
		{
			if (blockPos[i][0] >= 0)
			{
				blocks[blockPos[i][0]][blockPos[i][1]] = FALSE;
			}
		}
		for (int i = 0; i < 4; ++i)
		{
			++blockPos[i][0]; // 下移
		}
		for (int i = 0; i < 4; ++i)
		{
			if (blockPos[i][0] >= 0)
			{
				blocks[blockPos[i][0]][blockPos[i][1]] = TRUE;
			}
		}
	}
	else // 如果完全露出
	{
		int watch[4] = { -1, -1, -1, -1 };
		for (int i = 0; i < 4; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				if (watch[j] == -1)
				{
					watch[j] = i;
					break;
				}
				else if (blockPos[watch[j]][1] == blockPos[i][1] && blockPos[watch[j]][0] < blockPos[i][0])
				{
					watch[j] = i;
					break;
				}
				else if (blockPos[watch[j]][1] == blockPos[i][1] && blockPos[watch[j]][0] > blockPos[i][0])
				{
					break;
				}
			}
		} // 获取监听对象
		BOOL flag2 = TRUE;
		for (int i = 0; i < 4 && watch[i] != -1; ++i)
		{
			if (blockPos[watch[i]][0] == 19 || blocks[blockPos[watch[i]][0] + 1][blockPos[watch[i]][1]])
			{
				flag2 = FALSE;
				break;
			}
		} // 碰撞检测
		if (flag2) // 如果没有碰撞
		{
			for (int i = 0; i < 4; ++i)
			{
				blocks[blockPos[i][0]][blockPos[i][1]] = FALSE;
			}
			for (int i = 0; i < 4; ++i)
			{
				++blockPos[i][0]; // 下移
			}
			for (int i = 0; i < 4; ++i)
			{
				blocks[blockPos[i][0]][blockPos[i][1]] = TRUE;
			}
		}
		else // 碰撞
		{
			int lines = 0;
			for (int i = 0; i < 20; ++i)
			{
				BOOL flag3 = TRUE;
				for (int j = 0; j < 10; ++j)
				{
					if (!blocks[i][j])
					{
						flag3 = FALSE;
						break;
					}
				}
				if (flag3) // 如果可以消行
				{
					for (int j = i; j > 0; --j)
					{
						for (int k = 0; k < 10; ++k)
						{
							blocks[j][k] = blocks[j - 1][k];
						}
						for (int k = 0; k < 10; ++k)
						{
							blocks[j - 1][k] = FALSE;
						}
					}
					++lines;
				}
			}
			score += lines * lines * lines; // 累计分数
			SetTimer(hwnd, TIMER_ID, (UINT)(SPEED / (score / 120.0 + 1)), NULL); // 修改速度
			BOOL flag3 = FALSE;
			for (int i = 0; i < 10; ++i) // 检测游戏是否结束
			{
				if (blocks[0][i])
				{
					flag3 = TRUE;
					break;
				}
			}
			if (flag3) // 游戏结束
			{
				KillTimer(hwnd, TIMER_ID);
				char mess1[64], mess2[32];
				maxScore = saveScore(score);
				sprintf_s(mess1, 64, "当前分数：%d\n最高分数：%d\n开启新游戏？", score, maxScore);
				sprintf_s(mess2, 32, "%s 游戏结束", title);
				int anr = MessageBox(hwnd, mess1, mess2, MB_OKCANCEL); // 显示对话框
				if (anr == IDOK) // 重新开始
				{
					clear();
					srand((unsigned int)time(NULL));
					newBlock();
					SetTimer(hwnd, TIMER_ID, SPEED, NULL);
				}
				else	 // 退出
				{
					DeleteObject(whitePen);
					DeleteObject(blackPen);
					DeleteObject(whiteBrush);
					DeleteObject(grayBrush);
					PostQuitMessage(0);
				}
			}
			else // 暂未结束
			{
				newBlock(); // 新增方块
			}
		}
	}
}

int saveScore(int s)
{
	FILE* fp;
	fopen_s(&fp, "score.dat", "rb");
	int m = 0;
	if (fp != NULL)
	{
		fread((void*)&m, sizeof(m), 1, fp);
		fclose(fp);
	}
	if (s > m)
	{
		m = s;
	}
	fopen_s(&fp, "score.dat", "wb");
	if (fp != NULL)
	{
		fwrite((void*)&m, sizeof(m), 1, fp);
		fclose(fp);
	}
	return m;
}

void toLeft()
{
	for (int i = 0; i < 4; ++i)
	{
		if (blockPos[i][0] < 0)
		{
			return;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (blockPos[i][1] == 0)
		{
			return;
		}
	}
	int watch[4] = { -1, -1, -1, -1 };
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (watch[j] == -1)
			{
				watch[j] = i;
				break;
			}
			else if (blockPos[watch[j]][0] == blockPos[i][0] && blockPos[watch[j]][1] > blockPos[i][1])
			{
				watch[j] = i;
				break;
			}
			else if (blockPos[watch[j]][0] == blockPos[i][0] && blockPos[watch[j]][1] < blockPos[i][1])
			{
				break;
			}
		}
	}
	for (int i = 0; i < 4 && watch[i] != -1; ++i)
	{
		if (blocks[blockPos[watch[i]][0]][blockPos[watch[i]][1] - 1])
		{
			return;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		blocks[blockPos[i][0]][blockPos[i][1]] = FALSE;
	}
	for (int i = 0; i < 4; ++i)
	{
		--blockPos[i][1];
	}
	for (int i = 0; i < 4; ++i)
	{
		blocks[blockPos[i][0]][blockPos[i][1]] = TRUE;
	}
}

void toRight()
{
	for (int i = 0; i < 4; ++i)
	{
		if (blockPos[i][0] < 0)
		{
			return;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (blockPos[i][1] == 9)
		{
			return;
		}
	}
	int watch[4] = { -1, -1, -1, -1 };
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (watch[j] == -1)
			{
				watch[j] = i;
				break;
			}
			else if (blockPos[watch[j]][0] == blockPos[i][0] && blockPos[watch[j]][1] < blockPos[i][1])
			{
				watch[j] = i;
				break;
			}
			else if (blockPos[watch[j]][0] == blockPos[i][0] && blockPos[watch[j]][1] > blockPos[i][1])
			{
				break;
			}
		}
	}
	for (int i = 0; i < 4 && watch[i] != -1; ++i)
	{
		if (blocks[blockPos[watch[i]][0]][blockPos[watch[i]][1] + 1])
		{
			return;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		blocks[blockPos[i][0]][blockPos[i][1]] = FALSE;
	}
	for (int i = 0; i < 4; ++i)
	{
		++blockPos[i][1];
	}
	for (int i = 0; i < 4; ++i)
	{
		blocks[blockPos[i][0]][blockPos[i][1]] = TRUE;
	}
}

void toReSet()
{
	for (int i = 0; i < 4; ++i)
	{
		if (blockPos[i][0] < 0) // 如果没有完全露出，那么不重置
		{
			return;
		}
	}
	BOOL b1[4][4], b2[4][4];
	int rightUp[2] = { 20, -1 }, leftUp[2] = { 20, 20 };
	int pos[4][2];
	for (int i = 0; i < 4; ++i) // 清空b1 b2
	{
		for (int j = 0; j < 4; ++j)
		{
			b1[i][j] = b2[i][j] = FALSE;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (rightUp[0] > blockPos[i][0])
		{
			rightUp[0] = blockPos[i][0];
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (rightUp[1] < blockPos[i][1])
		{
			rightUp[1] = blockPos[i][1];
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (leftUp[0] > blockPos[i][0])
		{
			leftUp[0] = blockPos[i][0];
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (leftUp[1] > blockPos[i][1])
		{
			leftUp[1] = blockPos[i][1];
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		b1[blockPos[i][0] - rightUp[0]][3 - rightUp[1] + blockPos[i][1]] = TRUE;
	}
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			b2[i][j] = b1[j][3 - i];
		}
	}
	for (int i = 0, k = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (b2[i][j])
			{
				pos[k][0] = leftUp[0] + i;
				pos[k][1] = leftUp[1] + j;
				++k;
				if (k == 4)
				{
					break;
				}
			}
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		if (pos[i][0] > 19 || pos[i][1] < 0 || pos[i][1] > 9)
		{
			return;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		blocks[blockPos[i][0]][blockPos[i][1]] = FALSE;
	}
	for (int i = 0; i < 4; ++i)
	{
		if (blocks[pos[i][0]][pos[i][1]])
		{
			for (int j = 0; j < 4; ++j)
			{
				blocks[blockPos[j][0]][blockPos[j][1]] = TRUE;
			}
			return;
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			blockPos[i][j] = pos[i][j];
		}
	}
	for (int i = 0; i < 4; ++i)
	{
		blocks[blockPos[i][0]][blockPos[i][1]] = TRUE;
	}
}

void clear()
{
	for (int i = 0; i < 20; ++i)
	{
		for (int j = 0; j < 10; ++j)
		{
			blocks[i][j] = FALSE;
		}
	}
	score = 0;
}
