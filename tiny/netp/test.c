#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    int status;
    struct addrinfo hints;
    struct addrinfo *serviceInfo; // 요청한 서비스와 포트데 맞는 소켓

    memset(&hints, 0, sizeof(hints)); // hints 구조체의 모든 값을 0으로 초기화
    //hints.ai_family = AF_UNSPEC; // IPv4와 IPv6 상관하지 않고 결과를 모두 받겠다
    hints.ai_family = AF_UNSPEC; // IPv4와 IPv6 상관하지 않고 결과를 모두 받겠다
    //hints.ai_socktype = SOCK_STREAM; // TCP stream socket type : 1
    
    if(!strcmp(argv[1],"0")){
        status = getaddrinfo(NULL, NULL, &hints, &serviceInfo);
    }
    if(!strcmp(argv[1],"1")){
        status = getaddrinfo(argv[2], NULL, &hints, &serviceInfo);
    }
    if(!strcmp(argv[1],"2")){
        status = getaddrinfo(NULL, argv[2], &hints, &serviceInfo);
    }
    if(!strcmp(argv[1],"3")){
        status = getaddrinfo(argv[2], argv[3], &hints, &serviceInfo);
    }

    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        return 1;
    }

    // serviceInfo 포인터를 이용하여 리스트의 각 addrinfo 구조체에 접근하여 정보를 출력
    for (struct addrinfo *p = serviceInfo; p != NULL; p = p->ai_next) {
        struct sockaddr *addr = p->ai_addr;
        // addrinfo 구조체의 정보 출력
        char host[NI_MAXHOST], service[NI_MAXSERV];
        status = getnameinfo(p->ai_addr, p->ai_addrlen, host, NI_MAXHOST, service, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
        if (status != 0) {
            fprintf(stderr, "getnameinfo error: %s\n", gai_strerror(status));
            continue;
        }
        //printf("Address: %s, Length: %d\n", host, p->ai_addrlen);
        printf("Address: %s, socket type: %d\n", host, p->ai_socktype);
        
        if (addr->sa_family == AF_INET) { // IPv4 주소인 경우
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)addr;
            printf("Port: %d\n", ntohs(ipv4->sin_port)); // 포트 번호 출력
        } else if (addr->sa_family == AF_INET6) { // IPv6 주소인 경우
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)addr;
            printf("Port: %d\n", ntohs(ipv6->sin6_port)); // 포트 번호 출력
        }
    }

    // 메모리 해제
    freeaddrinfo(serviceInfo);

    return 0;
}


// 출력
// Address: 142.250.207.100, Length: 16