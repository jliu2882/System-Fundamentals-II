#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cook.h"

void test(){
	printf("Cookbook file can be found here: %s\n", cookbook);
	printf("Main Recipe: %s\n", recipe->name);
	printf("We are specifying that we need %lli cooks\n", maxCooks);
	printf("First Name: %s\n", cbp->recipes->next->name);
}

int validateRecipe(char *mainRecipe){
	RECIPE *first = cbp->recipes; //Iterate through all the recipes to validate our recipe is in it
	while(first!=NULL){ //If th current recipe is null, we reached the end
		if(!strcmp(first->name, mainRecipe)) break; //Note that we assume names are unique to differentiate eggs vs poached_eggs
		first=first->next; //Go to the next recipe
	} //Iterated through the recipes
	if(first==NULL) return -1; //If we reached the end without finding a match, return an error
	recipe = first; //Set the recipe to the one we found; this line prevents us from doing return first==NULL
	return 0; //We successfully found the recipe within the cookbook
}
