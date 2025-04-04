#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define _CRT_SECURE_NO_DEPRECATE
#include <Windows.h>
#include <tchar.h> 
#include <xstring>
#include <string>
#include <wchar.h>
#include "resource.h"
#include <strsafe.h>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <commdlg.h>
#include <iostream>
#include <cstdlib>
#include <filesystem>
#include <atlstr.h>

#define ArrSz(x)    (sizeof(x)/sizeof(x[0]))

int CheckType(std::vector<std::string>& vec, int& row, int pos, int& valuedType);
int FindVar(std::vector<std::string>& vec, const std::string& var);
int SkipComments(std::vector<std::string>& vec, int& row, int pos);

char Output[255];
char varsBuffer[255];
char codeBuffer[255];

static TCHAR nameCode[256] = _T("");
static TCHAR nameVars[256] = _T("");
static TCHAR outputVars[256] = _T("Result.txt");
int mainnCmdShow;

#define BUFFER_SIZE 200
INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DialogHistory(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

HINSTANCE Inst;
HWND mainHdlg;

TCHAR dir[255];

int mainchecks() {
	HANDLE hFile;
	char tab2[1024];
	std::vector<std::string> myLines;//вектор строк
	std::string line;//вспомогательная строка
	std::string var;//строка содержащая переменные
	std::string types[] = { "char", "int", "bool", "float", "double", "void", "DWORD", "BYTE", "short","long", "unsigned", "signed", "struct", "enum", "union" };
	DWORD dwBytesWrite = 0;
	WCHAR WriteBuffer[BUFFER_SIZE] = { 0 };
	std::ifstream infile(nameCode);//подключаем файл с кодом
	std::ifstream qfile(nameVars);

	int opencomment = 0, commentpos = -1, lastcommentpos = -1, usetypedef = -1, row = 0, commentrow = 0;

	while (std::getline(infile, line))//читаем входной файл
	{
		if (line.size()) {
			if (line.size() > 100)//кол-во символов в одной строке не должно превышать 100
			{
				return -1;
			}
			int pos = 0;
			usetypedef = line.find("typedef", 0, 7);
			if (usetypedef >= 0)
			{
				if ((usetypedef + 7 < line.size()) && line.at(usetypedef + 7) != ' ')//если в строке присутствует слово typedef
					usetypedef = -1;
			}
			if (line.size() >= 2)//проверка на наличие typedef внутри комментариев
			{
				if (!opencomment) //если не установлен признак открытого коментария
					pos = line.find("/*", 0, 2);

				if (pos >= 0)
					while (pos < line.size() - 1)
					{
						if (line.at(pos) == '/' && line.at(pos + 1) == '*' || line.at(pos) == '*' && line.at(pos + 1) == '/')
						{
							if (opencomment && (commentrow < row || commentrow == row && lastcommentpos < usetypedef))
								usetypedef = -1;
							opencomment ^= 1;
							commentrow = row;
							lastcommentpos = pos;
						}
						pos++;
					}
			}
			if (usetypedef >= 0)
			{
				if (opencomment && (commentrow < row || commentrow == row && lastcommentpos < usetypedef))//если открыт комментарий и typedef внутри
					usetypedef = -1;
				int shortpos = line.find("//", 0, 2);//проверка на короткий коментарий			
				if (usetypedef >= 0 && shortpos >= 0 && shortpos < usetypedef)//если короткий коментарий перед typedef 
					usetypedef = -1;
				if (usetypedef >= 0)//если typedef обнаружен в строке
				{
					return -1;
				}
			}
			row++;
			myLines.push_back(line);//добавляем строки в вектор строк
		}
	}
	if (!myLines.size())//проверка на наличие кода в файле с программой
	{
		return -1;
	}
	std::getline(qfile, line);
	if (!line.size())//файл с переменными не должен быть пустым
	{
		return -1;
	}
	int pos = 0;
	int varcount = 0;
	if (line.find('\t') != std::string::npos || line.find('.') != std::string::npos || line.find(',') != std::string::npos || line.find('/') != std::string::npos)//проверка на недопустимые разделители в файле с переменными
	{
		return -1;
	}
	do
	{
		if (line.at(pos) != ' ') {
			var += line.at(pos);//добавляем символ из текущей позиции в var
			if (!isalnum(line.at(pos)) && line.at(pos) != '_')//проверка на недопустимые символы в именах переменных
			{
				return -1;
			}
			if (var.size() == 1 && isdigit(var.at(0)))//переменная не должна начинаться с цифры
			{
				return -1;
			}
			int res = 0;
			for (int i = 0; i < ArrSz(types); i++)//переменная не должна начинаться с цифры
				if (var.compare(types[i]) == 0) {
					return -1;
				}
		}
		if (var.size() && (pos == line.size() - 1 || pos < line.size() - 1 && line.at(pos) == ' '))//если есть переменные и позиция на последнем символе или позиция не на последнем символе и это пробел
		{
			varcount++;
			var.clear();
		}
		pos++;
	} while (pos < line.size());

	hFile = CreateFile(
		outputVars,                // file to open
		GENERIC_WRITE,          // open for reading
		FILE_SHARE_READ,    // share for reading
		NULL,                  // default security
		CREATE_ALWAYS,         // existing file only
		FILE_ATTRIBUTE_NORMAL, // normal file
		NULL
	);

	pos = 0;
	do
	{
		if (line.at(pos) != ' ')
			var += line.at(pos);//добавляем символ из текущей позиции в var
		if (var.size() && (pos == line.size() - 1 || pos < line.size() - 1 && line.at(pos) == ' '))//если есть переменные и позиция на последнем символе или позиция не на последнем символе и это пробел
		{
			if (FindVar(myLines, var)) {//если переменная найдена
				var = var + "\n";
				strcpy_s(tab2, var.c_str());
				WriteFile(hFile, tab2, strlen(tab2), &dwBytesWrite, NULL);
			}
			var.clear();
		}
		pos++;
	} while (pos < line.size());

	std::ifstream outfile;
	outfile.open("Result.txt");
	std::string str1;
	std::string res = "Found vars:";
	
	if (outfile.is_open())
	{
		while (outfile) {
			std::getline(outfile, str1);
			res = res + " " + str1;
		}
	}
	
	if (!res.empty()) {
		MessageBoxA(mainHdlg, res.c_str(), "Found vars", MB_ICONQUESTION | MB_OK);
	}
}


void addHistoryFile(TCHAR *pathfile) {
	HANDLE hFile=0;
	static OPENFILENAME file;
	DWORD dwBytesWrite = 0;
	static TCHAR hustoryFiles[256] = _T("History.txt");
	wcscpy(hustoryFiles, dir);
	char tmp1[255];
	wcscat(hustoryFiles, _T("\\History.txt"));
	ZeroMemory(codeBuffer, sizeof(codeBuffer));
	ZeroMemory(&file, sizeof(file));

		hFile = CreateFile(
			hustoryFiles,                // file to open
			GENERIC_WRITE|GENERIC_READ,          // open for reading
			0,    // share for reading
			NULL,                  // default security
			OPEN_ALWAYS,         // existing file only
			FILE_ATTRIBUTE_NORMAL, // normal file
			NULL
		);
	TCHAR tmp[255];
	wcscpy(tmp, pathfile);
	SetFilePointer(hFile, 0, NULL, FILE_END);
	wcscat(tmp, L"\r\n");
	wcstombs(tmp1, tmp, wcslen(tmp) + 1);
	WriteFile(hFile, tmp1, strlen(tmp1) *sizeof(char), &dwBytesWrite, NULL);
	CloseHandle(hFile);
}

int displayHistory(HWND hDlg) 
{
	TCHAR tmp1[255];
	HMENU rm = GetSubMenu(GetMenu(hDlg), 0);
	std::ifstream outfile;
	wcscpy(tmp1, dir);
	wcscat(tmp1, _T("\\History.txt"));
	outfile.open(tmp1);

	int i = 0;
	std::string str;
	if (outfile.is_open())
	{
		while (outfile) {
			std::getline(outfile, str);
			if (str.length()==0)
			{
				break;
			}
			_tcscpy_s(tmp1, CA2T(str.c_str()));
			AppendMenu(rm, MF_STRING, 5000 + i, tmp1);
			i++;
		}
	}
	return 0;
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR args, int nCmdShow) {

	MSG msg;
	BOOL ret;
	Inst = hInst;
	mainnCmdShow = nCmdShow;
	GetCurrentDirectory(255, dir);
	mainHdlg = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
	
	ShowWindow(mainHdlg, mainnCmdShow);

	while ((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if (ret == -1)
			return -1;

		if (!IsDialogMessage(mainHdlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	/*
	HWND hWnd;
	MSG msg;
	Inst = hInst;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 8);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	wcex.lpszClassName = _T("MainFrame");
	wcex.hIconSm = nullptr;

	if (!RegisterClassExW(&wcex)) return 0;


	hWnd = CreateWindow(_T("MainFrame"), _T("Echo-app"), WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, HWND_DESKTOP, nullptr, hInst, nullptr);

	ShowWindow(hWnd, ncmdshow);

	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}*/
	return 0;
}

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static OPENFILENAME file;
	static std::vector<std::string> v;

	HANDLE hFile;
	DWORD dwBytesRead = 0;
	WCHAR ReadBuffer[BUFFER_SIZE] = { 0 };

	switch (uMsg)
	{
	case WM_INITDIALOG: {
		//HMENU rm = GetSubMenu(GetMenu(hDlg), 0);
		displayHistory(hDlg);
		//AppendMenu(rm, MF_STRING, 5000 + i, " ");
		return 0;
	}
	case WM_CLOSE:
		if (MessageBox(hDlg, TEXT("Close the window?"), TEXT("Close"), MB_ICONQUESTION | MB_YESNO) == IDYES)
		{
			//CloseHandle(hFile);
			DestroyWindow(hDlg);
		}
		return TRUE;

	case WM_DESTROY:
		//CloseHandle(hFile);
		PostQuitMessage(0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case ID_FIND:
			mainchecks();
			break;

		case ID_CODE_FILE:
			ZeroMemory(codeBuffer, sizeof(codeBuffer));
			ZeroMemory(&file, sizeof(file));

			file.lStructSize = sizeof(OPENFILENAME);
			file.hInstance = Inst;
			file.lpstrFilter = _T(".c\0 *.c\0.cpp\0 *.cpp");
			file.lpstrFile = nameCode;
			file.nMaxFile = 256;
			file.lpstrInitialDir = _T(".\\");
			file.lpstrDefExt = _T("txt");

			if (!GetOpenFileName(&file)) return 1;
			hFile = CreateFile(
				nameCode,
				GENERIC_READ,          
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);		
			char c_szText[1024];
			
			addHistoryFile(nameCode);
			if (ReadFile(hFile, codeBuffer, (BUFFER_SIZE - 1), &dwBytesRead, NULL))
			{
				codeBuffer[dwBytesRead] = 0;
				SetDlgItemTextA(hDlg, IDC_EDIT1, codeBuffer);
			}
			CloseHandle(hFile);
			break;

		case ID_VARS_FILE:

			ZeroMemory(varsBuffer, sizeof(varsBuffer));
			ZeroMemory(&file, sizeof(file));

			file.lStructSize = sizeof(OPENFILENAME);
			file.hInstance = Inst;
			file.lpstrFilter = _T(".txt\0 *.txt");
			file.lpstrFile = nameVars;
			file.nMaxFile = 256;
			file.lpstrInitialDir = _T(".\\");
			file.lpstrDefExt = _T("txt");

			if (!GetOpenFileName(&file)) return 1;
			hFile = CreateFile(
				nameVars,                // file to open
				GENERIC_READ,          // open for reading
				FILE_SHARE_READ,    // share for reading
				NULL,                  // default security
				OPEN_EXISTING,         // existing file only
				FILE_ATTRIBUTE_NORMAL, // normal file
				NULL
			);
			char c_sText[1024];
			//wcstombs(c_sText, nameVars, wcslen(nameVars) + 1);

			addHistoryFile(nameVars);
			if (ReadFile(hFile, varsBuffer, (BUFFER_SIZE - 1), &dwBytesRead, NULL))
			{				
				varsBuffer[dwBytesRead] = 0;
				SetDlgItemTextA(hDlg, IDC_EDIT2, varsBuffer);
			}
			CloseHandle(hFile);
			break;

		case ID_FILE_EXIT:
			if (MessageBox(hDlg, TEXT("Close the window?"), TEXT("Close"), MB_ICONQUESTION | MB_YESNO) == IDYES)
			{
				DestroyWindow(hDlg);
			}
		}
	}
	//CloseHandle(hFile);
	return FALSE;
}




/*
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	
	PAINTSTRUCT ps;
	HDC hdc;
	int wmId;
	static TCHAR name[256] = _T("");
	static OPENFILENAME file;
	std::ifstream in;
	std::ofstream out;
	static std::vector<std::string> v;
	std::vector<std::string>::iterator it;
	std::string st;
	int y, k;
	RECT rc = { 0 };
	static int nFrame = 0;
	TCHAR szFrame[64] = { 0 };
	static int n, length, sx, sy, cx, iVscrollPos, iHscrollPos, COUNT, MAX_WIDTH;
	static SIZE size = { 8, 16 };
	int nFrameLen = 0;
	COLORREF clref = 0;
	switch (message)
	{
	case WM_CREATE:
		file.lStructSize = sizeof(OPENFILENAME);
		file.hInstance = Inst;
		file.lpstrFilter = _T("Text\0 *.txt\0Все файлы\0 *.*");
		file.lpstrFile = name;
		file.nMaxFile = 256;
		file.lpstrInitialDir = _T(".\\");
		file.lpstrDefExt = _T("txt");
		break;

	case WM_SIZE:
		sx = LOWORD(lParam);
		sy = HIWORD(lParam);
		k = n - sy / size.cy;
		if (k > 0) COUNT = k; else COUNT = iVscrollPos = 0;
		SetScrollRange(hWnd, SB_VERT, 0, COUNT, FALSE);
		SetScrollPos(hWnd, SB_VERT, iVscrollPos, TRUE);
		k = length - sx / size.cx;
		if (k > 0) MAX_WIDTH = k; else MAX_WIDTH = iHscrollPos = 0;
		SetScrollRange(hWnd, SB_HORZ, 0, MAX_WIDTH, FALSE);
		SetScrollPos(hWnd, SB_HORZ, iHscrollPos, TRUE);
		break;

	case WM_VSCROLL:
		switch (LOWORD(wParam)) {
		case SB_LINEUP: iVscrollPos--; 
			break;
		case SB_LINEDOWN: iVscrollPos++; 
			break;
		case SB_PAGEUP: iVscrollPos -= sy / size.cy; 
			break;
		case SB_PAGEDOWN: iVscrollPos += sy / size.cy; 
			break;
		case SB_THUMBPOSITION: iVscrollPos = HIWORD(wParam); 
			break;
		}
		iVscrollPos = max(0, min(iVscrollPos, COUNT));
		if (iVscrollPos != GetScrollPos(hWnd, SB_VERT)) {
			SetScrollPos(hWnd, SB_VERT, iVscrollPos, TRUE);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;

	case WM_HSCROLL:
		switch (LOWORD(wParam)) {
		case SB_LINEUP: iHscrollPos--; 
			break;
		case SB_LINEDOWN: iHscrollPos++; 
			break;
		case SB_PAGEUP: iHscrollPos -= 8; 
			break;
		case SB_PAGEDOWN: iHscrollPos += 8; 
			break;
		case SB_THUMBPOSITION: iHscrollPos = HIWORD(wParam); 
			break;
		}
		iHscrollPos = max(0, min(iHscrollPos, MAX_WIDTH));
		if (iHscrollPos != GetScrollPos(hWnd, SB_HORZ)) {
			SetScrollPos(hWnd, SB_HORZ, iHscrollPos, TRUE);
			InvalidateRect(hWnd, NULL, TRUE);
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case ID_FILE_NEW:
			if (!v.empty()) std::vector<std::string>().swap(v);
			n = length = 0;
			SendMessage(hWnd, WM_SIZE, 0, sy << 16 | sx);
			InvalidateRect(hWnd, NULL, TRUE);
			break;

		case ID_FILE_OPEN:
			file.lpstrTitle = _T("Открыть файл для чтения");
			file.Flags = OFN_HIDEREADONLY;
			if (!GetOpenFileName(&file)) return 1;
			in.open(name);
			while (getline(in, st)) { 
				if (length < st.length()) length = st.length();
				v.push_back(st); 
			}
			in.close();
			n = v.size();
			SendMessage(hWnd, WM_SIZE, 0, sy << 16 | sx);
			InvalidateRect(hWnd, NULL, 1);
			break;

		case ID_FILE_SAVE:
			file.lpstrTitle = _T("Открыть файл для записи");
			file.Flags = OFN_NOTESTFILECREATE;
			if (!GetSaveFileName(&file)) return 1;
			out.open(name);
			for (it = v.begin(); it != v.end(); ++it) out << *it << '\n';
			out.close();
			break;

		case ID_FILE_OPTIONS:
			//DialogBox(Inst, MAKEINTRESOURCE(DIALOG1), hWnd, OptionsProg);
			InvalidateRect(hWnd, NULL, TRUE);
			break;
			
		case ID_FILE_EXIT:
			DestroyWindow(hWnd);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		for (y = 0, it = v.begin() + iVscrollPos; it != v.end() && y < sy; ++it, y += size.cy)
			if (iHscrollPos < it->length()) {
				TabbedTextOutA(hdc, 0, y, it->data() + iHscrollPos, it->length() - iHscrollPos, 0, NULL, 0);
			}
			
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY: PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	
	return 0;
}
*/
/*
INT_PTR CALLBACK OptionsProg(HWND hDlg, UINT uInt, WPARAM wParam, LPARAM lParam) {
	switch (uInt)
	{
	case WM_INITDIALOG:
		if (iColor == IDC_RADIO1) {	
			CheckDlgButton(hDlg, IDC_RADIO1, BST_CHECKED);
		}
		else if (iColor == IDC_RADIO2) {
			CheckDlgButton(hDlg, IDC_RADIO2, BST_CHECKED);
		}
		else if (iColor == IDC_RADIO3) {
			CheckDlgButton(hDlg, IDC_RADIO3, BST_CHECKED);
		}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			GetDlgItemText(hDlg, IDC_TXT, czUserText, 1024);
			if (IsDlgButtonChecked(hDlg, IDC_RADIO1)) {
				iColor = IDC_RADIO1;	
			}
			else if (IsDlgButtonChecked(hDlg, IDC_RADIO2)) {
				iColor = IDC_RADIO2;
			}
			else if (IsDlgButtonChecked(hDlg, IDC_RADIO3)) {
				iColor = IDC_RADIO3;
			}
			DialogBox(Inst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, OptionsProg2);
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		default:
			break;
		}
	default:
		break;
	}
	return FALSE;
}


INT_PTR CALLBACK OptionsProg2(HWND hDlg, UINT uInt, WPARAM wParam, LPARAM lParam) {
	HWND hw;
	HDC hdc;
	PAINTSTRUCT ps;
	COLORREF clref = 0;
	switch (uInt)
	{
	case WM_INITDIALOG:
		hw = GetDlgItem(hDlg, 2000);
		if (iColor == IDC_RADIO1) {
			SetWindowText(hw, red);
		}
		else if (iColor == IDC_RADIO2) {
			SetWindowText(hw, black);
		}
		else if (iColor == IDC_RADIO3) {
			SetWindowText(hw, green);
		}
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hDlg, 0);
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		default:
			break;
		}
	case WM_PAINT:
		hdc = BeginPaint(hDlg, &ps);
		clref = GetTextColor(hdc);
		
		EndPaint(hDlg, &ps);
	default:
		break;
	}
	return FALSE;
}
*/
