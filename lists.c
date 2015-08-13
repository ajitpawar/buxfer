#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lists.h"

/* Add a group with name group_name to the group_list referred to by 
* group_list_ptr. The groups are ordered by the time that the group was 
* added to the list with new groups added to the end of the list.
*
* Returns 0 on success and -1 if a group with this name already exists.
*
* (I.e, allocate and initialize a Group struct, and insert it
* into the group_list. Note that the head of the group list might change
* which is why the first argument is a double pointer.) 
*/
char *add_group(Group **group_list_ptr, const char *group_name) {

    // Build return strings
    int size = strlen(group_name) + 50;
    char s1[size];
    char s2[size];
    snprintf(s1, size, "Group %s added\r\n", group_name);
    snprintf(s2, size, "Group %s already exists\r\n", group_name);

    char *success = malloc(size);
    char *fail = malloc(size);
    strncpy(success, s1, size-1);
    strncpy(fail, s2, size-1);


   if (find_group(*group_list_ptr, group_name) == NULL) {
        Group *newgrp;
        if ((newgrp = malloc(sizeof(Group))) == NULL) {
           perror("Error allocating space for new Group");
           exit(1);
        }
        // set the fields of the new group node
        // first allocate space for the name
        int needed_space = strlen(group_name) + 1;
        if (( newgrp->name = malloc(needed_space)) == NULL) {
           perror("Error allocating space for new Group name");
           exit(1);
        }
        strncpy(newgrp->name, group_name, needed_space);
        newgrp->users = NULL;
        newgrp->xcts = NULL;
        newgrp->next = NULL;

        if (*group_list_ptr == NULL) {
            *group_list_ptr = newgrp;
            //return 0;
            return success;

        } else {
            Group * current = *group_list_ptr;
            while (current->next != NULL) {
                current = current->next;
          }
            current->next = newgrp;
            //return 0;
            return success;
        }
    } else {
        //return -1;
        return fail;
    }
}

/* Print to standard output the names of all groups in group_list, one name
*  per line. Output is in the same order as group_list.
*/
char *list_groups(Group *group_list) {
        
    // Loop once to gather string sizes
    int size = 0;
    Group * current = group_list;
    while (current != NULL) {
        int space_needed = snprintf(NULL, 0, "%s \t", current->name);   
        size += space_needed;
        current = current->next;
    }
    
    // Allocate space and copy strings
    current = group_list; 
    char *output = malloc(size+1);
    char *t = output;
    while (current != NULL) {
        sprintf(t, "%s ", current->name); // copy into string
        t = &t[strlen(t)];          // move pointer to end of string
        current = current->next;
    }

    return output;
}

/* Search the list of groups for a group with matching group_name
* If group_name is not found, return NULL, otherwise return a pointer to the 
* matching group list node.
*/
Group *find_group(Group *group_list, const char *group_name) {
    Group *current = group_list;
    while (current != NULL && (strcmp(current->name, group_name) != 0)) {
        current = current->next;
    }
    return current;
}

/* Add a new user with the specified user name to the specified group. Return zero
* on success and -1 if the group already has a user with that name.
* (allocate and initialize a User data structure and insert it into the
* appropriate group list)
*/
char *add_user(Group *group, const char *user_name) {

    // Build return strings
    int size = strlen(user_name) + strlen(group->name) + 50;
    char s1[size];
    char s2[size];
    snprintf(s1, size, "User %s successfully added to group %s\r\n", user_name, group->name);
    snprintf(s2, size, "User %s already exists in group %s\r\n", user_name, group->name);

    char *success = malloc(size);
    char *fail = malloc(size);
    strncpy(success, s1, size-1);
    strncpy(fail, s2, size-1);

    
    User *this_user = find_prev_user(group, user_name);
    if (this_user != NULL) {
        //return -1;
        return fail; 
    }

    User *newuser;
    if ((newuser = malloc(sizeof(User))) == NULL) {
        perror("Error allocating space for new User");
        exit(1);
    }
    // set the fields of the new user node
    // first allocate space for the name
    int name_len = strlen(user_name);
    if ((newuser->name = malloc(name_len + 1)) == NULL) {
        perror("Error allocating space for new User name");
        exit(1);
    }
    strncpy(newuser->name, user_name, name_len + 1);
    newuser->balance = 0.0;

    // insert this user at the front of the list
    newuser->next = group->users;
    group->users = newuser;
    
    //return 0;
    return success;   
}

/* Print to standard output the names and balances of all the users in group,
* one per line, and in the order that users are stored in the list, namely 
* lowest payer first.
*/
char *list_users(Group *group) {

    /* Overflow explanation:
    -------------------------
        We will use snprintf to calculate the exact number of bytes needed to store 
        the string. When enough space is unavailable in the buffer, snprintf will return 
        the number of bytes (excluding the terminating byte) which would have been 
        written had enough space been available. 

        By suppling the second argument as zero, we are basically getting the entire
        space needed for the string. So, we are guaranteed that the string will fit in
        that space. Also, snprintf() always null terminates and we will always leave
        room for it by allocating size+1 space.
    */

    // Calculate total space needed
    int size = 0;
    User *current = group->users;
    while (current != NULL) {
        // snprintf() will return the entire space needed for this string
        int space_needed = snprintf(NULL, 0, "%s %f\n", current->name, current->balance);   
        size += space_needed;       // add to total
        current = current->next;
    }

    // Allocate space and copy strings
    current = group->users;            
    char *output = malloc(size+1);     // allocate extra space for terminating character
    char *t = output;                  // temp pointer for keeping track of position in string
    while (current != NULL) {   
        sprintf(t, "%s %f\n", current->name, current->balance); // copy into string
        t = &t[strlen(t)];          // move pointer to end of string
        current = current->next;
    }

    return output;    
}


