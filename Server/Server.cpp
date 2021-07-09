#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#pragma comment(lib,"ws2_32.lib")
using namespace std;
void BroadcastServer(fd_set Master, SOCKET Listening);
int DataClient[10]; //menyimpan data socket yang terhubung
int countMAXClient = 0; // jumlah data socket yang telah terhubung
int main()
{
	//menginisiasi winsock
	WSADATA wsData;
	int wsOk = WSAStartup(MAKEWORD(2, 2), &wsData);
	if (wsOk != 0)
	{
		cout << "Can't create winsock";
		return 1;
	}
	//menginisiasi socket dengan menggunakan protokol TCP
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can`t create a socket!" << endl;
		return 1;
	}
	
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(listening, (sockaddr*)&hint, sizeof(hint));
	
	listen(listening, SOMAXCONN);

	fd_set master;
	FD_ZERO(&master);

	FD_SET(listening, &master);

	thread ServerBroadcast(BroadcastServer, master, listening);
	ServerBroadcast.join();

	return 0;
}

void BroadcastServer(fd_set Master,SOCKET Listening)
{
	int a = 1;
	while (a)
	{
		fd_set copy = Master;
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);
		string tempChat[1000];
		int count = 0;
		for (int i = 0; i < socketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == Listening)
			{
				SOCKET client = accept(Listening, nullptr, nullptr);

				FD_SET(client, &Master);

				string welcomeMsg = "Welcome to the Chat Server!\r\n";
				send(client, welcomeMsg.c_str(), welcomeMsg.size() + 1, 0);
				for (int k = 0; k < 10; k++)
				{
					if (DataClient[k] == client)
					{
						break;
					}
					else
					{
						DataClient[countMAXClient] = client;
						ostringstream ss;
						ss << "SOCKET #" << DataClient[countMAXClient] << " a.k.a Client " << countMAXClient + 1 << " connect to chat server.\r\n";
						countMAXClient++;
						string strOut = ss.str();
						for (int j = 0; j < Master.fd_count; j++)
						{
							SOCKET outSock = Master.fd_array[j];
							if (outSock != Listening && outSock != client)
							{
								send(outSock, strOut.c_str(), strOut.size() + 1, 0);
							}
						}
						cout << strOut << endl;
						break;
					}
				}
			}
			else
			{
				int indexClient = 0;
				char buf[4096];
				ZeroMemory(buf, 4096);

				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					closesocket(sock);
					FD_CLR(sock, &Master);
				}
				else
				{
					string cmd = string(buf, (bytesIn - 1));
					if (cmd == "/Quit Server")
					{
						a--;
						string msg = "Server is shutting down. Goodbye\r\n";
						while (Master.fd_count > 0)
						{
							SOCKET sock = Master.fd_array[0];

							send(sock, msg.c_str(), msg.size() + 1, 0);

							FD_CLR(sock, &Master);
							closesocket(sock);
						}

						WSACleanup();
						break;
					}
					else
					{
						ostringstream ss;
						for (int i = 0; i < countMAXClient; i++)
						{
							if (DataClient[i] == sock)
							{
								ss << "Client " << i + 1 << " :" << buf << "\r\n";
								string strOut = ss.str();
								for (int i = 0; i < Master.fd_count; i++)
								{
									SOCKET outSock = Master.fd_array[i];
									if (outSock != Listening && sock != outSock)
									{
										send(outSock, strOut.c_str(), strOut.size() + 1, 0);
									}
								}
								cout << strOut << endl;

								ofstream foutput;
								ifstream finput;
								finput.open("ReceivedChat.txt");
								foutput.open("ReceivedChat.txt", ios::app);

								if (finput.is_open())
									foutput << strOut;
								finput.close();
								foutput.close();
							}
						}
					}
				}
			}
		}
	}
}