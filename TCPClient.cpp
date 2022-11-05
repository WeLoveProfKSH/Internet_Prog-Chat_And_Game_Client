#include "Common.h"
#include <iostream>
#include <string>

char *SERVERIP = (char *)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

// 데이터 통신에 사용할 변수
SOCKET sock;
char buf[BUFSIZE + 1];
char input[BUFSIZE + 1];
int len;
int retval;

std::string nick; // 채팅방 닉네임

DWORD WINAPI Svrrecv(LPVOID arg){	// 메시지 수신 스레드용
	char r[BUFSIZE + 1];

	while (1) {
		// 데이터 받기
		strcpy(r, "");
		retval = recv(sock, r, BUFSIZE, 0);

		// 받은 데이터 출력
		r[retval] = '\0';
		//printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
		printf("[메시지 수신] >> %s\n\n", r);
	}
}

int main(int argc, char *argv[]){
	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];
	if (argc > 2) nick = argv[2];

	if (nick == "") {
		printf("채팅방에서 사용할 이름을 정하십시오 : ");
		std::cin >> nick;
		getchar(); // 입력버퍼 비우기
	}
	nick = nick + " : ";

	// 윈속 초기화
	WSADATA wsa{};
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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

	// 서버로부터 데이터 수신 시작 (수신 전용 스레드 하나 돌리기)
	HANDLE hThread = CreateThread(NULL, 0, Svrrecv, (LPVOID)sock, 0, NULL);
	if (hThread == NULL) { closesocket(sock); }
	else { CloseHandle(hThread); }

	printf("<글자를 입력하시고 엔터를 누르시면 메시지가 전송됩니다.>\n");
	for(int i = 0; i < 30; i++)	printf("──");
	printf("\n\n");
	// 서버와 데이터 통신
	while (1) {
		//printf("\nbuf : '%s', input : '%s'\n", buf, input);
		strcpy(buf, nick.c_str());

		// 데이터 입력
		//printf("\n입력 : ");
		if (fgets(input, BUFSIZE + 1, stdin) == NULL)
			break;

		strncat(buf, input, sizeof(input));

		// '\n' 문자 제거1
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';
		if (strlen(buf) == 0)
			break;
		// '\n' 문자 제거2
		len = (int)strlen(input);
		if (input[len - 1] == '\n')
			input[len - 1] = '\0';
		if (strlen(input) == 0)
			break;

		// 데이터 보내기
		retval = send(sock, buf, (int)strlen(buf), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			break;
		}
		printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n\n", retval);
	}

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
