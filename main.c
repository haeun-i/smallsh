#include "smallsh.h"

char *prompt;

int main() {
	
	signal(SIGINT, SIG_IGN);
	while(userin(prompt) != EOF)
		procline();
	return 0;
}
