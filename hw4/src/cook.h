#include "cookbook.h"

typedef struct node {
    struct recipe *recipe;
    struct node *next;
} NODE;
typedef struct state{
    int complete;
} STATE;

COOKBOOK *cbp;
char *cookbook;
RECIPE *recipe;
unsigned long long maxCooks;
unsigned long long busyCooks;
NODE *workQueue;

int invalidRecipe(char *mainRecipe);
void addToQueue(RECIPE *leaf);
int isLeaf(RECIPE *node);
int isCompleted(RECIPE *node);
int existsInQueue(char *name);
RECIPE *popAndProcessQueue();
void processRecipe(RECIPE *node);
int dfs(RECIPE *node, int level);
int isQueueEmpty();
void freeCBP();
void sigchld_handler(int sig);
int runPipeline(TASK *task);
void runPipelineHelper(int inputPipe, int outputPipe, STEP *step, char *input_file, char *util, char *backup);