#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr,
                    &clientlen);  // line:netp:tiny:accept
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    
    Close(connfd);  // line:netp:tiny:close
  }
}

void doit(int fd) {
  int is_static;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];

  // rio_readlineb를 위해 rio_t 타입(구조체)의 읽기 버퍼를 선언
  rio_t rio;

  /* Read request line and headers */
  /* Rio = Robust I/O */
  // rio_t 구조체를 초기화 해준다.
  Rio_readinitb(&rio, fd); // &rio 주소를 가지는 읽기 버퍼와 식별자 connfd를 연결한다.
  Rio_readlineb(&rio, buf, MAXLINE); // 버퍼에서 읽은 것이 담겨있다.
  printf("Request headers:\n");
  printf("%s", buf); // "GET / HTTP/1.1"
  sscanf(buf, "%s %s %s", method, uri, version); // 버퍼에서 자료형을 읽는다, 분석한다.

 if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD")) {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  
  /* GET method라면 읽어들이고, 다른 요청 헤더들을 무시한다. */
  read_requesthdrs(&rio);
  
  /* Parse URI form GET request */
  /* URI 를 파일 이름과 비어 있을 수도 있는 CGI 인자 스트링으로 분석하고, 요청이 정적 또는 동적 컨텐츠를 위한 것인지 나타내는 플래그를 설정한다.  */
  is_static = parse_uri(uri, filename, cgiargs);
  printf("uri : %s, filename : %s, cgiargs : %s \n", uri, filename, cgiargs);

  /* 만일 파일이 디스크상에 있지 않으면, 에러메세지를 즉시 클라아언트에게 보내고 메인 루틴으로 리턴 */
  if (stat(filename, &sbuf) < 0) { //stat는 파일 정보를 불러오고 sbuf에 내용을 적어준다. ok 0, errer -1
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

    /* Serve static content */
  if (is_static) {
    // 파일 읽기 권한이 있는지 확인하기
    // S_ISREG : 일반 파일인가? , S_IRUSR: 읽기 권한이 있는지? S_IXUSR 실행권한이 있는가?
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
        // 권한이 없다면 클라이언트에게 에러를 전달
        clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
        return;
    }
    // 그렇다면 클라이언트에게 파일 제공
    serve_static(fd, filename, sbuf.st_size);
  } else { /* Serve dynamic content */
    /* 실행 가능한 파일인지 검증 */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
        // 실행이 불가능하다면 에러를 전달
        clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
        return;
    }
    // 그렇다면 클라이언트에게 파일 제공.
    serve_dynamic(fd, filename, cgiargs);
  }
}
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg) {
  char buf[MAXLINE], body[MAXBUF];
  /* Build the HTTP response body */
  /* 브라우저 사용자에게 에러를 설명하는 응답 본체에 HTML도 함께 보낸다 */
  /* HTML 응답은 본체에서 컨텐츠의 크기와 타입을 나타내야하기에, HTMl 컨텐츠를 한 개의 스트링으로 만든다. */
  /* 이는 sprintf를 통해 body는 인자에 스택되어 하나의 긴 스트리잉 저장된다. */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);
  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int) strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}


