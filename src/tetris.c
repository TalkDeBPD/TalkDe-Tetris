// ����˹�����C���԰汾

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

// ȫ�ֱ���
const char title[] = "����˹����";
BOOL blocks[20][10];
int blockPos[4][2];
BOOL nextBlockShow[6];
int score;
int maxScore;
int nextBlock; // ��һ������ĺ���
HPEN whitePen, blackPen;
HBRUSH whiteBrush, grayBrush;
HFONT font;

// ��������
LRESULT __stdcall WndProc(HWND, UINT, WPARAM, LPARAM); // ������Ӧ
void paint(HWND); // ���ƣ�BeginPaint()��
void rePaint(HWND); // ���ƣ�GetDC()��
void newBlock(); // ��������
void setNextBlock(int); // ����nextBlockShow
void move(HWND); // �ƶ�������
int saveScore(int); // ��ȡ��������߼�¼
void toLeft(); // ����
void toRight(); // ����
void toReSet(); // ��ת
void clear(); // ����

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
		MessageBox(NULL, "�������ɴ���", title, MB_OK | MB_ICONERROR);
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
		MessageBox(NULL, "�������ɴ���", title, MB_OK | MB_ICONERROR);
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
		srand((unsigned int)time(NULL)); // ��ʼ�������������
		nextBlock = (rand() / 93) % 9; // ��ʼ����һ������
		setNextBlock(nextBlock);
		whitePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
		blackPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
		whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
		grayBrush = CreateSolidBrush(RGB(127, 127, 127));
		// ���ʡ���ˢ��ʼ��
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
			"����"
		);
		maxScore = saveScore(0); // ��ȡ��߼�¼
		newBlock(); // ���ɵ�һ������
		SetTimer(hwnd, TIMER_ID, SPEED, NULL); // ��ʼ����ʱ��
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
	Rectangle(hdc, 0, 0, 420, 515); // ˢ��ɫ
	SelectObject(hdc, blackPen);
	MoveToEx(hdc, 255, 0, NULL);
	LineTo(hdc, 255, 505); // ����ƽ����
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
			Rectangle(hdc, 5 + 25 * j, 5 + 25 * i, 25 * (j + 1), 25 * (i + 1)); // ���Ʒ���
		}
	}

	char b[16];
	TextOut(hdc, 260, 30, "����", (int)strlen("����"));
	sprintf_s(b, 16, "%06d", score);
	TextOut(hdc, 260, 30 + tm.tmHeight, b, 6);
	TextOut(hdc, 260, 30 + 2 * tm.tmHeight, "��߷�", (int)strlen("��߷�"));
	sprintf_s(b, 16, "%06d", maxScore);
	TextOut(hdc, 260, 30 + 3 * tm.tmHeight, b, 6); // ����������Ϣ
	TextOut(hdc, 260, 30 + 5 * tm.tmHeight, "TalkDe��Ʒ��", (int)strlen("TalkDe��Ʒ��"));
	TextOut(hdc, 260, 30 + 6 * tm.tmHeight, "������Ʒ��", (int)strlen("������Ʒ��"));
	TextOut(hdc, 260, 30 + 8 * tm.tmHeight, "�������ʱ�䣺", (int)strlen("�������ʱ�䣺"));
	TextOut(hdc, 260, 30 + 9 * tm.tmHeight, "2021-07-21", (int)strlen("2021-07-21"));
	TextOut(hdc, 260, 30 + 11 * tm.tmHeight, "ʹ�÷�����", (int)strlen("ʹ�÷���"));
	TextOut(hdc, 260, 30 + 12 * tm.tmHeight, "�á�WASD���ļ����ƣ�", (int)strlen("�á�WASD���ļ����ƣ�"));
	TextOut(hdc, 260, 30 + 13 * tm.tmHeight, "��Ҫ��Ӣ������ģʽ��", (int)strlen("��Ҫ��Ӣ������ģʽ��"));
	TextOut(hdc, 260, 30 + 14 * tm.tmHeight, "���ƣ���Ȼ��Ҳ����ʹ", (int)strlen("���ơ���Ȼ��Ҳ����ʹ"));
	TextOut(hdc, 260, 30 + 15 * tm.tmHeight, "�÷�������ơ�", (int)strlen("�÷�������ơ�"));

	// ������һ������
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
		Rectangle(hdc, 285 + 25 * i, 305, 305 + 25 * i, 325); // ���Ʒ���
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
		Rectangle(hdc, 260 + 25 * i, 330, 280 + 25 * i, 350); // ���Ʒ���
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

	// ������һ������
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
		Rectangle(hdc, 285 + 25 * i, 305, 305 + 25 * i, 325); // ���Ʒ���
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
		Rectangle(hdc, 260 + 25 * i, 330, 280 + 25 * i, 350); // ���Ʒ���
	}
	
	ReleaseDC(hwnd, hdc);
}

