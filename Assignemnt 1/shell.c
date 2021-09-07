#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

// Simplifed xv6 shell.

#define MAXARGS 10

// All commands have at least a type. Have looked at the type, the code
// typically casts the *cmd to some specific cmd type.
struct cmd {
  int type;          //  ' ' (exec), | (pipe), '<' or '>' for redirection
};

struct execcmd {
  int type;              // ' '
  char *argv[MAXARGS];   // arguments to the command to be exec-ed
};

struct redircmd {
  int type;          // < or > 
  struct cmd *cmd;   // the command to be run (e.g., an execcmd)
  char *file;        // the input/output file
  int mode;          // the mode to open the file with
  int fd;            // the file descriptor number to use for the file
};

struct pipecmd {
  int type;          // |
  struct cmd *left;  // left side of pipe
  struct cmd *right; // right side of pipe
};

int fork1(void);  // Fork but exits on failure.
struct cmd *parsecmd(char*);

// Execute cmd.  Never returns.
void
runcmd(struct cmd *cmd)
{
  int p[2], r;
  struct execcmd *ecmd;
  struct pipecmd *pcmd;
  struct redircmd *rcmd;

  if(cmd == 0)
    exit(0);
  
  switch(cmd->type){
  default:
    fprintf(stderr, "unknown runcmd\n");
    exit(-1);

  case ' ':
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0)
      exit(0);
    //fprintf(stderr, "exec not implemented\n");
    // Your code here ...
	
	/* execcmd looks at the argv commands to execute*/
	//https://brennan.io/2015/01/16/write-a-shell-in-c/
	//Format:  [exec command] [other], first word is the execution command, the rest is possibly anything else.
	//https://linux.die.net/man/3/execvp
	//Replaces the current process with the exec argv 0 command and processes the rest later.
	//The first argument, by convention, should point to the filename associated with the file being executed
	//Together they describe a list of one or more pointers to null-terminated strings that represent the argument list available to the executed program
	
	//First argument points to the path,  second argument is the file.
	//argv[0] contains the executable name
	//int execvp(const char *file, char *const argv[]);
	execvp(ecmd->argv[0], ecmd->argv);
    break;

  case '>':
  case '<':
    rcmd = (struct redircmd*)cmd;
    //fprintf(stderr, "redir not implemented\n");
    // Your code here ...
	
	//https://www.tutorialspoint.com/unix/unix-io-redirections.htm
	/* Redirects input of a command  */

	//The file descriptor the program is currently using for input or output is available in rcmd->fd
	
	//In Unix, dup2 works as int dup2(int old_file_descriptor, int new_file_descriptor);
	//Parser provides the left command in pcmd->left and the right command in pcmd->right

	//The file descriptor the program is currently using for input or output is available in rcmd->fd
	//Using RCMD
	
	/*

	File descriptors and fork interact to make I/O redirection easy to implement.
	Fork copies the parent’s file descriptor table along with its memory, so that the child
	starts with exactly the same open files as the parent


	if(fork() == 0) {
	close(0);
	open("input.txt", O_RDONLY);
	exec("cat", argv);
	}

	*/
	
	//Open(char *filename, mode_t mode, permission)
	//open("input.txt", O_RDONLY);
	//open(filename, O_RDWR|O_CREAT, 0666)
	//Also needs permission to write and read stuff
	int pid;
	pid = open(rcmd->file, rcmd->mode, 0777);
	
	if(pid < 0)
	{
		//Did not open file
		fprintf(stderr, "Process Error: Failed to open file %s\n", rcmd->file);
		exit(0);
	}
	//In Unix, dup2 works as int dup2(int old_file_descriptor, int new_file_descriptor);
	//2. The file descriptor the program is currently using for input or output is available in rcmd->fd
	//Fork copies the parent’s file descriptor table along with its memory, so that the child
	//starts with exactly the same open files as the parent. The system call exec replaces the
	//calling process’s memory but preserves its file table
	
	else if(dup2(pid, rcmd->fd) >= 0)
	{
		//Is a valid process.  Can execute.
		close(pid);
		runcmd(rcmd->cmd);
	}
	else
	{
		//Fork / duplicate failed
		fprintf(stderr, "Execution Failure\n");
		exit(0);
	}
    break;

  case '|':
    pcmd = (struct pipecmd*)cmd;
    //fprintf(stderr, "pipe not implemented\n");
    // Your code here ...
	
	//https://www.geeksforgeeks.org/c-program-demonstrate-fork-and-pipe/
	//https://pdos.csail.mit.edu/6.828/2014/xv6/book-rev8.pdf
	//Parser provides the left command in pcmd->left and the right command in pcmd->right
	
	//Create a pipe and return fd’s in p
	/*
	pipe(p);
	if(fork() == 0) {
	close(0);
	dup(p[0]);
	close(p[0]);
	close(p[1]);
	exec("/bin/wc", argv);
	} else {
	write(p[1], "hello world\n", 12);
	close(p[0]);
	close(p[1]);
	}
	*/
	
	int x = pipe(p);
	if(x < 0)
	{
		fprintf(stderr, "Pipe Initalization Failure\n");
		exit(-1);
	}
	//Note that fork(2) creates an exact copy of the current process.
	int forkPipe;
	forkPipe = fork();
	
	if(forkPipe < 0)
	{
		fprintf(stderr, "Fork Failure\n");
		exit(0);
	}
	//The parser provides the left command in pcmd->left and the right command in pcmd->right.
	/* Child process, look at the left or input side */
	else if(forkPipe == 0)						//Child process
	{
		close(1);						//Close OUT
		dup(p[1]);						//dup(p[1]);
		close(p[0]);					//close(p[0]);
		close(p[1]);					//close(p[1]);
		runcmd(pcmd->left);				//Read left side
	}
	
	/* Parent process look at the right side or output */
	else
	{
		close(0);					//Close IN
		dup(p[0]);					//Duplicate read side and run it.
		close(p[0]);				//close(p[0]);
		close(p[1]);				//close(p[1]);
		runcmd(pcmd->right);
		//Wait for child to end
		wait(&forkPipe);
	}
    break;
  }
  exit(0);
}

