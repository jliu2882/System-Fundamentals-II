#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "cook.h"
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include "csapp.h"

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
	return state->complete==1; //This will be 1 if the recipe is complete; 0 if unstarted; -1 on errors
} //Relatively simple function, but the abstraction helps with readability

int existsInQueue(char *name){ //Checks to see if the name exists in the queue already
	NODE *node; //Create a node that exists after function call(malloc recipe_link didn't work)
	node = workQueue; //Give it the value of workQueue, so we can iterate through it
	while(node!=NULL){ //Note that NULL means the end of a workQueue(or an empty queue)
		if(!strcmp(node->recipe->name, name)) break; //Check if the recipe name exists
		node=node->next; //Very similar invalidRecipe, so with some modifications, extensibility could be a focus
	} //Finished checking every node in workQueue
	int result = node!=NULL; //If we broke out before reaching the end, we must have found an existing copy
	return result;
} //An ideal program could potentially combine existInQueue with invalidRecipe

RECIPE *popAndProcessQueue(){ //Pop the first node in the workQueue, process it and return the processed recipe
	busyCooks++; //One cook is currently working on this leaf recipe
	NODE *first = workQueue; //Get the most recently inserted node
	workQueue = workQueue->next; //Move the workQueue to the next spot(if there is none, it will be null)
	RECIPE *poppedRecipe = first->recipe; //Get the recipe associated with the node
	free(first); //Free the workQueue node(NOTE: we don't free state here because the recipe may still be checked)
	processRecipe(poppedRecipe); //Process the recipe
	busyCooks--; //The cook is finished working on this recipe
	dfs(recipe,0); //Re-add any new leaves created
	return poppedRecipe; //Return the processed recipe(useful for testing purposes)
} //I chose to do pop and process because there's no world where we pop but then do nothing

void processRecipe(RECIPE *node){ //Process the recipe and set the state to complete
	int completeStatus = 1; //Store the status of the recipe
	TASK *taskPointer = node->tasks; //Get the first task object
	while(taskPointer!=NULL){ //Iterate through the tasks(note that for tasks like get_gas, we skip this process)
		if(runPipeline(taskPointer)) completeStatus = -1; //Run the pipeline and if it fails, set our status to -1
		taskPointer=taskPointer->next; //Move onto the next task
		while ((wait(NULL)) > 0){} //wait for all to finish so cook doesn't do double duty
	} //Finish executing the tasks for node
	STATE *state; //Declare a pointer to a state
	if(node->state==NULL){ //If the state does not exist
		state = malloc(sizeof(*state)); //Initialize a pointer to state that can last
		node->state = state; //Set the state of the node to reflect this fact
	} else{ //There must have been a previous state
		state = node->state; //Keeps previous state
	} //state now points to the state of the recipe
	state->complete = completeStatus; //Set the completed flag to the complete status
} //idk I like having comments here

int dfs(RECIPE *node, int level){ //Recursively traverse the first recipe, returning the leaf status of node
	//printf("%*s%s\n",level,"",node->name); //Print the tree in dfs
	if(isLeaf(node) && !isCompleted(node)) { //If node is a leaf and has not been finished
		addToQueue(node); //Add the leaf to the workQueue
		return 1;//If the node is a leaf, we want to let the caller know(not really needed but w.e)
	} //This guarentees that node is a not a leaf, although it only matters for the return code
	RECIPE_LINK *current = node->this_depends_on; //Get the first dependency of the recipe
	while(current!=NULL){ //Iterate through the dependencies
		if(isCompleted(current->recipe)) return 1; //If the dependency is done, we don't need to continue
		dfs(current->recipe,level+1); //Delve into the recipe if it wasn't completed yet
		current=current->next; //Move to the next dependency
	} //We have finished checking node
	return 0; //node isn't a leaf
} //level is entire unneeded but is useful for debugging

int isQueueEmpty(){ //Returns true if the queue is empty
	return workQueue==NULL; //If the workQueue is null, it must be empty
} //Function used for abstraction purposes

void freeCBP(){ //Free any states we have allocated for states
	RECIPE *recipe = cbp->recipes; //Get the first recipe within the cookbook
	while(recipe!=NULL){ //Iterate through all the recipes within the cookbook
		if(recipe->state!=NULL){ //If we allocated memory for the state(and therefore it is non-NULL)
			free(recipe->state); //Free the allocated memorys
		} //We have successfully freed the state for this recipe
		recipe=recipe->next; //Get the next recipe in the cookbook
	} //Iterated through the cookbook successfully
} //Not going to check cbp->state because I didn't use it

void sigchld_handler(int sig) { //Handler for SIGCHLD signal


//TODO HANDLE CHILD BETTER
//	printf("RECEIVED SIGNAL\n");


} //i like comments




