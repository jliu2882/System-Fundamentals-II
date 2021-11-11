#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cook.h"

void test(){
	printf("\n\n");
	printf("Cookbook file can be found here: %s\n", cookbook);
	printf("Main Recipe: %s\n", recipe->name);
	printf("We are specifying that we need %lli cooks\n", maxCooks);
	printf("First Name: %s\n", cbp->recipes->next->name);
}

int invalidRecipe(char *mainRecipe){ //Implicitly has extern variables from cook.h
	RECIPE *first = cbp->recipes; //Iterate through all the recipes to validate our recipe is in it
	while(first!=NULL){ //If th current recipe is null, we reached the end
		if(!strcmp(first->name, mainRecipe)) break; //Note that we assume names are unique to differentiate eggs vs poached_eggs
		first=first->next; //Go to the next recipe
	} //Iterated through the recipes
	if(first==NULL) return 1; //If we reached the end without finding a match, the recipe WAS invalid
	recipe = first; //Set the recipe to the one we found; this line prevents us from doing return first==NULL
	return 0; //We successfully found the recipe within the cookbook
}

void addToQueue(RECIPE_LINK *leaf){ //I guess doing workQueue=&leaf for nonpointer leaf doesn't work...
//TODORNOT: check if recipe is leaf(might not if we don't clear the depends_on list)
	NODE *queueNode = malloc(sizeof(*queueNode));
	queueNode->recipe = leaf->recipe;
	if(!checkQueue(queueNode->recipe->name)) return;
	if(workQueue==NULL){ //Check if the queue is initalized yet(or emptied out)
		queueNode->next = NULL; //Sanitize the shallow copy
		workQueue=queueNode; //If the queue is empty, initialize it with the leaf node
	} else{ //The queue is non empty
		queueNode->next=workQueue; //Insert our leaf at the front(for LIFO)
		workQueue=queueNode; //Use our leaf as the new front of queue
	}
}

NODE *popQueue(){ //TODO RETEST AND CHECK FOR MEMLEAKS
	NODE *first = workQueue; //Get the most recently inserted node
	workQueue = workQueue->next; //Move the workQueue to the next spot(if there is none, it will be null)
	first->next = NULL; //Sanitize the node before we pop it(somehow it has to occur after ^)
	free(first);
	return first; //Return the first node
}

int checkQueue(char *leafName){ //An ideal program might combine checkQueue with invalidRecipe and take first/node as params

	NODE *node = malloc(sizeof(*node ));
	node = workQueue;


	while(node!=NULL){ //This is the same program as invalidRecipe
		if(!strcmp(node->recipe->name, leafName)) break; //Just that we use workQueue instead of cbp->recipes
		node=node->next; //In a larger scale program, the extensibility might be something we pay attention too
	} //or maybe not idk this definitely makes the code more readable
	if(node==NULL) return 1; //althought renaming the function and return 0 here and 1 later might help
printf("~%s is already in our queue\n", leafName);
	return 0; //but trust the process i guess
}

int dfs(int level, RECIPE *bruh){
	for(int i =0 ; i< level; i++){
		printf(" ");
	}
	printf("%s\n",bruh->name);
	if(bruh->this_depends_on==NULL){
		printf("DEAL WITH ME PLEASE\n");
		return 1;
	}
	RECIPE_LINK *current = bruh->this_depends_on;
	while(current!=NULL){
		dfs_helper(level+1,current);
		current=current->next;
	}
	return 0;
}

void dfs_helper(int level, RECIPE_LINK *bruh){
	for(int i =0 ; i< level; i++){
		printf("  ");
	}
	printf("%s\n",bruh->name);
	if(bruh->recipe->this_depends_on==NULL){
		addToQueue(bruh);
		return;
	}
	RECIPE_LINK *current = bruh->recipe->this_depends_on;
	while(current!=NULL){
		dfs_helper(level+1,current);
		current=current->next;
	}
}