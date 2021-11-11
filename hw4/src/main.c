#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <getopt.h> /* getopt */
#include "cook.h" //cook.h is in src because I'm scared include might be replaced in grading

#include "cookbook.h"

int main(int argc, char *argv[]) {
    extern COOKBOOK *cbp; //Declare a pointer to a cookbook; I guess putting is not needed, but for clarity in main
    extern char *cookbook; //We use extern to not have to pass these values
    extern unsigned long long maxCooks; //ok honestly if you overflow this, mb dude
    extern RECIPE *recipe; //Just declaring all globals here(I didn't put them in cook.c since the code works)
    extern NODE *workQueue; //This way, if cook.h isn't included, it's abundantly clear that these aren't syntax errors
    cookbook = "rsrc/cookbook.ckb"; //Set the default value for cookbook
    maxCooks = 1; //Default max cooks should be 1; cookbook, maxCooks and cbp are extern from cook.h
    workQueue = NULL; //The queue should be empty by default
    int err = 0; //Declare a variable for error codes
    FILE *in; //File to store cookbook input
    char *mainRecipe; //Store a string of the main recipe so we can find it within the cookbook later
    int specifiedFile = 0; //If we want to provide a cookbook, we shouldn't try to do more than one
    int specifiedCooks = 0; //If we want to provide a number of cook, we shouldn't try to do it more than once
    int specifiedRecipe = 0; //If we want to specify a recipe, we shouldn't do it more than once
    char ch; //Here so we can determine if the number of cooks is an integer; it just works :)
    int opt; //Create an variable to parse the arguments
    while ((opt = getopt(argc, argv, "-:f:c:")) != -1) { //Keep reading the arguments until we run out
        switch (opt) { //Check the argument with a switch/case
            case 'f': //Set the value of the cookbook file
                if(!specifiedFile){ //If we have not specified the cookbook file, here it is
                    specifiedFile = 1; //Set the flag so we don't try to use different files later
                    cookbook = optarg; //Set the cookbook to our specified file
                    break; //Break
                } //If we specify multiple files, the behavior is undefined so the user should know about the usage
                goto USAGE; //Print the usage since we tried to specify multiple cookbook
            case 'c': //Set the value of maxCooks
                if(!specifiedCooks){ //If we have not specified the number of cooks, here is our number
                    specifiedCooks = 1; //Set the flag so we don't try to use different number of cooks
                    while(optarg[0]=='0') optarg++; //Remove leading 0s since it led to errors
                    if((sscanf(optarg, "%lli%c", &maxCooks, &ch) == 1)) break; //If the argument is invalid, we should print usage
                } //If we specify multiple number of cooks, the behavior is undefined so the user should know about the usage
                goto USAGE; //Print the usage since our argument either wasn't a valid number or was a repeated number
            case 1: //Non-option arguments; should occur once and be the main_recipe_name
                if(!specifiedRecipe){ //If we have not specified a recipe yet, we have found a recipe
                    specifiedRecipe = 1; //Set the flag so we don't specify multiple recipes
                    mainRecipe=optarg; //Store the main recipe
                    break; //Move onto the next argument
                } //If we specify multiple recipes, the behavior is undefined so the user should know about the usage
                goto USAGE; //Extraneous but just in case we wanted to add more cases
            case '?': //Missing arguments
            case ':': //Unknown Options
            default: //Catch any other arguments
            USAGE: //Prints the correct usage and exits the program
                fprintf(stderr, "USAGE: %s [-f cookbook] [-c max_cooks] [main_recipe_name]\n" \
                    "   -f cookbook\n" \
                    "         cookbook should be a valid cookbook we can reach from this directory\n\n" \
                    "   -c max_cooks\n" \
                    "         max_cooks should be the number of cooks we can utilize as a time\n\n" \
                    "   main_recipe_name\n" \
                    "         main_recipe_name should be a valid recipe in our cookbook\n\n" \
                    "Note that while the arguments are optional, extraneous arguments will not be accepted\n"
                    , argv[0]); //Lets the user know how to run the program
                    exit(EXIT_FAILURE); //Exits the program with an error
        } //Check the current option
    } //Finish parsing the arguments
    if((in = fopen(cookbook, "r")) == NULL) { //Can't open cookbook
    	fprintf(stderr, "Can't open cookbook '%s': %s\n", cookbook, strerror(errno)); //Print the message
    	exit(EXIT_FAILURE); //Exits the program with an error
    } //Succesfully opened the cookbook
    cbp = parse_cookbook(in, &err); //Try to parse the book
    if(err) { //Can't parse cookbook
    	fprintf(stderr, "Error parsing cookbook '%s'\n", cookbook); //Print the message
    	exit(EXIT_FAILURE); //Exits the program with an error
    } //Successfully parsed the cookbook
    if(mainRecipe==NULL) mainRecipe = cbp->recipes->name; //Use the first recipe if we didn't declare a main recipe
    if(invalidRecipe(mainRecipe)) exit(EXIT_FAILURE); //If the recipe was invalid exit with failure

/*
printf("%s\n",recipe->name);
printf("%s\n",recipe->next->name);
printf("%s\n",recipe->next->next->name);
printf("%s\n",recipe->next->next->next->name);
printf("%s\n",recipe->next->next->next->next->name);
printf("%s\n",recipe->next->next->next->next->next->name);
printf("%s\n",recipe->next->next->next->next->next->next->name);
printf("%s\n",recipe->next->next->next->next->next->next->next->name);
printf("%s\n",recipe->next->next->next->next->next->next->next->next->name);
printf("%s\n",recipe->next->next->next->next->next->next->next->next->next->name);
printf("%s\n",recipe->next->next->next->next->next->next->next->next->next->next->name);
printf("%s\n",recipe->next->next->next->next->next->next->next->next->next->next->next->name);
printf("%s\n",recipe->next->next->next->next->next->next->next->next->next->next->next->next->name);

    if(addLeaves()){ //The function isn't descriptive, but this means that recipe is a leaf
        //process recipe since it is a leaf now
        printf("we are starting %s\n",recipe->name);
    } //Didn't want to create a recipe_link from recipe
*/
    if(dfs(0,recipe)){ //The function isn't descriptive, but this means that recipe is a leaf
        //process recipe since it is a leaf now
        printf("we are starting %s\n",recipe->name);
    } //Didn't want to create a recipe_link from recipe




//test();
exit(0);
    printf("Main Recipe: %s\n", recipe->name);

//process recipes
//we will use a work queue
    //contains the recipes that are required, are ready to be processed b/c dependencies are cleared and have not been processed yet
    //will initially start as just leaves(don't need to include duplicates)
//main procesing loop goes like
    //if empty and nothing processed->cooking done and we gtfo exit with proper status
    //if nonempty but processed=numCooks->wait with sigsuspend syscall until something finishes
    //if nonempty and processed<numCooks->remove first from queue and calls fork to start cook process

//process recipe(singular)
//carries the task out in order
    //idk man kinda spooky here >:(


unparse_cookbook(cbp, stdout); //unparse the cookbook idk also prints to the screen

    exit(EXIT_SUCCESS); //Ran the program without error
}
