#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS 0


#include<WinSock2.h>


#include <thread>
#include <iostream>
using namespace std;

#pragma comment (lib,"ws2_32")
#pragma comment (lib,"winmm")

#pragma comment(lib, "Ws2_32.lib")
//�ͻ���
class modble_client {
private:
	WSADATA wsaData;
	SOCKET sHost;
	SOCKADDR_IN servAddr;

	//�ڲ���Ϣ����
	void cl(void Received(char* data), void discontinue(const char* err)) {
		char recv_buf[MAXBYTE] = { 0 };
		int recv_len;
		while (1) {
			recv_len = recv(sHost, recv_buf, MAXBYTE, 0);
			if (recv_len < 0) {
				discontinue("�����������Ͽ�����");
				break;
			}
			else
			{
				Received(recv_buf);
			}

		}
	}
public:
	//�����IP,�����port,�Ƿ�����,���ݵ����¼�,�Ͽ������¼�
	void init(const char* ip, int port, bool isok, void Received(char* data), void discontinue(const char* err)) {
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
			discontinue("Winsock load faild!");
			return;
		}

		//  �������׽���
		sHost = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sHost == INVALID_SOCKET) {
			discontinue("socket faild!");
			return;
		}

		servAddr.sin_family = AF_INET;
		//  ע��   ���ѿͻ��˳��򷢵����˵ĵ���ʱ �˴�IP���Ϊ���������ڵ��Ե�IP
		servAddr.sin_addr.S_un.S_addr = inet_addr(ip);
		servAddr.sin_port = htons(port);
		//  ���ӷ�����
		if (connect(sHost, (LPSOCKADDR)& servAddr, sizeof(servAddr)) == SOCKET_ERROR) {
			discontinue("connect faild!");
			return;
		}
		if (isok) {
			cl(Received, discontinue);
		}
		else
		{
			thread t123(&modble_client::cl, this, Received, discontinue);
			t123.detach();
		}

		return;

	}
	void ssend(const char* data) {
		send(sHost, data, MAXBYTE, 0);
	}
	void exit() {
		closesocket(sHost);
		WSACleanup();
	}
};

/*******************************************************************************************/

//*****************************����Ϊmodsocket_server*******************************************
//�����
class modsocket_server {

private:
	WSADATA wsaData;
	sockaddr_in addr;
	SOCKET s;
	//�����Ͽ���ͬʱ����5000��
	SOCKET clientSock[5000];
	SOCKADDR clientAddr[5000];
	sockaddr_in clientsa[5000];
	int uid = 0;

	/*
	�ڲ���Ϣ����
	����һ:���ݵ����¼�
	������:�û��Ͽ��¼�
	������:�û�socket�ṹ��
	������:�û�info�ṹ��
	*/
	void cl(void Received(SOCKET, sockaddr_in, char*), void leave(SOCKET, sockaddr_in), SOCKET xxyh, sockaddr_in yhinfo) {
		//���ϵĽ��տͻ��˷��͹�������Ϣ
		while (TRUE)
		{
			char buff[MAXBYTE] = { 0 };
			//�������߿ͻ��˷���������
			int i ;
			i = recv(xxyh, buff, MAXBYTE, 0);
			if (i >= 0 ) {
				//�¼�֪ͨ::��������
				Received(xxyh, yhinfo, buff);
			}
			else
			{

				//�¼�֪ͨ::�ͻ��Ͽ�
				closesocket(xxyh);
				leave(xxyh, yhinfo);
				//uid--;
				return;
			}
		}
	}

	/*
	�ڲ��¼�����
	����һ:�û������¼�
	������:���ݵ����¼�
	������:�û��Ͽ��¼�
	������:�˿�
	*/
	void forecho(void  into(SOCKET, sockaddr_in), void Received(SOCKET, sockaddr_in, char*), void leave(SOCKET, sockaddr_in),void error(const char*), int port) {

		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {

			error("Winsock load faild!");
			return;
		}
		s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

		if (s == INVALID_SOCKET) {
			error("socket faild!");
			return;
		}

		addr.sin_addr.S_un.S_addr = inet_addr("0.0.0.0");
		addr.sin_port = htons(port);
		addr.sin_family = PF_INET;

		bind(s, (SOCKADDR*)& addr, sizeof(SOCKADDR));

		if (listen(s, 1)) {
			error("����ʧ��,���ܶ˿��Ѿ�����,�볢�Ը���...");
			return;
		}

		int nSize = sizeof(SOCKADDR);


		while (TRUE) {
			clientSock[uid] = accept(s, (SOCKADDR*)& clientAddr, &nSize);

			//�¼�֪ͨ::�û�����
			int len = sizeof(clientsa[uid]);
			getpeername(clientSock[uid], (struct sockaddr*) & clientsa[uid], &len);
			into(clientSock[uid], clientsa[uid]);
			//��Ϣ����

			//cl( Received, leave);
			thread t123(&modsocket_server::cl, this, Received, leave, clientSock[uid], clientsa[uid]);
			t123.detach();
			uid++;
		}

	}
public:

	/*
	��ʼ���¼�
	����һ:�û������¼�
	������:���ݵ����¼�
	������:�û��Ͽ��¼�
	������:������
	������:�˿�
	������:�Ƿ�����
	*/
	void init(void  into(SOCKET, sockaddr_in), void Received(SOCKET, sockaddr_in, char*), void leave(SOCKET, sockaddr_in),void error(const char*), int port, bool isok) {
		if (isok) {
			//����ģʽ
			forecho(into, Received, leave,error, port);
		}
		else
		{
			//������ģʽ
			thread t123a(&modsocket_server::forecho, this, into, Received, leave, error, port);
			t123a.detach();
		}


	}
	//��ȡ�û�Զ��IP
	char* get_user_ip(SOCKET client) {
		sockaddr_in sa;
		int len = sizeof(sa);
		getpeername(client, (struct sockaddr*) & sa, &len);
		return inet_ntoa(sa.sin_addr);
	}

	//��ȡ�û�Զ�̶˿�
	int get_user_port(SOCKET client) {
		sockaddr_in sa;
		int len = sizeof(sa);
		getpeername(client, (struct sockaddr*) & sa, &len);
		return ntohs(sa.sin_port);
	}
	void Collective_Notice(const char* text,SOCKET nosend) {
		for (int i = 0; i < sizeof(clientSock); i++ ) {
			if (clientSock[i] != nosend) {
				send(clientSock[i], text, MAXBYTE, 0);
			}
			
		}
	}
	void Collective_Notice(const char* text) {
		for (int i = 0; i < sizeof(clientSock); i++) {
			send(clientSock[i], text, MAXBYTE, 0);
		}
	}
	void ssend(SOCKET client, const char* text) {
		send(client, text, MAXBYTE, 0);
	}
	void exit() {
		//�رշ���
		closesocket(s);
		WSACleanup();
	}
};

//***************************************����Ϊmodsocket_server********************************************