int
getcmd(char *buf, int nbuf)
{
  
  if (isatty(fileno(stdin)))
    fprintf(stdout, "cs6233> ");
  memset(buf, 0, nbuf);
  fgets(buf, nbuf, stdin);
  if(buf[0] == 0) // EOF
    return -1;
  return 0;
}

int
main(void)
{
  static char buf[100];
  int fd, r;

  // Read and run input commands.
  while(getcmd(buf, sizeof(buf)) >= 0){
    if(buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' '){
      // Clumsy but will have to do for now.
      // Chdir has no effect on the parent if run in the child.
      buf[strlen(buf)-1] = 0;  // chop \n
      if(chdir(buf+3) < 0)
        fprintf(stderr, "cannot cd %s\n", buf+3);
      continue;
    }
    if(fork1() == 0)
      runcmd(parsecmd(buf));
    wait(&r);
  }
  exit(0);
}

int
fork1(void)
{
  int pid;
  
  pid = fork();
  if(pid == -1)
    perror("fork");
  return pid;
}

struct cmd*
execcmd(void)
{
  struct execcmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = ' ';
  return (struct cmd*)cmd;
}

struct cmd*
redircmd(struct cmd *subcmd, char *file, int type)
{
  struct redircmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = type;
  cmd->cmd = subcmd;
  cmd->file = file;
  cmd->mode = (type == '<') ?  O_RDONLY : O_WRONLY|O_CREAT|O_TRUNC;
  cmd->fd = (type == '<') ? 0 : 1;
  return (struct cmd*)cmd;
}

struct cmd*
pipecmd(struct cmd *left, struct cmd *right)
{
  struct pipecmd *cmd;

  cmd = malloc(sizeof(*cmd));
  memset(cmd, 0, sizeof(*cmd));
  cmd->type = '|';
  cmd->left = left;
  cmd->right = right;
  return (struct cmd*)cmd;
}

// Parsing

char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";

int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '<':
    s++;
    break;
  case '>':
    s++;
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;
  
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}

int
peek(char **ps, char *es, char *toks)
{
  char *s;
  
  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return *s && strchr(toks, *s);
}

struct cmd *parseline(char**, char*);
struct cmd *parsepipe(char**, char*);
struct cmd *parseexec(char**, char*);

// make a copy of the characters in the input buffer, starting from s through es.
// null-terminate the copy to make it a string.
char 
*mkcopy(char *s, char *es)
{
  int n = es - s;
  char *c = malloc(n+1);
  assert(c);
  strncpy(c, s, n);
  c[n] = 0;
  return c;
}

struct cmd*
parsecmd(char *s)
{
  char *es;
  struct cmd *cmd;

  es = s + strlen(s);
  cmd = parseline(&s, es);
  peek(&s, es, "");
  if(s != es){
    fprintf(stderr, "leftovers: %s\n", s);
    exit(-1);
  }
  return cmd;
}

struct cmd*
parseline(char **ps, char *es)
{
  struct cmd *cmd;
  cmd = parsepipe(ps, es);
  return cmd;
}

struct cmd*
parsepipe(char **ps, char *es)
{
  struct cmd *cmd;

  cmd = parseexec(ps, es);
  if(peek(ps, es, "|")){
    gettoken(ps, es, 0, 0);
    cmd = pipecmd(cmd, parsepipe(ps, es));
  }
  return cmd;
}

struct cmd*
parseredirs(struct cmd *cmd, char **ps, char *es)
{
  int tok;
  char *q, *eq;

  while(peek(ps, es, "<>")){
    tok = gettoken(ps, es, 0, 0);
    if(gettoken(ps, es, &q, &eq) != 'a') {
      fprintf(stderr, "missing file for redirection\n");
      exit(-1);
    }
    switch(tok){
    case '<':
      cmd = redircmd(cmd, mkcopy(q, eq), '<');
      break;
    case '>':
      cmd = redircmd(cmd, mkcopy(q, eq), '>');
      break;
    }
  }
  return cmd;
}

struct cmd*
parseexec(char **ps, char *es)
{
  char *q, *eq;
  int tok, argc;
  struct execcmd *cmd;
  struct cmd *ret;
  
  ret = execcmd();
  cmd = (struct execcmd*)ret;

  argc = 0;
  ret = parseredirs(ret, ps, es);
  while(!peek(ps, es, "|")){
    if((tok=gettoken(ps, es, &q, &eq)) == 0)
      break;
    if(tok != 'a') {
      fprintf(stderr, "syntax error\n");
      exit(-1);
    }
    cmd->argv[argc] = mkcopy(q, eq);
    argc++;
    if(argc >= MAXARGS) {
      fprintf(stderr, "too many args\n");
      exit(-1);
    }
    ret = parseredirs(ret, ps, es);
  }
  cmd->argv[argc] = 0;
  return ret;
}