void runPipelineHelper(int inputPipe, int outputPipe, STEP *step, char *input_file, char *util, char *backup){ //Run step
	if ((fork())==0) { //Create a new process
		if(inputPipe!=0){ //Don't try to read from the last pipe if we are the first input
			dup2(inputPipe, STDIN_FILENO); //Read the input from the last pipe
			close(inputPipe); //Close the pipe
		} else{ //If we are the first pipe, we want to read from input_file if applicable
			if(input_file!=NULL){ //If the input file exists, we want to use it
				int fw; //Create an int to store the result
				if((fw=open(input_file,O_RDONLY))<0) exit(EXIT_FAILURE); //Try to open the file
				dup2(fw, STDIN_FILENO); //Use the file as an input
			} //We are using the file as stdin, otherwise it is just default behavior
		} //Input is taken care of
		if(outputPipe!=1) { //Except for
			dup2(outputPipe, STDOUT_FILENO); //Send the output for the next pipe
			close(outputPipe); //Closee the pipe
		} //The output is taken care of
		if(execvp(util, step->words)<0) if(execvp(backup,step->words)) exit(EXIT_FAILURE); //If both fail, the recipe fail
    } //Finished setting up the input/output and running the step
} //comment nice ;)

int runPipeline(TASK *task) { //Run the pipeline on a task
	int index = 0; //Declare an index to store how many pipes we are in deep
	int inputPipe = 0; //The first step in the pipeline should get input here
	int fd[2]; //Declare the "pipe" we will be using
	char *backup; //Declare a pointer to the task
	char *util; //Declare a pointer for the task in the util library
	STEP *currentStep = task->steps; //Get the first step in the task
	if(currentStep==NULL) return 0; //Return success since there are no substeps in the task
	while(currentStep->next!=NULL){ //We want to iterate through the steps but stop before the last one
		char **taskWord = currentStep->words; //Get the words for the current step
		backup = malloc(sizeof(char)*(3+strlen(taskWord[0])) ); //We need space for ./ and potentially an /0
		util = malloc(sizeof(char)*(6+strlen(taskWord[0])) ); //We need space for util/ and potentially an /0
	   	strcpy(backup, "./"); //execute should start looking in util initially; but ./ feels better first
		strcat(backup,taskWord[0]); //Append the rest of the task
	   	strcpy(util, "util/"); //execute should start looking in util initially
		strcat(util,taskWord[0]); //Append the rest of the task
		pipe(fd); //Create a pipe with fd[1] as the new write for the current child
		runPipelineHelper(inputPipe, fd[1], currentStep, task->input_file, util, backup); //Run the step in the pipeline
		close (fd [1]); //Close the pipe this step is done with
		inputPipe = fd [0]; //The read end of the pipe(for the next child) was the write end from the helper
		free(backup); //We no longer need the pointer to the task
		free(util); //We no longer need the pointer to the task
		currentStep=currentStep->next; //Go to the next step
		index++; //Add to the index to let us know we are on the next step
	} //Iterated through all but the last step
	char **taskWord = currentStep->words; //Get the words for the current step
	backup = malloc(sizeof(char)*(3+strlen(taskWord[0])) ); //We need space for ./ and potentially an /0
	util = malloc(sizeof(char)*(6+strlen(taskWord[0])) ); //We need space for util/ and potentially an /0
   	strcpy(backup, "./"); //execute should start looking in util initially; but ./ feels better first
	strcat(backup,taskWord[0]); //Append the rest of the task
   	strcpy(util, "util/"); //execute should start looking in util initially
	strcat(util,taskWord[0]); //Append the rest of the task
	if(inputPipe != 0){ //If we have something from input(AKA it isn't just currentStep), we should use it
		dup2(inputPipe, STDIN_FILENO); //Read the input from the inputPipe
		if(task->output_file!=NULL){ //If the output file exists, we want to use it
			int fw; //Create an int to store the variable
			if((fw=open(task->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666))<0) return -1; //Try to open the file
			dup2(fw, STDOUT_FILENO); //Use the file as an output
		} //We are using the file as stdout, otherwise it is just default behavior
	} //Used all the input
	int status = 0; //Store the status
	if(fork()==0){ //fork to run the last thing so we can return status
		if((status = execvp(util, currentStep->words))<0){ //If util fails
			if((status = execvp(backup,currentStep->words))<0){ //Try default
				exit(EXIT_FAILURE); //Both fail=>recipe fail because we cant do this task
			} //listen dik
		} //its just a bndaid fix
	} //heheyer y
	wait(NULL); //wait for fork to finsih to continue
	free(backup); //We no longer need the pointer to the task
	free(util); //We no longer need the pointer to the task
	return status; //Get the status
} //comment cool :)