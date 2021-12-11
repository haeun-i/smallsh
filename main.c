// 12191586 김하은 shell 구현 과제 main.c

#include "smallsh.h"

char *prompt;

int main() {
	
	signal(SIGINT, SIG_IGN); // shell에서는 SIGINT 무시
	while(userin(prompt) != EOF)  // EOF이 아니면
		procline(); // procline 실행 (parsing해서 실제 응용프로그램 실행)
	return 0;
}
