#include "smallsh.h"

static char inpbuf[MAXBUF];
static char tokbuf[2*MAXBUF];
static char *ptr = inpbuf;
static char *tok = tokbuf;

static char special[] = {' ', '\t', '&', ';', '\n', '\0'};

int userin(char* p) { 
	int c, count;
	ptr = inpbuf;
	tok = tokbuf;

	printf("%s", p);
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
	
	for (;;) {
		switch (toktype = gettok(&arg[narg])) {
			case ARG:
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
					runcommand(arg, type, narg);
				}
				if (toktype == EOL) return;
				narg = 0;
				break;
		}
	}
}

int runcommand(char **cline, int where, int narg) {
	pid_t pid;
	int status;
	int index;
	char* path = NULL;

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