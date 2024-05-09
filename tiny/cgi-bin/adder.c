#include "../csapp.h"

int main(void) {
  //버퍼와 포인터 설정
  char *buf, *p;
  //인자를 받을 변수 1,2 와 페이지정보를 기록할 content 생성
  char args1[MAXLINE], args2[MAXLINE], content[MAXLINE];
  //n1, n2 숫자형 선언
  int n1=0, n2=0;

  //GET 요청으로 온 쿼리스트링 값을 BUF에 넣고 해당 값이 있는 경우
  if((buf = getenv("QUERY_STRING")) != NULL){
    // 포인터에 해당 값 검색하고 주소값 전달
    p = strchr(buf, '&');
    // 매개변수를 끊어주기 위해 줄바꿈 문자로 치환
    *p = '\0';
    // 1번 변수에 끊은 문자 삽입
    strcpy(args1, buf);
    // 2번 변수에 포인터 이후 문자 삽입
    strcpy(args2, p+1);
    
    strcpy(args1, strchr(args1, '=')+1);
    strcpy(args2, strchr(args2, '=')+1);
    
    // 정수 변환 작업
    n1 = atoi(args1);
    n2 = atoi(args2);
  }
  // 출력 바디 셋팅
  sprintf(content, "QUERY_STRING=%s \r\n", buf);
  sprintf(content, "<p>Welcome to add.com : ");
  sprintf(content, "%sTHE Internet addition portal.</p> \r\n", content);
  sprintf(content, "%s<p>THE answer is : %d + %d = %d</p> \r\n", content, n1, n2, n1+n2);
  sprintf(content, "%s<div>Thanks for visiting!\r\n</div>", content);
  sprintf(content, "%s<a href='..'>before\r\n</a>", content);

  //헤더 설정
  printf("Connection: close\r\n");
  //본문 길이 설정 해당 설정이 맞지 않는 경우 브라우저에서 에러 발생되거나 본문이 잘림
  printf("Content-length: %d\r\n", (int)strlen(content));
  //헤더 이후 빈줄 생성해서 헤더와 본문 구분
  printf("Content-type: text/html\r\n\r\n");
  printf("%s", content);
  //플루시로 콘솔 내용 클라이언트에게 전달
  fflush(stdout);
  
  exit(0);
}
/* $end adder */
