#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>    /* Internet domain header */
#include "wrapsock.h"
#include "buxfer.h"

#ifndef PORT
#define PORT 51376
#endif


void init_pool(int listenfd, Pool *p);
void add_client(int connfd, Pool *p);
int readfromclient(Client *c);
void check_clients(Pool *p);
void bindandlisten(int listenfd);


int main(int argc, char **argv) 
{
	int listenfd, connfd;
	socklen_t clilen;
	struct sockaddr_in cliaddr;
	Pool pool;

	// Bind and listen
	listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	bindandlisten(listenfd);

	// Initialize pool
	init_pool(listenfd, &pool);

	while(1) {
		pool.rset = pool.allset;      
		pool.nready = Select(pool.maxfd+1, &pool.rset, NULL, NULL, NULL);	// non-blocking

		if (FD_ISSET(listenfd, &pool.rset)) {    // new client connection
			clilen = sizeof(cliaddr);
			connfd = Accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
			add_client(connfd, &pool);		// add new client	
		}

		check_clients(&pool);	// check for data from all active clients
	}
}

void bindandlisten(int listenfd)
{
	int yes = 1;
	struct sockaddr_in servaddr;

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(PORT);

	if((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))) == -1) {
		perror("setsockopt failed");
	}

	Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));	// bind
	Listen(listenfd, LISTENQ);	// listen
}


// Initialize pool
void init_pool(int listenfd, Pool *p) 
{
    int i;

    p->group = NULL;		// initialize Group list
    p->maxi = -1;        
    p->maxfd = listenfd; 
    for (i=0; i< MAXCLIENTS; i++){	
		p->client[i].soc = -1; 		// set all sockets as available
		p->client[i].curpos = 0;		
	}
    FD_ZERO(&p->allset);
    FD_SET(listenfd, &p->allset);	// listen for new client connections
}


// Add new client connection
void add_client(int connfd, Pool *p)
{
	int i;
	p->nready--;

	// Look for an available socket and add client there
	for (i = 0; i < MAXCLIENTS; i++){
		if (p->client[i].soc < 0) {		
			p->client[i].soc = connfd; 	
			memset(p->client[i].username, 0, MAXLINE); 	// Clear username
			memset(p->client[i].groupname, 0, MAXLINE); // Clear groupname
			memset(p->client[i].buf, 0, MAXLINE); 		// Clear userinput buffer
			memset(p->client[i].cmdbuf, 0, MAXLINE); 	// Clear command buffer
			p->client[i].curpos = 0;
			break;
		}
	}

	// Add new descriptor to set
	FD_SET(connfd, &p->allset);    
	if (connfd > p->maxfd)
		p->maxfd = connfd; // for select 
	if (i > p->maxi)
		p->maxi = i;   // max index in client[] array 

	// Ask client for username
	char *sbuf = "What is your name? ";
	if(write(connfd, sbuf, strlen(sbuf)) == -1){
		perror("write failed");
	}

}


// Check all online clients for data in a non-blocking way
void check_clients(Pool *p)
{
	int i;
	for (i = 0; (i<=p->maxi) && (p->nready > 0); i++) {  		
		if (p->client[i].soc < 0)	
			continue;				// no active client at this socket.

		if (FD_ISSET(p->client[i].soc, &p->rset)) // client state changed
		{	
			p->nready--;
			int result = readfromclient(&p->client[i]);		
			
			// Client closed connection
			if(result == -1)  {		
				Close(p->client[i].soc);
				FD_CLR(p->client[i].soc, &p->allset);
				p->client[i].soc = -1;	// make socket available again
			}

			// Client sent some input
			else{
				int bufsize = strlen(p->client[i].cmdbuf); 
				char *c_buf = p->client[i].cmdbuf;	
				int c_soc = p->client[i].soc;
				char b[MAXLINE];

				// Ignore empty userinput
				if(bufsize == 0)
					continue;
				
				// Check if username is set for this client
				// If not, then set it and welcome him/her
				if(strlen(p->client[i].username) == 0){		// if username not set
					char *tok = strtok(c_buf, "\r\n");		// tokenize
					if(tok != NULL){
						strncpy(p->client[i].username, tok, MAXLINE-1);		// set username
						snprintf(b, MAXLINE-1, "Welcome %s! Please enter Buxfer commands:\n", p->client[i].username);
						if(write(c_soc, b, strlen(b)+1) == -1) {	// send welcome message
							perror("write failed");
						}
						continue;
					}
				}
				
				// Tokenize userinput and run them as buxfer commands
				int retval = tokenize_and_run(&p->client[i], p);

				// Client has "quit"
				if(retval == -1)	
				{ 	
					if(write(c_soc, "Goodbye!\r\n", 11) == -1){	 // Send goodbye message
						perror("write failed");
					}
					// Drop client and make socket available
					Close(p->client[i].soc);	// close socket
					FD_CLR(p->client[i].soc, &p->allset);
					p->client[i].soc = -1;		// make socket available again
				}	

				// Client has joined a new group
				// Notify to all other members of this group
				else if(retval == 1){
					notify(p, p->client[i].username, p->client[i].groupname);
				}

			}
		}
	}
}


/* Notify other users in the group of this client joining their group
*/
void notify(Pool *p, char *username, char *groupname)
{
	int i;
	int size = strlen(username) + strlen(groupname) + 30;
	char mssg[size];
	snprintf(mssg, size-1, "%s has been added to group %s\r\n", username, groupname);

	// Iterate over all online clients
	for (i = 0; i <= p->maxi; i++){
		if (p->client[i].soc < 0)	// ignore sockets not in use
			continue;

		// For all clients that match this groupname
		if(strcmp(p->client[i].groupname, groupname) == 0) {	
			// For all clients, except current user
			if(strcmp(p->client[i].username, username) != 0) {	
				// Broadcast message
				if(write(p->client[i].soc, mssg, strlen(mssg)) == -1){
					perror("write failed");
				}
			}
		}		
	}
}

/*  read from the client
* 	returns -1 if the socket needs to be closed and 0 otherwise */
int readfromclient(Client *c) 
{	
	char *startptr = &c->buf[c->curpos];
	int len = read(c->soc, startptr, MAXLINE - c->curpos);
	if(len <= 0) {
		if(len == -1) {
			perror("read on socket failed");
		}
		return -1;	// connection closed by client

	} else {
		c->curpos += len;
		c->buf[c->curpos] = '\0';

		// Did we get a whole line?
		if(strchr(c->buf, '\n') || strchr(c->buf, '\r')) {

			// Copy everything upto first \r or \n
			// Treat this as the command
			strncpy(c->cmdbuf, c->buf, MAXLINE-1);
			
			// Need to shift anything still in the buffer over to beginning.
			char *leftover = &c->buf[c->curpos];
			memmove(c->buf, leftover, c->curpos);
			c->curpos = 0;
			return 0;
		} else {
			return 0;	// empty userinput
		}
	}
}