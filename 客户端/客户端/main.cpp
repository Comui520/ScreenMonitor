#include <iostream>
#include <WinSock2.h>
#include <fstream>
#include <easyx.h>
#include <string>
#pragma comment (lib, "ws2_32.lib")
#define width 1440
#define height 810
char addr[100] = "./send.jpg";
int DSKwideth = 0;
int DSKheight = 0;
IMAGE backGroundImage = { 0 };
using namespace std;
struct pack_header {
	/*
	1为图片，2为文字。
	*/
	int type;
	unsigned long long int size;
};
void getHdcandHwnd(HWND& deskHwnd, HDC& deskHdc) {
	deskHwnd = GetDesktopWindow();
	//得到桌面的HDC(绘图设备) 
	deskHdc = GetWindowDC(deskHwnd);
}
SOCKET connectTarget() {
	SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in target = { 0 };
	target.sin_family = AF_INET;
	char ip[16] = "127.0.0.1";
	cout << "please enter the target ip:";
	cin >> ip;
	target.sin_addr.s_addr = inet_addr(ip);
	char port[6] = "8080";
	cout << "please enter the target port:";
	cin >> port;
	target.sin_port = htons(atoi(port));
	if (-1 == connect(client_socket, (sockaddr*)&target, sizeof(target))) {
		cout << "connect failed" << endl;
		closesocket(client_socket);
		return -1;
	}
	return client_socket;
}
int setPackHeader(pack_header& pack) {
	ifstream fin;
	fin.open(addr, ios::ate | ios::binary);
	if (!fin.is_open()) {
		cout << "file opened failed" << endl;
		fin.close();
		return 0;
	}
	pack.type = 1;
	pack.size = fin.tellg();
	fin.close();
	return 1;
}
int main() {
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	
	//获取到桌面窗口的HWND（窗口句柄）编号
	HWND deskHwnd;
	//得到桌面的HDC(绘图设备) 
	HDC deskHdc;
	getHdcandHwnd(deskHwnd, deskHdc);
	SOCKET client_socket = connectTarget();
	//测试
	/*while (1)
	{
		Rectangle(deskHdc, 100, 100, 200, 200);
	}*/
	//获取桌面大小，储存。
	/*DSKwideth = GetSystemMetrics(SM_CXSCREEN);
	DSKheight = GetSystemMetrics(SM_CYSCREEN);*/
	//该方法在本电脑不适用，不知道为何。
	DEVMODE devMode;
	int modeNum = 0;
	
	DSKwideth = 2560;
	DSKheight = 1600;
	//DSKwideth = GetDeviceCaps(deskHdc, HORZRES);
	//DSKheight = GetDeviceCaps(deskHdc, VERTRES);
	EnumDisplaySettings(NULL, modeNum, &devMode);
	DSKwideth = devMode.dmPelsWidth;
	DSKheight = devMode.dmPelsHeight;
	backGroundImage = IMAGE(DSKwideth, DSKheight);
	//拷贝到该image类型中
	//BOOL BitBlt(
	//	HDC hdc,		//目标DC
	//	int x, int y,	//目标起始坐标
	//	int cx, int cy,	//拷贝长度
	//	HDC hdcSrc,		//源dc
	//	int x1,int y1,	//原dc拷贝起点坐标
	//	DWORD rop		//标记
	//);
	HDC imageDC = GetImageHDC(&backGroundImage);
	while (1)
	{
		BitBlt(imageDC, 0, 0, DSKwideth, DSKheight, deskHdc, 0, 0, SRCCOPY);
		saveimage(addr, &backGroundImage);
		//发送
		//1.先发送数据头部
		pack_header pack;
		if (setPackHeader(pack) == 0) {
			return 0;
		}
		if (pack.size <= 0 || pack.type != 1) {
			continue;
		}
		int s = send(client_socket, (const char*)&pack, sizeof(pack), 0);
		if (s <= 0) {
			break;
		}
		//2.发送图片数据
		ifstream fin;
		fin.open(addr, ios::in | ios::binary);
		if (!fin.is_open()) {
			cout << "file opened failed" << endl;
			system("pause");
			return -1;
		}
		char buffer[1024];
		while (fin.read(buffer, sizeof(buffer)) || fin.gcount() > 0) {
			int sendbyte = fin.gcount();
			s = send(client_socket, buffer, sendbyte, 0);
			if (s < 0) {
				closesocket(client_socket);
				cout << "send failed" << endl;
				system("pause");
				return 0;
			}
		}
		fin.close();
	}
	closesocket(client_socket);
	return 0;
}