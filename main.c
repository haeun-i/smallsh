#include "smallsh.h"

char *prompt; 
// char *prompt = "Command> "; 기존 prompt 초기화 문자 삭제


int main() {
	// shell에서는 SIGINT가 발생해도 종료되지 않도록 설정
	struct sigaction act;
	sigfillset(&(act.sa_mask));
	act.sa_handler = SIG_IGN;
	sigaction(SIGINT, &act, NULL);
	
	while(userin(prompt) != EOF)
		procline();
	return 0;
}
