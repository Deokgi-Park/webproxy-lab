#include "../csapp.h"

int main(void) {
  //버퍼와 포인터 설정
  char *buf, *p;
  //인자를 받을 변수 1,2 와 페이지정보를 기록할 content 생성
  char args1[MAXLINE], args2[MAXLINE], content[MAXLINE];
  //n1, n2 숫자형 선언
  int n1=0, n2=0;

  //쿼리
  if((buf = getenv("QUERY_STRING")) != NULL){
    p = strchr(buf, '&');
    *p = '\0';
    strcpy(args1, buf);
    strcpy(args2, p+1);
    n1 = atoi(args1);
    n2 = atoi(args2);
  }

  sprintf(content, "QUERY_STRING=%s", buf);
  sprintf(content, "Welcome to add.com : ");
  sprintf(content, "%sTHE Internet addition portal. \r\n<p>", content);
  sprintf(content, "%sTHE answer is : %d + %d = %d \r\n</p>", content, n1, n2, n1+n2);
  sprintf(content, "%s<div>Thanks for visiting!\r\n</div>", content);

  printf("Connection: close\r\n");
  printf("Content-length: %d\r\n", (int)strlen(content));
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content);
  fflush(stdout);
  
  exit(0);
}
/* $end adder */
