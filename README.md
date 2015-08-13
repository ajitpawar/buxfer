## Buxfer

Buxfer is a service that lets group of people track shared expenses.<br/>
Idea inspired by the actual company [Buxfer](https://www.buxfer.com/)

### How does it work
For each expense, Buxfer records the person who paid the expense and the expense cost. It al lows group
members to examine the amount that individuals have paid, look at the history of all  transactions, or determine the group member that is currently owing the most.

### Commands supported

```add_group group_name``` <br/>
Register a new group with name group_name

```list_groups``` <br/>
List the names of all groups that are currently registered.

```add_user group_name user_name``` <br/>
Register a new user with name user_name as a member of group group_name.

```list_users group_name``` <br/>
List the names of all users of group group_name together with their current balances (i.e. how much each has paid so far), sorted by their balances (lowest payer first).

```user_balance group_name user_name``` <br/>
Return the balance of user user_name in group group_name

```remove_user group_name user_name``` <br/>
Remove user user_name from group group_name

```underpaid group_name``` <br/>
Output the name of the user in group group_name who has paid the least.

```add_xct group_name user_name amount``` <br/>
Add a new transaction for group group_name. The transaction is paid by user_name and the amount is amount.

```recent_xct group_name num_xct``` <br/>
List the num_xct most recent transactions for group group_name.

```quit``` <br/>
Shut down buxfer
