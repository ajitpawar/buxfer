#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "lists.h"
#include "buxfer.h"


/* 
 * Read and process buxfer commands
 * Return -1 to indicate user has quit
 * Return 1 to indicate broadcast message must be sent to all other users
 * Return 0 otherwise
 */
int process_args(int cmd_argc, char **cmd_argv, Client *c, Pool *p) 
{
    Group *g;
    char *buf;
    char *username = c->username;

    if (cmd_argc <= 0) {
        return 0;

    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return -1;  // -1 indicates used has quit
        
    } else if (strcmp(cmd_argv[0], "add_group") == 0 && cmd_argc == 2) {
        
        // add group
        buf = add_group(&p->group, cmd_argv[1]);  
        Write(c->soc, buf);

        // add user
        if ((g = find_group(p->group, cmd_argv[1])) == NULL) {    
            buf = "Cannot add user. Group does not exist";    
        } else{
            buf = add_user(g, username);                               
            strncpy(c->groupname, cmd_argv[1], MAXLINE);    // set groupname for this client            
            Write(c->soc, buf);   
            return 1;       // Return 1 indicates notification should be sent to all other users
        }   

    } else if (strcmp(cmd_argv[0], "list_groups") == 0 && cmd_argc == 1) {
        buf = list_groups(p->group);
    
    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 2) {
        if ((g = find_group(p->group, cmd_argv[1])) == NULL) {
            buf = "Group does not exist";
        } else {
            buf = list_users(g);
        }
      
    } else if (strcmp(cmd_argv[0], "user_balance") == 0 && cmd_argc == 2) {
        if ((g = find_group(p->group, cmd_argv[1])) == NULL) {
            buf = "Group does not exist";
        } else {
            buf = user_balance(g, username);            
        }
        
    } else if (strcmp(cmd_argv[0], "add_xct") == 0 && cmd_argc == 3) {
        if ((g = find_group(p->group, cmd_argv[1])) == NULL) {
            buf = "Group does not exist";            
        } else {
            char *end;
            double amount = strtod(cmd_argv[2], &end);
            if (end == cmd_argv[2]) {
                buf = "Incorrect number format";
            } else {
                buf = add_xct(g, username, amount);
            }
        }

    } else {
        buf = "Incorrect syntax";
    }

    // Write output to client
    Write(c->soc, buf);     // Write output to client
    return 0;
}


// Tokenize userinput and then process them as buxfer commands
// This was previously the "main" for buxfer.c
int tokenize_and_run(Client *c, Pool *p) 
{
    char *cmd_argv[INPUT_ARG_MAX_NUM];
    int cmd_argc;

    // Tokenize arguments 
    char *next_token = strtok(c->cmdbuf, DELIM);
    cmd_argc = 0;
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            printf("Too many arguments!\n");
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }
    cmd_argv[cmd_argc] = NULL;

    // Process arguments
    return process_args(cmd_argc, cmd_argv, c, p);
}


// Write message to client
void Write(int connfd, char *mssg){
    if(write(connfd, mssg, strlen(mssg)) == -1){
        perror("write failed");
    }
}