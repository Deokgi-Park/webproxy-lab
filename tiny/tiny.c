#include "csapp.h"

//하위 함수 끌어 올리기
void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *filename, char *cgiargs);
void serve_static(int fd, char *filename, int filesize, char *method);
void get_filetype(char *filename, char *filetype);
void serve_dynamic(int fd, char *filename, char *cgiargs);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);

int main(int argc, char **argv) {
  //듣기 소켓, 연결 소켓 생성(fd)
  int listenfd, connfd;
  //호스트IP(or 도메인), port(or 프로토콜) 변수 선언
  char hostname[MAXLINE], port[MAXLINE];
  //클라이언트 소켓 길이 설정 
  socklen_t clientlen;
  //클라이언트 소켓 저장소 설정 
  struct sockaddr_storage clientaddr;
  /* 포트 인자 존재 여부 체크 */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  /* 포트 인자 존재 여부 체크 */

  //내 소켓 찾고 듣기 소켓으로 설정
  listenfd = Open_listenfd(argv[1]);
  //run
  while (1) {
    //클라이언트 주소 크기를 스토리지 사이즈만큼의 크기로 설정
    clientlen = sizeof(clientaddr);
    //연결 소켓 값을 받은 준비
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); 
    // 받은 소켓 정보 셋팅
    Getnameinfo((SA*) &clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    // 받은 소켓 정보 확인
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    // connfd에 동작 수행
    doit(connfd); 
    // 소켓 닫기
    Close(connfd);
  }
}

void doit(int fd) {
  // 정적 여부 체크
  int is_static;
  //stat : 파일의 메타데이터 저장 포멧 
  struct stat sbuf;
  //버퍼, 메소드, 주소, 프로토콜 버전
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  //파일 이름, 
  char filename[MAXLINE], cgiargs[MAXLINE];
  /* Rio = Robust I/O */
  // rio패키지 사용을 위해 rio_t 타입(구조체) 선언
  rio_t rio;
  // &rio 주소를 가지는 읽기 버퍼와 식별자 fd를 연결(초기셋팅)한다.
  Rio_readinitb(&rio, fd); 
  // &rio 주소에 버퍼를 사용한 MAXLINE 만큼 읽기.
  Rio_readlineb(&rio, buf, MAXLINE);
  // 요청자의 헤더 출력
  printf("Request headers:\n");
  // 요청자의 헤더 출력값
  // "GET / HTTP/1.1"
  printf("%s", buf); 
  //버퍼에서 세개 값을 가져오고 %s 에 각각 넣는다 => ex)  'GET' '/' 'HTTP/1.1'
  sscanf(buf, "%s %s %s", method, uri, version);
  // 메소드가 GET이 아닐경우 501 에러 출력, HEAD 도 사용 가능하게 추가
  if (strcasecmp(method, "GET") && strcasecmp(method, "HEAD")) {
    clienterror(fd, method, "501", "Not implemented", "Tiny does not implement this method");
    return;
  }
  
  /* GET 읽어들이고, 다른 요청 헤더들을 무시한다. */
  read_requesthdrs(&rio);
  
  /* Parse URI form GET request */
  /* URI 를 파일 이름과 비어 있을 수도 있는 CGI 인자 스트링으로 분석하고,
  실제로 포인터에 설정한다. (filename, cgiargs)
  리턴으로 정적 또는 동적 컨텐츠를 위한 것인지 나타내는 플래그를 설정한다.  */
  is_static = parse_uri(uri, filename, cgiargs);
  // 파싱 후 내용 확인
  printf("uri : %s, filename : %s, cgiargs : %s \n", uri, filename, cgiargs);

  /* 만일 파일이 경로에 있지 않으면, 
  에러메세지 404를 클라이언트에게 보내고 리턴 */
  if (stat(filename, &sbuf) < 0) { //stat는 파일 정보를 불러오고 sbuf에 내용을 적어준다. ok 0, errer -1
    clienterror(fd, filename, "404", "Not found", "Tiny couldn't find this file");
    return;
  }

  /* Serve static content */
  //정적 콘텐츠인 경우
  if (is_static) {
    // 파일 읽기 권한이 있는지 확인하기
    // S_ISREG : 일반 파일인가? , S_IRUSR: 읽기 권한이 있는지? S_IXUSR 실행권한이 있는가?
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
        // 권한이 없다면 클라이언트에게 에러를 전달
        clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't read the file");
        return;
    }
    // 그렇다면 클라이언트에게 파일 제공
    serve_static(fd, filename, sbuf.st_size, method);
  }
  //동적 콘텐츠인 경우
  else { 
    /* 실행 가능한 파일(일반파일[폴더/링크 등등 제외])인지 검증 */
    /* + 파일 읽기 권한이 있는지 검증 */
    if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode)) {
        //권한이 충분치 않다면 에러를 전달
        clienterror(fd, filename, "403", "Forbidden", "Tiny couldn't run the CGI program");
        return;
    }
    // 검증 통과시 동적 파일 제공
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

