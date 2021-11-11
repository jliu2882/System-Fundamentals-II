#include "cookbook.h"

typedef struct node {
    struct recipe *recipe;
    struct node *next;
} NODE;

COOKBOOK *cbp;
char *cookbook;
RECIPE *recipe;
unsigned long long maxCooks;
NODE *workQueue;

int invalidRecipe(char *mainRecipe);
void addToQueue(RECIPE_LINK *leaf);
NODE *popQueue();
int checkQueue(char *leafName);
int dfs(int level, RECIPE *bruh);
void dfs_helper(int level, RECIPE_LINK *bruh);




void test(); //hehecar