#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "cook.h"
#include <unistd.h>

int invalidRecipe(char *mainRecipe){ //Check if the mainRecipe string is valid and if so, assign recipe to the recipe
	RECIPE *first = cbp->recipes; //Iterate through all the recipes to validate our recipe is in it
	while(first!=NULL){ //If the current recipe is null, we reached the end
		if(!strcmp(first->name, mainRecipe)) break; //Names are unique to differentiate eggs vs poached_eggs
		first=first->next; //Go to the next recipe
	} //Iterated through the recipes; could use a for loop to lower the lines of code but
	recipe = first; //Set the recipe to the one we found; if none was found, the program will exit soon anyway
	return first==NULL; //If we reached the end without finding a match, the recipe is invalid
} //Implicitly has extern variables from cook.h

void addToQueue(RECIPE *node){ //Add a node to our queue given a recipe(since recipe_link can lead to recipes)
	if(!isLeaf(node) || existsInQueue(node->name)) return; //If the recipe isn't a leaf or exists already, don't add it
	NODE *queueNode = malloc(sizeof(*queueNode)); //Create a node that will last(malloc recipe_link didn't work)
	queueNode->recipe = node; //Assign it the value of the recipe in the leaf(to create a shallow copy)
	if(workQueue==NULL){ //If the queue is empty
		queueNode->next = NULL; //It should be NULL by default, but we can't guarentee with malloc
		workQueue=queueNode; //If the queue is empty, initialize it with the leaf node
	} else{ //If the queue is not empty
		queueNode->next=workQueue; //Our node should point to the first node in the workQueue
		workQueue=queueNode; //Use our leaf as the new front of queue because we're doing LIFO
	} //The node has been added to our queue
} //I guess doing workQueue=&leaf for nonpointer leaf doesn't work...

int isLeaf(RECIPE *node){ //Return true if node is a leaf
	if(node->this_depends_on==NULL) return 1; //The node has no dependencies
	RECIPE_LINK *current = node->this_depends_on; //Get the first dependency of the recipe
	while(current!=NULL){ //Iterate through the dependencies
		if(!isCompleted(current->recipe)) return 0; //If the dependency isn't done, we don't have a leaf
		current=current->next; //Move to the next dependency
	} //We have finished checking node
	return 1; //We iterated through all the dependencies and learned that they are all done
} //We will mark recipes with completed status to determine the new leaves

int isCompleted(RECIPE *node){ //Return true if node is completed
	STATE *state = (STATE *)(node->state); //Get the pointer to the state
	if(state==NULL) return 0; //We necessarily initialize state when we complete a recipe
	return state->complete; //This will be 1 if the recipe is complete; 0 otherwise
} //Relatively simple function, but the abstraction helps with readability

int existsInQueue(char *name){ //Checks to see if the name exists in the queue already
	NODE *node = malloc(sizeof(*node)); //Create a node that exists after function call(malloc recipe_link didn't work)
	node = workQueue; //Give it the value of workQueue, so we can iterate through it
	while(node!=NULL){ //Note that NULL means the end of a workQueue(or an empty queue)
		if(!strcmp(node->recipe->name, name)) break; //Check if the recipe name exists
		node=node->next; //Very similar invalidRecipe, so with some modifications, extensibility could be a focus
	} //Finished checking every node in workQueue
	return node!=NULL; //If we broke out before reaching the end, we must have found an existing copy
} //An ideal program could potentially combine existInQueue with invalidRecipe

RECIPE *popAndProcessQueue(){ //Pop the first node in the workQueue, process it and return the processed recipe
	NODE *first = workQueue; //Get the most recently inserted node
	workQueue = workQueue->next; //Move the workQueue to the next spot(if there is none, it will be null)
	RECIPE *poppedRecipe = first->recipe; //Get the recipe associated with the node
	free(first); //Free the workQueue node(NOTE: we don't free state here because the recipe may still be checked)
	processRecipe(poppedRecipe); //Process the recipe
	return poppedRecipe; //Return the processed recipe(useful for testing purposes)
} //I chose to do pop and process because there's no world where we pop but then do nothing

void processRecipe(RECIPE *node){ //Process the recipe and set the state to complete


//printf("Just an FYI, we should be starting %s\n", node->name);
//Run the recipe thing
//ther are multiple tasks to run but do first for now
/*
printf("changing dir too util(0 succses): %i\n",chdir("util/"));
char **task =node->tasks->steps->words;
printf("we just did: \n");
    for (int i = 0; i < 4; ++i) {
        printf("%s ", task[i]);
    }
printf("\n~%s!\n",task[0]);
//eceute must be mlaloc of 2+tsizeoftgkasof
char *execute = "./";
printf("\n~~~~%s\n",execute);
strcat(execute,task[0]);
printf("\n~~~~%s\n",execute);
fflush(stdout);
int ret = execvp(execute, task);
printf("return code: %i\n",ret);
*/
//free execiute from its mortal shell


	STATE *state; //Declare a pointer to a state
	if(node->state==NULL){ //If the state does not exist
		state = malloc(sizeof(*state)); //Initialize a pointer to state that can last
		node->state = state; //Set the state of the node to reflect this fact
	} else{ //There must have been a previous state
		state = node->state; //Keeps previous state
	} //state now points to the state of the recipe
	state->complete = 1; //Set the completed flag to true since we finished processing node
	dfs(recipe,0); //Re-add any new leaves created
} //idk I like having comments here

int dfs(RECIPE *node, int level){ //Recursively traverse the first recipe, returning the leaf status of node
//printf("%*s%s\n",level,"",node->name);
	if(isLeaf(node)) { //If node is a leaf
		addToQueue(node); //Add the leaf to the workQueue
		return 1;//If the node is a leaf, we want to let the caller know(only main will even check)
	} //This guarentees that node is a not a leaf, although it only matters for the return code
	RECIPE_LINK *current = node->this_depends_on; //Get the first dependency of the recipe
	while(current!=NULL){ //Iterate through the dependencies
		if(isCompleted(current->recipe)) return 1; //If the dependency is done, we don't need to continue
		dfs(current->recipe,level+1); //Delve into the recipe if it wasn't completed yet
		current=current->next; //Move to the next dependency
	} //We have finished checking node
	return 0; //node isn't a leaf
} //Level is entire unneeded but is useful for debugging