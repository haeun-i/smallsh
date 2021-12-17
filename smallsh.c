#include "smallsh.h"

static char inpbuf[MAXBUF];
static char tokbuf[2*MAXBUF];
static char *ptr = inpbuf;
static char *tok = tokbuf;

static char special[] = {' ', '\t', '&', ';', '\n', '\0'};
void join(char* com1[], char* com2[]);
int fatal(char* );

int userin(char* p) { 
	int c, count;
	ptr = inpbuf;
	tok = tokbuf;

	p = strcat(getcwd(p, MAXBUF), "$ "); // 경로 뒤에 '&'값 붙이기 위해 strcat 함수 사용
	printf("%s", p); // 경로 출력
	count = 0;

	while(1) {
		if ((c = getchar()) == EOF)
			return EOF;
		if (count < MAXBUF)
			inpbuf[count++] = c;
		if (c == '\n' && count < MAXBUF) {
			inpbuf[count] = '\0';
			return count;
		}
		if (c == '\n' || count >= MAXBUF) {
			printf("smallsh: input line too long\n");
			count = 0;
			printf("%s", p);
		}
	}
}

int gettok(char** outptr) {
	int type;
	*outptr = tok;
	while (*ptr == ' ' || *ptr == '\t')
		ptr++;
	*tok++ = *ptr;
	switch (*ptr++) {
		case '\n':
			type = EOL;
			break;
		case '&':
			type = AMPERSAND;
			break;
		case ';':
			type = SEMICOLON;
			break;
		case '|':
			type = PIPE;
			break;
		default:
			type = ARG;
			while(inarg(*ptr))
				*tok++ = *ptr++;
	}
	*tok++ = '\0';
	return type;
}

int inarg (char c) {
	char *wrk;

	for (wrk = special; *wrk; wrk++) {
		if (c == *wrk)
			return 0;
	}
	return 1;
}

void procline() {
	char *arg[MAXARG + 1];
	int toktype, type;
	int narg = 0;
	int checkpipe = 0;
	
	for (;;) {
		switch (toktype = gettok(&arg[narg])) {
			case PIPE:
			case ARG:
				if (toktype == PIPE) checkpipe = 1;
				if (narg < MAXARG)
					narg++;
				break;
			case EOL:
			case SEMICOLON:
			case AMPERSAND:
				if (toktype == AMPERSAND) type = BACKGROUND;
				else type = FOREGROUND;
				
				if (narg != 0) {
					arg[narg] = NULL;
					runcommand(arg, type, narg, checkpipe);
				}
				if (toktype == EOL) return;
				narg = 0;
				break;
		}
	}
}

int runcommand(char **cline, int where, int narg, int checkpipe) {
	pid_t pid;
	int status;
	int index;
	char* path = NULL;

	struct sigaction act;
	sigfillset(&(act.sa_mask));
	act.sa_handler = SIG_DFL;
	act.sa_flags = SA_RESTART | SA_NOCLDSTOP | SA_NOCLDWAIT;
	sigaction(SIGCHLD, &act, NULL);

        for(int i=0; cline[i]!=NULL; i++){
		if(!strcmp(cline[i], ">")){
			index = i+1;
		       	path = cline[index];
		}
	}

	if(!strcmp(cline[0], "exit")){
		exit(1);
	}
	else if(!strcmp(cline[0], "cd")){
		if(narg > 2) printf("Usage: cd <directory>\n");
		else
			chdir(cline[1]);
	}
	else{
		switch (pid = fork()) {
			case -1:
				perror("smallsh");
				return -1;
			case 0:
				if(where == FOREGROUND){ // foreground procee의 경우 SIGINT가 발생했을 때 종료
					struct sigaction act2;
                	                sigfillset(&(act2.sa_mask));
       					act2.sa_handler = SIG_DFL;
        				sigaction(SIGINT, &act2, NULL);
				}
				else{ // background process의 경우 SIGINT가 발생해도 종료되지 않음
					struct sigaction act2;
                        	        sigfillset(&(act2.sa_mask));
					act2.sa_handler = SIG_IGN;
					sigaction(SIGINT, &act2, NULL);
				}

				if(checkpipe){
					char* com1[10];
					char* com2[10];
					
					for(int i=0; cline[i] != NULL; i++){
						if(*cline[i] == '|') {
							int j=0;
							for(; cline[i+1] != NULL; j++) {
								com2[j] = cline[i+1];
								i++;
							}
							com2[j] = NULL;
						}

						com1[i] = cline[i];
					}

					join(com1, com2);
					return 0;
				}
			
					
				if(path != NULL){
				int fd = open(path, O_WRONLY|O_APPEND|O_CREAT, 0644);
					dup2(fd, 1);
					close(fd);

					for(int i=index-1; cline[i]!=NULL; i++){
						cline[i] = cline[i+2];
					}	
					
				}

				execvp(*cline, cline);
				perror(*cline);
				exit (1);
		}
	}

	/* following is the code of parent */
	if (where == BACKGROUND) {
		printf("[Process id] %d\n", pid);
		return 0;
	}
	if (waitpid(pid, &status, 0) == -1)
		return -1;
	else
		return status;
}

void join (char *com1[], char *com2[]) {
	int p[2], status;

	switch (fork()) {
		case -1:
			fatal("1st fork call in join");
		case 0:
			break;
		default:
			wait(&status);
	}

	if (pipe(p) == -1)
		fatal("pipe call in join");
	switch (fork()) {
		case -1:
			fatal("2nd fork call in join");
		case 0:
			dup2(p[1],1);
			close(p[0]);
			close(p[1]);
			execvp (com1[0], com1);
			fatal("1st execvp call in join");
		default:
			dup2(p[0], 0);
			close(p[0]);
			close(p[1]);
			execvp(com2[0], com2);
			fatal("2nd execvp call in join");
	}
}

int fatal(char* s){
	perror(s);
	exit(1);
}
