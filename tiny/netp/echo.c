#include "../csapp.h"
#include <pthread.h>
#include <unistd.h>

void echo(int connfd){
    size_t n;
    static char buf[MAXLINE];
    static rio_t rio;
    pthread_t thread;


    void *additional_thread(int fd) {
        while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
            Fputs(buf, stdout);
        }
    }

    Rio_readinitb(&rio, connfd);
    
    pthread_create(&thread, NULL, additional_thread, &connfd);    
    
    while(1){
        //서버 응답 준비
        Fgets(buf, MAXLINE, stdin);
        // 클라이언트에게 돌려줄  문자 소켓에 기록
        Rio_writen(connfd, buf, strlen(buf));
    }
}