#include "smallsh.h"

static char inpbuf[MAXBUF];
static char tokbuf[2*MAXBUF];
static char *ptr = inpbuf;
static char *tok = tokbuf;

static char special[] = {' ', '\t', '&', ';', '\n', '\0'}; // 특수문자 처리

int userin(char* p) {  
	int c, count;
	ptr = inpbuf;
	tok = tokbuf;

	printf("%s", p);
	count = 0;

	while(1) { 
		if ((c = getchar()) == EOF)
			return EOF;
		if (count < MAXBUF) // 입력받은 글자를 inpbuf에 삽입한다.
			inpbuf[count++] = c;
		if (c == '\n' && count < MAXBUF) { 
			inpbuf[count] = '\0'; // 줄바뀜 문자 입력되면 null문자 삽입
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
					runcommand(arg, type, narg); // runcommand에 argment의 수 까지 매개변수에 보내도록 변경
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
		if(!strcmp(cline[i], ">")){ // > 가 입력된 경우 파일을 출력할 path의 정보부터 확인한다. 
			index = i+1;  // >가 입력된 이후 다음의 argument가 path에 관한 정보이다.
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
	else{ // cd, exit가 아닌 경우
		switch (pid = fork()) {
			case -1:
				perror("smallsh");
				return -1;
			case 0:
				if(path != NULL){ // '>'가 입력되었을 때만 path가 지정됨 -> path가 null이 아니면 > 명령이 아니므로 바로 프로그램을 실행한다.
					int fd = open(path, O_WRONLY|O_APPEND|O_CREAT, 0644); // 명령어의 내용이 작성될 파일 open
					dup2(fd, 1); // 결과값을 파일에 담는다.
					close(fd);

					for(int i=index-1; cline[i]!=NULL; i++){
						cline[i] = cline[i+2]; 
						// '>'와 path 경로의 관한 정보를 배열의 뒷 부분을 앞으로 끌어서 가져옴으로써
						// redirect 명령을 완전히 종료시키고 다른 명령을 받을 수 있게 한다.
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
