#include "Common.h"
#include <direct.h>
#include <iostream>
#include <string>

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

// ������ ��ſ� ����� ����
SOCKET sock;
char buf[BUFSIZE + 1];		// Ŭ���̾�Ʈ�� ������ ���� �޽����� ��Ƶ� �迭
char input[BUFSIZE + 1];	// Ŭ���̾�Ʈ�� ����ڷκ��� �Է� ���� ���ڸ� ��Ƶ� �迭
int len;
int retval;

std::string nick; // ä�ù� �г���

DWORD WINAPI Svrrecv(LPVOID arg) {	// �޽��� ���� �������
	char r[BUFSIZE + 1];	// ������ �����͸� ��Ƶ� char �迭

	while (1) {	// ������ �ִ� ������ �ޱ�
		strcpy(r, "");	// recv()�Լ��� �����͸� �ޱ����� �迭 ����
		retval = recv(sock, r, BUFSIZE, 0);	// ������ ����

		// ���� ������ ���
		r[retval] = '\0';
		//printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		printf("[�޽��� ����] >> %s\n\n", r);
	}
}

int main(int argc, char* argv[]) {
	// ����� �μ��� ������ IP �ּҷ� ���
	if (argc > 1) SERVERIP = argv[1];
	// 2��° �μ��� �г�������
	if (argc > 2) nick = argv[2];

	while(nick == ""){	// ���� �г����� ���ڰ����� �־����� �ʾҴٸ�
		printf("ä�ù濡�� ����� �̸��� ���Ͻʽÿ� : ");
		std::cin >> nick;
		getchar(); // �Է¹ް� \n �ѱ��� �����
	}
	nick = nick + " : ";

	// ���� �ʱ�ȭ
	WSADATA wsa{};
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ------- �����κ��� ������ ���� ���� (���� ���� ������ �ϳ� ������) -----
	HANDLE hThread = CreateThread(NULL, 0, Svrrecv, (LPVOID)sock, 0, NULL);
	if (hThread == NULL) { closesocket(sock); }
	else { CloseHandle(hThread); }
	//-------------------------------------------------------------------------
	printf("< ���ڸ� �Է��Ͻð� ���͸� �����ø� �޽����� ���۵˴ϴ�. >\n");
	printf("< �����Ϸ��� 'exit'�� �Է��Ͻʽÿ�. >\n");
	for (int i = 0; i < 30; i++)	printf("����");
	printf("\n\n");

	// ������ ���
	while (1) {
		//printf("\nbuf : '%s', input : '%s'\n", buf, input);	// ����׿� ���
		strcpy(buf, nick.c_str());	// �ϴ� buf�� "�г��� : " �̷������� �����ϱ�

		// ���� ������ �Է�
		if (fgets(input, BUFSIZE + 1, stdin) == NULL)	// fgets()�Լ��� input�� �Է¹޴µ�, �ƹ��͵� �Է����� ������ break;
			break;

		strncat(buf, input, sizeof(input));	// buf�� input�̶� ��ġ��, -> "�г��� : " + '����� �Է�'

		if (strcmp(input, "exit\n") == 0) {		// exit �Է��ϸ� ���α׷� ����
			printf("[���α׷��� �����մϴ�.]\n");
			break;
		}

		if (strcmp(input, "blj\n") == 0) {	// blj �Է� ������ ���� ����
			printf("[���� ������ �����մϴ�.]\n");
			_getcwd(buf, 512);				// ���� ���� ��� ���
			system(strcat(buf, "\\bljc.jar"));
			continue;
		}

		// buf �迭�� '\n' ���� ����
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		// input �迭�� '\n' ���� ����
		len = (int)strlen(input);
		if (input[len - 1] == '\n')
			input[len - 1] = '\0';
		
		// ������ ������ ������
		if (strlen(input) == 0)	continue;	// ���� ����ڰ� �ƹ��͵� �Է����� �ʾҴٸ� �ѱ��

		retval = send(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� ���½��ϴ�.\n\n", retval);
	}

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}