//다른 요청 헤더 읽기, PASS 하기
void read_requesthdrs(rio_t *rp)
{
  //버터 변수 생성
  char buf[MAXLINE];
  //한 줄 읽기(빈줄 제거)
  Rio_readlineb(rp, buf, MAXLINE);
  printf("%s", buf);
  /* strcmp 두 문자열을 비교하는 함수 */
  /* 헤더의 마지막 줄은 비어있기에 \r\n 만 buf에 담겨있다면 while문을 탈출한다.  */
  while (strcmp(buf, "\r\n"))
  {
    //rio 설명에 나와있다 싶이 rio_readlineb는 \n를 만날때 멈춘다.
    Rio_readlineb(rp, buf, MAXLINE);
    //버퍼 출력
    printf("%s", buf);
  }
  return;
}
//URI 파싱 작업 수행
int parse_uri(char *uri, char *filename, char *cgiargs) {
  //문자 포인터 생성
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

void serve_static(int fd, char *filename, int filesize, char *method) {
  int srcfd;
  char *srcp, filetype[MAXLINE], buf[MAXBUF];
  
  get_filetype(filename, filetype);
  
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  sprintf(buf, "%sServer : Tiny Web Server \r\n", buf);
  sprintf(buf, "%sConnection : close \r\n", buf);
  sprintf(buf, "%sContent-length : %d \r\n", buf, filesize);
  sprintf(buf, "%sContent-type : %s \r\n\r\n", buf, filetype);
  Rio_writen(fd, buf, strlen(buf));

  printf("Response headers : \n");
  printf("%s", buf);

  if(!(strcasecmp(method, "GET"))) {

    srcfd = Open(filename, O_RDONLY, 0); 
    
    //과제 11.9 로 주석
    srcp = Mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    //과제 11.9 로 추가
    //srcp = (char *)malloc(filesize);
    //과제 11.9 로 추가
    //Rio_readn(srcfd, srcp, filesize);
    
    Close(srcfd); // 닫기

    Rio_writen(fd, srcp, filesize);

    //과제 11.9 로 주석
    Munmap(srcp, filesize); //메모리 해제
    //과제 11.9 로 추가
    //free(srcp);
  }
}


//파일의 마임타임을 지정
void get_filetype(char *filename, char *filetype) {
  if (strstr(filename, ".html")) {
    strcpy(filetype, "text/html");
  } else if (strstr(filename, ".gif")) {
    strcpy(filetype, "image/gif");
  } else if (strstr(filename, ".png")) {
    strcpy(filetype, "image/.png");
  } else if (strstr(filename, ".jpg")) {
    strcpy(filetype, "image/jpeg");
  } else if (strstr(filename, ".ico")) {
    strcpy(filetype, "image/vnd.microsoft.icon");
  } else if (strstr(filename, ".mp4")) {
    strcpy(filetype, "video/mp4");
  } else {
    strcpy(filetype, "text/plain");
  }
}

void serve_dynamic(int fd, char *filename, char *cgiargs) {
  // 동적파일 제공을 위한 변수 선언
  char buf[MAXLINE], *emptylist[] = {NULL};
  
  // 헤더 작성
  sprintf(buf, "HTTP/1.0 200 OK\r\n");
  // 파일에 헤더 작성
  Rio_writen(fd, buf, strlen(buf));
  // 헤더 작성
  sprintf(buf, "Server: Tiny Web Server\r\n");
  // 파일에 헤더 작성
  Rio_writen(fd, buf, strlen(buf));
  // 자식 프로세스 생성
  if (Fork() == 0) { 
    // 클라이언트가 전달한 아규먼트 셋팅
    setenv("QUERY_STRING", cgiargs, 1);
    // fd를 지우고 표준 출력에 연결
    Dup2(fd, STDOUT_FILENO); 
    /*
    파일 네임에 해당하는 파일(ex addr)을 실행해서 표준 출력을
    fflush로 콘솔에 전달하면 표준 출력이 파일 디스크립터와 연결되있으니 
    클라이언트가 해당 내용을 받게됨
    */ 
    Execve(filename, emptylist, environ);

  }
  //프로세스 종료를 대기
  Wait(NULL); 
}
