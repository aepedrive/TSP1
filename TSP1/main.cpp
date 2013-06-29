#include <Windows.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "gaTSP.h"
#include "defines.h"

using namespace std;

///////////////////////GLOBALS ////////////////////////////////////

LPCWSTR szApplicationName = L"Chapter4 - The TSP";
LPCWSTR szWindowClassName = L"TSPclass";

//pointer to the TSP object
CgaTSP* g_pTSP;

//-----------------------------------WinProc------------------------------------------
//
//------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//device context for our window
	HDC hdc;
	PAINTSTRUCT ps;
	//these hold the dimensions of the client window area
	static int cxClient, cyClient;
	//used to create the back buffer
	static HDC hdcBackBuffer;
	static HBITMAP hBitmap;
	static HBITMAP hOldBitmap;

	switch(msg)
	{
		case WM_CREATE:
			{
				//seed the random number generator
				srand((unsigned)time(NULL));
				//get the size of the client window
				RECT rect;
				GetClientRect(hWnd, &rect);
				cxClient = rect.right;
				cyClient = rect.bottom;

				//create a surface for us to render to(backbuffer)
				hdcBackBuffer = CreateCompatibleDC(NULL);
				HDC hdc = GetDC(hWnd);
				hBitmap = CreateCompatibleBitmap(hdc, cxClient, cyClient);
				ReleaseDC(hWnd, hdc);
				hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);

				//create our TSP class object
				//global instance of our TSP class
				g_pTSP = new CgaTSP(MUTATION_RATE, CROSSOVER_RATE, POP_SIZE, NUM_CITIES, WINDOW_WIDTH, WINDOW_HEIGHT);
			}
			break;
		case WM_KEYUP:
			{
				switch(wParam)
				{
					case VK_RETURN:
						{
							g_pTSP->Run(hWnd);
						}
						break;
					case VK_ESCAPE:
						{
							PostQuitMessage(0);
						}
						break;
					case VK_SPACE:
						{
							g_pTSP->Stop();
						}
						break;
				}
			}
			break;
		case WM_SIZE:
			{
				cxClient = LOWORD(lParam);
				cyClient = HIWORD(lParam);

				//create the city coordinates based on the area of the 
				//client window
				g_pTSP->Resize(cxClient, cyClient);
				//resize the backbuffer accordingly
				SelectObject(hdcBackBuffer, hOldBitmap);
				HDC hdc = GetDC(hWnd);
				hBitmap = CreateCompatibleBitmap(hdc, cxClient, cyClient);
				ReleaseDC(hWnd, hdc);
				hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);
			}
			break;
		case WM_PAINT:
			{
				//this is where we do all our drawing to screen. CgaTSP::Run() 
				//calls InvalidateRect & UpdateWindow each generation.
				hdc = BeginPaint(hWnd, &ps);

				//fill our backbuffer with white
				BitBlt(hdcBackBuffer, 0, 0, cxClient, cyClient, NULL, NULL, NULL, WHITENESS);

				g_pTSP->Render(hdcBackBuffer, cxClient, cyClient);

				//now blit backbuffer to front
				BitBlt(ps.hdc, 0, 0, cxClient, cyClient, hdcBackBuffer, 0, 0, SRCCOPY);
				ReleaseDC(hWnd, hdc);
				EndPaint(hWnd, &ps);
			}
			break;
		case WM_DESTROY:
			{
				SelectObject(hdcBackBuffer, hOldBitmap);
				//clean up our backbuffer objects
				DeleteDC(hdcBackBuffer);
				DeleteObject(hBitmap);
				// kill the application, this sends a WM_QUIT message 
				PostQuitMessage(0);
			}
			break;

	}

	return (DefWindowProc(hWnd, msg, wParam, lParam));
}

//-----------------------------------WinMain-----------------------------------------
//	Entry point for our windows application
//-----------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdline, int nCmdshow)
{
	WNDCLASSEX winclass;
	HWND hWnd;
	MSG msg;

	// first fill in the window class stucture
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hInstance;
	winclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = NULL;
	winclass.lpszMenuName = NULL;
	winclass.lpszClassName = szWindowClassName;
	winclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// register the window class
	if(!RegisterClassEx(&winclass))
		return 0;

	// create the window
	if(!(hWnd = CreateWindowEx(NULL, szWindowClassName, szApplicationName,
								WS_OVERLAPPEDWINDOW | WS_VISIBLE,
								GetSystemMetrics(SM_CXSCREEN) / 2 - WINDOW_WIDTH / 2,
								GetSystemMetrics(SM_CYSCREEN) / 2 - WINDOW_HEIGHT /2,
								WINDOW_WIDTH,
								WINDOW_HEIGHT,
								NULL, NULL, hInstance, NULL)))

		return 0;

	ShowWindow(hWnd, nCmdshow);
	UpdateWindow(hWnd);

	//enter the message loop
	bool bDone = false;

	while(!bDone)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				// Stop loop if it's a quit message
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		//if the user has started the run update the GA and display
		//accordingly
		if(g_pTSP->Started())
		{
			//update the gun
			g_pTSP->Epoch();
			//this will call WM_PAINT 
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
		}
	}//end while

	UnregisterClass(szWindowClassName, winclass.hInstance);

	//delete our GA instance
	if(g_pTSP)
		delete g_pTSP;

	return 0;
}