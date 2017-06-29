#include <windows.h>
#include <map>
#include <list>
#include <iostream>
#include <typeinfo>
#include <thread>
using namespace std;

//現在のカーソルの位置からの移動距離 [ x, y ]
struct move_distance {
   int x;
   int y;
};

HHOOK hHook;
map<DWORD,struct move_distance> move_distance;
bool was_clicked = false;

bool is_cursor_key(DWORD *check_key){
  for(auto itr = move_distance.begin(); itr != move_distance.end(); itr++)
    if (*check_key == itr -> first) return true;

  return false;
}

bool is_click_key(){
  if (GetAsyncKeyState(VK_SPACE) & 0x8000) return true;
  return false;
}

bool cursor_move(struct move_distance move_distance){
  POINT cursor_point;
  GetCursorPos(&cursor_point);

  int to_point_x = cursor_point.x + move_distance.x;
  int to_point_y = cursor_point.y + move_distance.y;
  SetCursorPos(to_point_x,to_point_y);
}

void check_and_click(){
  if(is_click_key() && was_clicked){
    return;
  }else if(is_click_key()){
    mouse_event(MOUSEEVENTF_LEFTDOWN,0,0,0,0);
    was_clicked = true;
  }else if(was_clicked){
    mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
    was_clicked = false;
  }
}

LRESULT CALLBACK KeyHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
		KBDLLHOOKSTRUCT*pKB = (KBDLLHOOKSTRUCT*)lParam;
    if(is_cursor_key(&pKB->vkCode)){
      cursor_move(move_distance[pKB->vkCode]);
      return -1;
    }
  }
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	static HWND hEdit;
	switch (msg) {
	case WM_CREATE:
		hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyHookProc, NULL, 0);
		hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEdit, 1, 1, 1, 1, false);
		break;
	case WM_DESTROY:
		UnhookWindowsHookEx(hHook);
		PostQuitMessage(0);
    mouse_event(MOUSEEVENTF_LEFTUP,0,0,0,0);
    cout << "shut down" << endl;
    break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow) {
	TCHAR szClassName[] = TEXT("Window");
	MSG msg;

	WNDCLASS wndclass = {CS_HREDRAW | CS_VREDRAW,
		                   WndProc,
		                   0,
		                   0,
		                   hInstance,
		                   0,
		                   LoadCursor(0,IDC_ARROW),
		                   (HBRUSH)(COLOR_WINDOW + 1),
		                   0,
		                   szClassName
	                    };

	RegisterClass(&wndclass);

	HWND hWnd = CreateWindow(szClassName,
		                       TEXT("Global hook test"),
		           						 WS_OVERLAPPEDWINDOW,
		                       CW_USEDEFAULT,
		                       0,
		                       CW_USEDEFAULT,
		                       0,
		                       0,
		                       0,
		                       hInstance,
		                       0);

	ShowWindow(hWnd, SW_SHOWDEFAULT);

	UpdateWindow(hWnd);

  int move_scale = 10;
  //int配列は、現在のカーソルの位置からの移動距離 [ x, y ]
  move_distance['W'] = (struct move_distance) {0,-move_scale};
  move_distance['A'] = (struct move_distance) {-move_scale,0};
  move_distance['S'] = (struct move_distance) {0,move_scale};
  move_distance['D'] = (struct move_distance) {move_scale,0};
  while(true){
    if(!GetMessage(&msg, 0, 0, 0)){
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    check_and_click();
    Sleep(2);
  }

	return (int)msg.wParam;
}
