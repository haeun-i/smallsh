#include "smallsh.h"

char *prompt; 

int main() {
	
	struct sigaction act;
	sigfillset(&(act.sa_mask));
	act.sa_handler = SIG_IGN;
	sigaction(SIGINT, &act, NULL);
	
	while(userin(prompt) != EOF)
		procline();
	return 0;
}