void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  
  /* strcmp 두 문자열을 비교하는 함수 */
  /* 헤더의 마지막 줄은 비어있기에 \r\n 만 buf에 담겨있다면 while문을 탈출한다.  */
  while (strcmp(buf, "\r\n"))
  {
     //rio 설명에 나와있다 싶이 rio_readlineb는 \n를 만날때 멈춘다.
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
    // 멈춘 지점 까지 출력하고 다시 while
  }
  return;
}
int parse_uri(char *uri, char *filename, char *cgiargs) {
  char *ptr;
  
  /* strstr 으로 cgi-bin이 들어있는지 확인하고 양수값을 리턴하면 dynamic content를 요구하는 것이기에 조건문을 탈출 */
  if (!strstr(uri, "cgi-bin")) { /* Static content*/
    strcpy(cgiargs, "");
    strcpy(filename, ".");
    strcat(filename, uri);

    //결과 cgiargs = "" 공백 문자열, filename = "./~~ or ./home.html
     // uri 문자열 끝이 / 일 경우 home.html을 filename에 붙혀준다.
    if (uri[strlen(uri) - 1] == '/') {
      strcat(filename, "home.html");
    }
    return 1;

  } else { /* Dynamic content*/
    // uri 예시: dynamic: /cgi-bin/adder?first=1213&second
    ptr = index(uri, '?');
    // index 함수는 문자열에서 특정 문자의 위치를 반환한다
    // CGI인자 추출
    if (ptr) {
      // 물음표 뒤에 있는 인자 다 갖다 붙인다.
      // 인자로 주어진 값들을 cgiargs 변수에 넣는다.
      strcpy(cgiargs, ptr + 1);
      // 포인터는 문자열 마지막으로 바꾼다.
      *ptr = '\0'; // uri물음표 뒤 다 없애기
    } else {
      strcpy(cgiargs, ""); // 물음표 뒤 인자들 전부 넣기
    }
    strcpy(filename, "."); // 나머지 부분 상대 uri로 바꿈,
    strcat(filename, uri); // ./uri 가 된다.
    return 0;
  }
}
void serve_static(int fd, char *filename, int filesize) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];
  /* send response headers to client*/
  //파일 접미어 검사해서 파일이름에서 타입 가지고
  get_filetype(filename, filetype);  // 접미어를 통해 파일 타입 결정한다.
  // 클라이언트에게 응답 줄과 응답 헤더 보낸다기
  // 클라이언트에게 응답 보내기
  // 데이터를 클라이언트로 보내기 전에 버퍼로 임시로 가지고 있는다.
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer : Tiny Web Server \r\n", buf);
  sprintf(buf, "%sConnection : close \r\n", buf);
  sprintf(buf, "%sContent-length : %d \r\n", buf, filesize);
  sprintf(buf, "%sContent-type : %s \r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));
  //rio_readn은 fd의 현재 파일 위치에서 메모리 위치 usrbuf로 최대 n바이트를 전송한다.
  //rio_writen은 usrfd에서 식별자 fd로 n바이트를 전송한다.
  //서버에 출력
  printf("Response headers : \n");
  printf("%s", buf);

  //읽을 수 있는 파일로 열기
  srcfd = Open(filename, O_RDONLY, 0); //open read only 읽고
  // PROT_READ -> 페이지는 읽을 수만 있다.
  // 파일을 어떤 메모리 공간에 대응시키고 첫주소를 리턴
  srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0); //메모리로 넘기고
  //srcp = (char *) Malloc(filesize);
  //Rio_readn(srcfd, srcp, filesize);
  // 매핑위치, 매핑시킬 파일의 길이, 메모리 보호정책, 파일공유정책,srcfd ,매핑할때 메모리 오프셋

  Close(srcfd); // 닫기

  Rio_writen(fd, srcp, filesize);
  // mmap() 으로 만들어진 -맵핑을 제거하기 위한 시스템 호출이다
  // 대응시킨 녀석을 풀어준다. 유효하지 않은 메모리로 만듦
  // void *mmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
  // int munmap(void *start, size_t length);
  Munmap(srcp, filesize); //메모리 해제
}

void get_filetype(char *filename, char *filetype) {
  if (strstr(filename, ".html")) {
    strcpy(filetype, "text/html");
  } else if (strstr(filename, ".gif")) {
    strcpy(filetype, "image/gif");
  } else if (strstr(filename, ".png")) {
    strcpy(filename, "image/.png");
  } else if (strstr(filename, ".jpg")) {
    strcpy(filetype, "image/jpeg");
  } else {
    strcpy(filetype, "text/plain");
  }
}

void serve_dynamic(int fd, char *filename, char *cgiargs) {
  char buf[MAXLINE], *emptylist[] = {NULL};
  /* Return first part of HTTP response*/
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  Rio_writen(fd, buf, strlen(buf)); //왜 두번 쓰나요?
  sprintf(buf, "Server: Tiny Web Server\r\n");
  Rio_writen(fd, buf, strlen(buf));
  //fd는 정보를 받자마자 전송하나요???
  //클라이언트는 성공을 알려주는 응답라인을 보내는 것으로 시작한다.
  if (Fork() == 0) { //타이니는 자식프로세스를 포크하고 동적 컨텐츠를 제공한다.
    setenv("QUERY_STRING", cgiargs, 1);
    //자식은 QUERY_STRING 환경변수를 요청 uri의 cgi인자로 초기화 한다.  (15000 & 213)
    Dup2(fd, STDOUT_FILENO); //자식은 자식의 표준 출력을 연결 파일 식별자로 재지정하고,

    Execve(filename, emptylist, environ);
    // 그 후에 cgi프로그램을 로드하고 실행한다.
    // 자식은 본인 실행 파일을 호출하기 전 존재하던 파일과, 환경변수들에도 접근할 수 있다.

  }
  Wait(NULL); //부모는 자식이 종료되어 정리되는 것을 기다린다.
}
