#include <stdlib.h>

#include "client_registry.h"
#include "maze.h"
#include "player.h"
#include "debug.h"
#include "server.h"

#include "csapp.h"
#include <signal.h>
#include <getopt.h> /* getopt */


static void SIGHUP_HANLDER(); //Create a handler for the SIGHUP signal
static void terminate(int status); //Terminate the function with an exit status
static char *default_maze[] = { //Set a default maze that will be used if there is not template_file
  "******************************",
  "***** %%%%%%%%% &&&&&&&&&&& **",
  "***** %%%%%%%%%        $$$$  *",
  "*           $$$$$$ $$$$$$$$$ *",
  "*##########                  *",
  "*########## @@@@@@@@@@@@@@@@@*",
  "*           @@@@@@@@@@@@@@@@@*",
  "******************************",
  NULL
};
static int terminating = 0; //Set a global var to let us if we received a SIGHUP
static int listenfd; //Keep so we can close within our handler

CLIENT_REGISTRY *client_registry;

int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    char *portNumber = NULL; //We will set the port, but it must not be null
    char *templateFile = NULL; //If we do set a template file, we will store it here to parse later
    int pv; //Here so we can determine if the port is an integer; it just works :)
    char ch; //Here so we can determine if the port is an integer; it just works :)
    int opt; //Create an variable to parse the arguments
    while ((opt = getopt(argc, argv, "-:p:t:")) != -1) { //Keep reading the arguments until we run out
        switch (opt) { //Check the argument with a switch/case
            case 'p': //Set the required argument for the port number
                if(portNumber==NULL){ //We don't want to set the port multiple times
                    portNumber=optarg; //Set the port
                    while(optarg[0]=='0') optarg++; //Remove leading 0s since it led to errors
                    if((sscanf(optarg, "%i%c", &pv, &ch) == 1)) break; //If argument is a number, don't print error
                } //This forces the port to be set at most once(we will do a check for at least once later)
                goto USAGE; //Print the usage
            case 't': //Set the optional argument in the form -t <template_file>
                if(templateFile==NULL){ //We have not set a template file yet
                    templateFile=optarg; //Set the template file which we will parse
                } //This forces template file to be set at most once(we don't mind no times)
                break; //Break the current iteration before we print an error
            case 1: //Non-option arguments
            case '?': //Missing arguments
            case ':': //Unknown Options
            default: //Catch any other arguments
            USAGE: //Prints the correct usage and exits the program
                fprintf(stderr,"Arguments Invalid\n"); //Commented out USAGE because it was more than one line
                exit(EXIT_FAILURE); //Exits the program with an error
        } //Check the current option
    } //Finish parsing the arguments
    if(portNumber==NULL) goto USAGE; //We finished parsing the arguments but never set the port
    if(pv>65535 || pv < 0) goto USAGE; //The port value was invalid

//TODO: Parse template file here and set the value of default_maze
printf("\n%s on port %s\n",templateFile,portNumber);

//totalSize = size of array(fseek or similar)
//entrySize = length until first \n(assume uniform length)
//arrSize = totalSize/entrySize(do a minus 1 since we don't want newline)
//we can now malloc(arrSize*sizeof(char*)) //Creates an array of pointers
//  each entry in the array is now a pointer
//iterate through the array and malloc(entrySize*sizeof(char))
    //on each entry, do fgets to insert
        //alternatively, just set it to like strinMax or 500 or w.e
//make sure to free at end if templateFile=/=NULL
//maybe just do like array[500000] and make strings of 5000000 or w.e lmao










    // Perform required initializations of the client_registry, maze, and player modules.
    client_registry = creg_init();
    maze_init(default_maze);
    player_init();
    debug_show_maze = 1;  // Show the maze after each packet.

    // Set up the server socket and enter a loop to accept connections on this socket.
    // For each connection, a thread should be started to run function mzw_client_service().
    // In addition, you should install a SIGHUP handler, so that receipt of SIGHUP will shutdown.

    //Install a signal handler so clean termination can occur
    Signal(SIGHUP, SIGHUP_HANLDER); //Uses sigaction inside of csapp.h

    //Set up server socket and accept connections
    int *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    listenfd = Open_listenfd(portNumber); //declared as global
    while(1) { //stay in the loop until we break using the signal handler(we use 1 here bc terminating is checked)
        clientlen=sizeof(struct sockaddr_storage);
        if((connfdp = malloc(sizeof(int)))==NULL){ //on error, close and exit
            close(listenfd);
            exit(EXIT_FAILURE);
        } //assume no error on accept bc otherwise idk how to tell if listenfd is closed from handler
        if((*connfdp = accept(listenfd, (SA *) &clientaddr, &clientlen))<0) close(listenfd); //hope that no error occur
        if(terminating) break; //terminating checked here because if accpet fails, we can't create
        if((pthread_create(&tid, NULL, mzw_client_service, connfdp))<0){ //on error, close and exit
            close(listenfd);
            exit(EXIT_FAILURE);
        }
    }
//  fprintf(stderr, "You have to finish implementing main() before the MazeWar server will function.\n");

    terminate(EXIT_FAILURE); //If we reach here without a clean exit, assume the worst
}



static void SIGHUP_HANLDER(int signum){ //Set a flag in the function since this is async safe
    terminating = 1; //We are terminating so we want to set the flag
    close(listenfd); //Close the socket
} //Note that we can include this function because it is statics


/*
 * Function called to cleanly shut down the server.
 */
void terminate(int status) {
    // Shutdown all client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    debug("Waiting for service threads to terminate...");
    creg_wait_for_empty(client_registry);
    debug("All service threads terminated.");

    // Finalize modules.
    creg_fini(client_registry);
    player_fini();
    maze_fini();

    debug("MazeWar server terminating");
    exit(status);
}
