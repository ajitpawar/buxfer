#ifndef LISTS_H
#define LISTS_H

struct group {
	char *name;
	struct user *users;
	struct xct *xcts;
	struct group *next;
};

struct user {
	char *name;
	double balance;
	struct user *next;
};

struct xct{
	char *name;
	double amount;
	struct xct *next;
};

typedef struct group Group;
typedef struct user User;
typedef struct xct Xct;

char *add_group(Group **group_list, const char *group_name);
char *list_groups(Group *group_list);
Group *find_group(Group *group_list, const char *group_name);

char *add_user(Group *group, const char *user_name);
char *list_users(Group *group);
char *user_balance(Group *group, const char *user_name);
User *find_prev_user(Group *group, const char *user_name);

char *add_xct(Group *group, const char *user_name, double amount);

#endif
