#ifndef BUXFER_H
#define BUXFER_H
#include "lists.h"

#define MAXLINE 256
#define MAXCLIENTS 30
#define LISTENQ 10

#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 5
#define DELIM " \n"


typedef struct {
	int soc;
	char buf[MAXLINE];		// stores all userinput
	char cmdbuf[MAXLINE];	// stores userinput upto \r or \n
	char username[MAXLINE];
	char groupname[MAXLINE];
	int curpos;
} Client;

typedef struct {
    int maxfd;         // max descriptor in allset    
    fd_set allset;     // set of all active descriptors 
    fd_set rset;       // subset of ready descriptors 
    int nready;        // number of ready descriptors from select 
    int maxi;          // index into client array 
    Client client[MAXCLIENTS];  // set of active connfd descriptors 
    Group *group;
} Pool;


void error(const char *msg);
int process_args(int cmd_argc, char **cmd_argv, Client *c, Pool *p);
int tokenize_and_run(Client *c, Pool *p);
void notify(Pool *p, char *username, char *groupname);
void Write(int connfd, char *mssg);

#endif