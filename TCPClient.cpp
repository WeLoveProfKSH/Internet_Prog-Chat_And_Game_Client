#include "Common.h"
#include <direct.h>
#include <iostream>
#include <string>
#include <windows.h>

HANDLE consoleHandler = GetStdHandle(STD_OUTPUT_HANDLE);
char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    512

// 데이터 통신에 사용할 변수
SOCKET sock;
char buf[BUFSIZE + 1];		// 클라이언트가 서버에 보낼 메시지를 담아둘 배열
char input[BUFSIZE + 1];	// 클라이언트가 사용자로부터 입력 받은 문자를 담아둘 배열
int len;
int retval;

std::string nick; // 채팅방 닉네임

int wherey()	//	Console 창에서의 커서 y값 반환해주는 메서드
{
	if (consoleHandler == INVALID_HANDLE_VALUE)
		return 0;
	CONSOLE_SCREEN_BUFFER_INFO screenInfo;
	GetConsoleScreenBufferInfo(consoleHandler, &screenInfo);
	return screenInfo.dwCursorPosition.Y + 1;
}


int gotoxy(int x, int y) // Console창 커서 위치 이동
{
	if (consoleHandler == INVALID_HANDLE_VALUE)
		return 0;
	COORD coords = { static_cast<short>(x), static_cast<short>(y)-1 };
	SetConsoleCursorPosition(consoleHandler, coords);
	return 1;
}

DWORD WINAPI Svrrecv(LPVOID arg) {	// 메시지 수신 스레드용
	char r[BUFSIZE + 1];	// 수신한 데이터를 담아둘 char 배열

	while (1) {	// 서버가 주는 데이터 받기
		strcpy(r, "");	// recv()함수로 데이터를 받기전에 배열 비우기
		retval = recv(sock, r, BUFSIZE, 0);	// 데이터 수신

		// 받은 데이터 출력
		r[retval] = '\0';
		//printf("[TCP 클라이언트] %d바이트를 받았습니다.\n", retval);
		gotoxy(0, wherey());
		printf("[메시지 수신] >> %s\n\n", r);
		printf("송신할 메시지 입력 >> ");
	}
}

int main(int argc, char* argv[]) {
	// 명령행 인수가 있으면 IP 주소로 사용
	if (argc > 1) SERVERIP = argv[1];
	// 2번째 인수는 닉네임으로
	if (argc > 2) nick = argv[2];

	while(nick == ""){	// 만일 닉네임이 인자값으로 주어지지 않았다면
		printf("채팅방에서 사용할 이름을 정하십시오 : ");
		std::cin >> nick;
		getchar(); // 입력받고 \n 한글자 지우기
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
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");

	// ------- 서버로부터 데이터 수신 시작 (수신 전용 스레드 하나 돌리기) -----
	HANDLE hThread = CreateThread(NULL, 0, Svrrecv, (LPVOID)sock, 0, NULL);
	if (hThread == NULL) { closesocket(sock); }
	else { CloseHandle(hThread); }
	//-------------------------------------------------------------------------
	printf("< 글자를 입력하시고 엔터를 누르시면 메시지가 전송됩니다. >\n");
	printf("< 종료하려면 'exit'를 입력하십시오. >\n");
	for (int i = 0; i < 30; i++)	printf("──");
	printf("\n\n");

	printf("송신할 메시지 입력 >> ");
	// 서버와 통신
	while (1) {
		//printf("\nbuf : '%s', input : '%s'\n", buf, input);	// 디버그용 출력
		strcpy(buf, nick.c_str());	// 일단 buf에 "닉네임 : " 이런식으로 저장하기

		// 보낼 데이터 입력
		if (fgets(input, BUFSIZE + 1, stdin) == NULL)	// fgets()함수로 input에 입력받는데, 아무것도 입력하지 않으면 break;
			break;

		strncat(buf, input, sizeof(input));	// buf랑 input이랑 합치기, -> "닉네임 : " + '사용자 입력'

		if (strcmp(input, "exit\n") == 0) {		// exit 입력하면 프로그램 종료
			printf("[프로그램을 종료합니다.]\n");
			break;
		}

		if (strcmp(input, "blj\n") == 0) {	// blj 입력 받으면 블랙잭 실행
			printf("[블랙잭 게임을 실행합니다.]\n");
			_getcwd(buf, 512);				// 현재 실행 경로 얻기
			system(strcat(buf, "\\bljc.jar"));
			continue;
		}

		if (strcmp(input, "hangman\n") == 0) {	// hangman 입력 받으면 행맨 실행
			printf("[행맨 게임을 실행합니다.]\n");
			_getcwd(buf, 512);				// 현재 실행 경로 얻기
			system(strcat(buf, "\\행맨.exe"));
			continue;
		}
		
		// buf 배열에 '\n' 문자 제거
		len = (int)strlen(buf);
		if (buf[len - 1] == '\n')
			buf[len - 1] = '\0';

		// input 배열에 '\n' 문자 제거
		len = (int)strlen(input);
		if (input[len - 1] == '\n')
			input[len - 1] = '\0';
		
		// 서버로 데이터 보내기
		if (strlen(input) == 0)	continue;	// 만일 사용자가 아무것도 입력하지 않았다면 넘기기

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


/*
참고문헌 목록 : 
https://pracon.tistory.com/123 (cmd 콘솔에서의 gotoxy()함수를 이용한 커서 위치 이동 테크닉)
*/
