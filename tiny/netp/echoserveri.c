#include "../csapp.h"
#include "echo.c"

void echo(int connfd);

int main(int argc, char **argv){
    //클라이언트를 위한 파일 디스크립션 생성, 듣기 소켓, 연결 상태 유지 소켓
    int listenfd, connfd;
    //소켓 주소 크기 체크용 변수
    socklen_t clientlen;
    //클라이언트 소켓 저장소
    struct sockaddr_storage clientaddr;
    //클라이언트 주소 및 포트 저장용 변수, 최대 MAXLINE 만큼만 저장 
    char client_hostname[MAXLINE], client_port[MAXLINE];
    // 듣기소켓 포트 설정
    if(argc != 2){
        //미지정시 랜덤 포트 지정
        argv[1] = "0"; 
        // fprintf(stderr, "usage : %s <port> \n", argv[0]);
        // exit(0);
    }
    // 자기 자신의 포트 정보로 소켓 검색
    listenfd = Open_listenfd(argv[1]);
    while(1){
        //소켓 저장소 구조체 크기만큼 clientlen(주소크기)를 설정
        clientlen = sizeof(struct sockaddr_storage);
        //연결 소켓을 받아들이는 것을 대기한 상태로 지정
        //listenfd(듣기소켓)의 연결 요청을 감지
        //clientaddr에 클라이언트 소켓정보 저장
        //clientlen clientaddr구조체 크기 정보 전달
        connfd = Accept(listenfd, (SA*) &clientaddr, &clientlen);
        // 받은 소켓(clientaddr)의 정보를 가져옴
        Getnameinfo((SA*) &clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        // 연결 상태 출력
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        // 서버에 출력하는 함수
        echo(connfd);
        // 소켓 닫기
        Close(connfd);
    }
    //프로그램 종료
    exit(0);
}