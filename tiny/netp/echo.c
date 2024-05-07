#include "../csapp.h"

void echo(int connfd){
    size_t n;
    char buf[MAXLINE];
    rio_t rio;

    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
        //입력 바이트 출력
        //printf("server received %d bytes\n", (int) n);
        //콘솔에 입력된 문자 출력
        Fputs(buf, stdout);
        // 입력된 문자 소켓에 기록
        Rio_writen(connfd, buf, n);
    }
}