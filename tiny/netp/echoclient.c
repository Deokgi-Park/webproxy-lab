#include "../csapp.h"
#include "echo.c"
#include <pthread.h>
#include <unistd.h>

int main(int argc, char **argv){
    //클라이언트 파일 디스크립션 생성
    int clientfd;
    //호스트 이름, 포트번호(or 프로토콜), 버퍼[크기]
    char *host, *port, buf[MAXLINE]; 
    //통신용 쓰기 읽기 함수 사용
    rio_t rio;

    // 아규먼트 갯수가 부족할 경우
    if(argc != 3){
        // 로컬 호스트의 랜덤한... 소켓을 찾도록 설정
        argv[1] = "localhost"; 
        // 해당 값 NULL 일 경우 랜덤한 포트를 호출...
        argv[2] = NULL; 
        //기존 코드 주석처리, 아큐먼트 없는 경우 자동으로 위 설정
        //fprintf(stderr, "usage : %s <host> <port>\n", argv[0]);
        //exit(0);
    }
    //첫먼째 아규먼트 주소(or DNS명)
    host = argv[1];
    //두번째 아규먼트 포트번호(or 프로토콜)
    port = argv[2];
    // 클라이언트에 fd 생성(3), host, port에 맞는 IP 값 리스트 호출
    // 해당 리스트는 TCP/UDP/IP 소켓으로 나뉘나 다음과 같은 설정으로 가져오기 때문에 TCP 소켓만을 리턴할 것이다.
    // hints.ai_socktype = SOCK_STREAM;  /* TCP 소켓만을 가져옴*/
    clientfd = Open_clientfd(host, port);
    //clientfd을 사용해 Rio 연결 및 초기 셋팅
    Rio_readinitb(&rio, clientfd);
    echo(clientfd);
    
    // //글자 입력 받기, NULL(ctrl+d)이 아니면 계속 동작, 
    // while(Fgets(buf, MAXLINE, stdin) != NULL){
    //     //서버 파일 쓰기(소켓)
    //     Rio_writen(clientfd, buf, strlen(buf));
    //     //서버 파일 읽기(소켓)
    //     Rio_readlineb(&rio, buf, MAXLINE);
    //     //서버 응답 읽기
    //     Fputs(buf, stdout);
    // }
    // // 종료 시 소켓 닫기
    Close(clientfd);
    // 프로그램 종료
    exit(0);
}