/* Print to standard output the balance of the specified user. Return 0
* on success, or -1 if the user with the given name is not in the group.
*/
char *user_balance(Group *group, const char *user_name) {

    // Build return strings
    int size = strlen(user_name) + strlen(group->name) + 50;
    char s1[size];
    char s2[size];    
    char *success = malloc(size);
    char *fail = malloc(size);
    
    
    User * prev_user = find_prev_user(group, user_name);
    if (prev_user == NULL) { 
        //return -1; 
        snprintf(s2, size, "User %s does not exist in group %s\r\n", user_name, group->name);
        strncpy(fail, s2, size-1);
        return fail;
    }
    if (prev_user == group->users) {
        // user could be first or second since previous is first
        if (strcmp(user_name, prev_user->name) == 0) {
            //printf("Balance is %f\n", prev_user->balance);
            //return 0;
            snprintf(s1, size, "%f", prev_user->balance);
            strncpy(success, s1, size-1);
            return success;
        }
    }
    //printf("Balance is %f\n", prev_user->next->balance);
    //return 0;
    snprintf(s1, size, "%f\n", prev_user->next->balance);
    strncpy(success, s1, size-1);    
    return success;
}

/* Return a pointer to the user prior to the one in group with user_name. If 
* the matching user is the first in the list (i.e. there is no prior user in 
* the list), return a pointer to the matching user itself. If no matching user 
* exists, return NULL. 
*
* The reason for returning the prior user is that returning the matching user 
* itself does not allow us to change the user that occurs before the
* matching user, and some of the functions you will implement require that
* we be able to do this.
*/
User *find_prev_user(Group *group, const char *user_name) {
    User * current_user = group->users;
    // return NULL for no users in this group
    if (current_user == NULL) { 
        return NULL;
    }
    // special case where user we want is first
    if (strcmp(current_user->name, user_name) == 0) {
        return current_user;
    }
    while (current_user->next != NULL) {
        if (strcmp(current_user->next->name, user_name) == 0) {
            // we've found the user so return the previous one
            return current_user;
        }
    current_user = current_user->next;
    }
    // if we get this far without returning, current_user is last,
    // and we have already checked the last element
    return NULL;
}

/* Add the transaction represented by user_name and amount to the appropriate 
* transaction list, and update the balances of the corresponding user and group. 
* Note that updating a user's balance might require the user to be moved to a
* different position in the list to keep the list in sorted order. Returns 0 on
* success, and -1 if the specified user does not exist.
*/
char *add_xct(Group *group, const char *user_name, double amount) {

    // Build return strings
    int size = strlen(user_name) + strlen(group->name) + 50;
    char s2[size];
    snprintf(s2, size, "User %s does not exist in group %s\r\n", user_name, group->name);

    char *success = malloc(size);
    char *fail = malloc(size);
    strncpy(success, "Xct added successfully\r\n", size-1);
    strncpy(fail, s2, size-1);


    User *this_user;
    User *prev = find_prev_user(group, user_name);
    if (prev == NULL) {
        //return -1;
        return fail;
    }
    // but find_prev_user gets the PREVIOUS user, so correct
    if (prev == group->users) {
        // user could be first or second since previous is first
        if (strcmp(user_name, prev->name) == 0) {
            // this is the special case of first user
            this_user = prev;
        } else {
            this_user = prev->next;
        }
    } else {
        this_user = prev->next;
    }

    Xct *newxct;
    if ((newxct = malloc(sizeof(Xct))) == NULL) {
        perror("Error allocating space for new Xct");
        exit(1);
    }
    // set the fields of the new transaction node
    // first allocate space for the name
    int needed_space = strlen(user_name) + 1;
    if ((newxct->name = malloc(needed_space)) == NULL) {
         perror("Error allocating space for new xct name");
         exit(1);
    }
    strncpy(newxct->name, user_name, needed_space);
    newxct->amount = amount;

    // insert this xct  at the front of the list
    newxct->next = group->xcts;
    group->xcts = newxct;

    // first readjust the balance
    this_user->balance = this_user->balance + amount;

    // since we are only ever increasing this user's balance they can only
    // go further towards the end of the linked list
    //   So keep shifting if the following user has a smaller balance

    while (this_user->next != NULL &&
                  this_user->balance > this_user->next->balance ) {
        // he remains as this user but the user next gets shifted
        // to be behind him
        if (prev == this_user) {
            User *shift = this_user->next;
            this_user->next = shift->next;
            prev = shift;
            prev->next = this_user;
            group->users = prev;
        } else { // ordinary case in the middle
            User *shift = this_user->next;
            prev->next = shift;
            prev = shift;
            this_user->next = shift->next;
            shift->next = this_user;
        }
    }
	
    //return 0;
    return success;
}

