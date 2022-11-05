#include "Common.h"
#include <iostream>
#include <string>

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

// ������ ��ſ� ����� ����
SOCKET sock;
char buf[BUFSIZE + 1];
char input[BUFSIZE + 1];
int len;
int retval;

std::string nick; // ä�ù� �г���

DWORD WINAPI Svrrecv(LPVOID arg){	// �޽��� ���� �������
	char r[BUFSIZE + 1];

	while (1) {
		// ������ �ޱ�
		strcpy(r, "");
		retval = recv(sock, r, BUFSIZE, 0);

		// ���� ������ ���
		r[retval] = '\0';
		//printf("[TCP Ŭ���̾�Ʈ] %d����Ʈ�� �޾ҽ��ϴ�.\n", retval);
		printf("[�޽��� ����] >> %s\n\n", r);
	}
}

int main(int argc, char *argv[]){
	// ����� �μ��� ������ IP �ּҷ� ���
	if (argc > 1) SERVERIP = argv[1];
	if (argc > 2) nick = argv[2];

	if (nick == "") {
		printf("ä�ù濡�� ����� �̸��� ���Ͻʽÿ� : ");
		std::cin >> nick;
		getchar(); // �Է¹��� ����
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
	retval = connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// �����κ��� ������ ���� ���� (���� ���� ������ �ϳ� ������)
	HANDLE hThread = CreateThread(NULL, 0, Svrrecv, (LPVOID)sock, 0, NULL);
	if (hThread == NULL) { closesocket(sock); }
	else { CloseHandle(hThread); }

	printf("<���ڸ� �Է��Ͻð� ���͸� �����ø� �޽����� ���۵˴ϴ�.>\n");
	for(int i = 0; i < 30; i++)	printf("����");
	printf("\n\n");
	// ������ ������ ���
	while (1) {
		//printf("\nbuf : '%s', input : '%s'\n", buf, input);
		strcpy(buf, nick.c_str());

		// ������ �Է�
		//printf("\n�Է� : ");
		if (fgets(input, BUFSIZE + 1, stdin) == NULL)
			break;

		strncat(buf, input, sizeof(input));

		// '\n' ���� ����1
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;
		// '\n' ���� ����2
		len = (int)strlen(input);
		if (input[len - 1] == '\n')
			input[len - 1] = '\0';
		if (strlen(input) == 0)
			break;

		// ������ ������
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
