#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <thread>
#pragma comment (lib, "ws2_32.lib")

using namespace std;

void recv_socket(SOCKET sock);

int main()
{
	string ipAddress = "127.0.0.1";
	int port = 54000;				

	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		cerr << "Can`t start Winsock, Err #" << wsResult << endl;
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET)
	{
		cerr << "Can`t create socket, Err #" << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		cerr << "Can`t connect to server, Err #" << WSAGetLastError() << endl;
		closesocket(sock);
		WSACleanup();
		return 1;
	}

	thread recv_thread(recv_socket, sock);

	string userInput;
	do
	{
		getline(cin, userInput);
		if (userInput.size() > 0)
		{
			send(sock, userInput.c_str(), userInput.size() + 1, 0);
		}
	} while (userInput.size() > 0);

	recv_thread.join();
	closesocket(sock);
	WSACleanup();
	return 0;
}

void recv_socket(SOCKET sock)
{
	char buf[4096];
	while (1)
	{
		ZeroMemory(buf, 4096);
		int bytesReceived = recv(sock, buf, 4096, 0);
		if (bytesReceived > 0)
		{
			cout << string(buf, 0, bytesReceived) << endl;
		}
	}
}