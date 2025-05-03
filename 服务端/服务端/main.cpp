#include <iostream>
#include <WinSock2.h>
#include <easyx.h>
#include <fstream>
#include <string>
#define width 1440
#define height 810
#pragma comment (lib, "ws2_32.lib")
using namespace std;
char addr[100] = "./receive.jpg";
struct pack_header {//包头
	/*
	1为图片，2为文字。
	*/
	int type;
	unsigned long long int size;
};
int main() {
	initgraph(width, height);
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == INVALID_SOCKET) {
		cout << "create socket failed" << endl;
		system("pause");

		return -1;
	}
	sockaddr_in local = { 0 };
	local.sin_family = AF_INET;
	local.sin_port = htons(8080);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	if (-1 == bind(listen_socket, (sockaddr*)&local, sizeof(local))) {
		cout << "bind socket failed" << endl;
		system("pause");

		return -1;
	}
	if (-1 == listen(listen_socket, 10)) {
		cout << "listen socket failed" << endl;
		system("pause");

		return -1;
	}
	int sizelen = sizeof(sockaddr_in);
	sockaddr_in connector = { 0 };
	while (1) {

		SOCKET client_socket = accept(listen_socket, (sockaddr*)&connector, &sizelen);
		if (INVALID_SOCKET == client_socket) {
			continue;
		}
		cout << "new connector:\n" << "ip:" << inet_ntoa(connector.sin_addr) << " port:" << htons(connector.sin_port) << endl;
		while(1) {
			pack_header header;
			int byteReceive = recv(client_socket, (char*)&header, sizeof(header), 0);
			if (byteReceive <= 0) {
				cout << "接收失败" << endl;
				closesocket(client_socket);
				closesocket(listen_socket);
				system("pause");
				return 0;
			}
			if (header.type == 0) {//文字
				continue;
			}
			else if (header.type == 1) {//图片
				ofstream fout;
				fout.open(addr, ios::out | ios::binary);
				int size = header.size;
				char buffer[4096];
				while (size > 0) {
					byteReceive = recv(client_socket, buffer, sizeof(buffer), 0);
					if (byteReceive <= 0) {
						cout << "接收失败" << endl;
						system("pause");
						fout.close();
						return 0;
					}
					fout.write(buffer, byteReceive);
					size -= byteReceive;
				}
				fout.close();
			}
			else {
				system("pause");
				continue;
			}
			IMAGE img;
			loadimage(&img, addr, width, height);
			BeginBatchDraw();
			putimage(0, 0, &img);
			FlushBatchDraw();
		}
	}
	closesocket(listen_socket);
	return 0;
}