void newBlock()
{
	int thisBlock = nextBlock;
	nextBlock = (rand() / 93) % 9;
	if (nextBlock == thisBlock) // ����ظ�������
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
	} // �Ƿ�ȫ¶���
	if (flag1) // ���û����ȫ¶��
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
			++blockPos[i][0]; // ����
		}
		for (int i = 0; i < 4; ++i)
		{
			if (blockPos[i][0] >= 0)
			{
				blocks[blockPos[i][0]][blockPos[i][1]] = TRUE;
			}
		}
	}
	else // �����ȫ¶��
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
		} // ��ȡ��������
		BOOL flag2 = TRUE;
		for (int i = 0; i < 4 && watch[i] != -1; ++i)
		{
			if (blockPos[watch[i]][0] == 19 || blocks[blockPos[watch[i]][0] + 1][blockPos[watch[i]][1]])
			{
				flag2 = FALSE;
				break;
			}
		} // ��ײ���
		if (flag2) // ���û����ײ
		{
			for (int i = 0; i < 4; ++i)
			{
				blocks[blockPos[i][0]][blockPos[i][1]] = FALSE;
			}
			for (int i = 0; i < 4; ++i)
			{
				++blockPos[i][0]; // ����
			}
			for (int i = 0; i < 4; ++i)
			{
				blocks[blockPos[i][0]][blockPos[i][1]] = TRUE;
			}
		}
		else // ��ײ
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
				if (flag3) // �����������
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
			score += lines * lines * lines; // �ۼƷ���
			SetTimer(hwnd, TIMER_ID, (UINT)(SPEED / (score / 120.0 + 1)), NULL); // �޸��ٶ�
			BOOL flag3 = FALSE;
			for (int i = 0; i < 10; ++i) // �����Ϸ�Ƿ����
			{
				if (blocks[0][i])
				{
					flag3 = TRUE;
					break;
				}
			}
			if (flag3) // ��Ϸ����
			{
				KillTimer(hwnd, TIMER_ID);
				char mess1[64], mess2[32];
				maxScore = saveScore(score);
				sprintf_s(mess1, 64, "��ǰ������%d\n��߷�����%d\n��������Ϸ��", score, maxScore);
				sprintf_s(mess2, 32, "%s ��Ϸ����", title);
				int anr = MessageBox(hwnd, mess1, mess2, MB_OKCANCEL); // ��ʾ�Ի���
				if (anr == IDOK) // ���¿�ʼ
				{
					clear();
					srand((unsigned int)time(NULL));
					newBlock();
					SetTimer(hwnd, TIMER_ID, SPEED, NULL);
				}
				else	 // �˳�
				{
					DeleteObject(whitePen);
					DeleteObject(blackPen);
					DeleteObject(whiteBrush);
					DeleteObject(grayBrush);
					PostQuitMessage(0);
				}
			}
			else // ��δ����
			{
				newBlock(); // ��������
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
		if (blockPos[i][0] < 0) // ���û����ȫ¶������ô������
		{
			return;
		}
	}
	BOOL b1[4][4], b2[4][4];
	int rightUp[2] = { 20, -1 }, leftUp[2] = { 20, 20 };
	int pos[4][2];
	for (int i = 0; i < 4; ++i) // ���b1 b2
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
