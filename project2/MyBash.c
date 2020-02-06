//      *              *         *         *       *   * * * *
//     * *            * *       * *        *       *   *
//    *   *          *   *     *   *       * * * * *   * * * *
//   * * * *        *     *   *     *      *       *   *
//  *       *      *       * *       *     *       *   *
// *         *    *         *         *	   *	   *   *
   

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/queue.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <errno.h>


#define MAX_LINE 100
#define HIST_LENGTH 256
int run = 1;
char *history[HIST_LENGTH] = {0};
int current = 0;
pid_t parent_pid = 0;
int history_cursor = 0;



void add_History(char* str)
{
    if(history[history_cursor]!=NULL){
        free(history[history_cursor]);
        history_cursor = NULL;
    }

    history[history_cursor] = strndup(str, strlen(str));
    history_cursor = (history_cursor+1) % HIST_LENGTH;
}

char* get_History()
{
    char*‌ temp = history[(history_cursor-1)%HIST_LENGTH];
    if(history[(history_cursor-1)%HIST_LENGTH] == NULL){
        return NULL;
    }

    return temp;
}

void string_parse(char* str, int* number_of_comm, char** args[MAX_LINE])
{
    const char* delimiters = "\t\n ";
    int index = 0;
    int t = 0;
    char buffer[64][256] = {{0}};
    char* token = strtok(str, delimiters)

    while(token =! NULL){
        if(*token =! '|'){
            memcpy(buffer[t], token, strlen(token));
            t++;
        } else{

        }
    }
                
    
    
}

void assign_args(char **args[MAX_LINE], int t, char buffer[64][256], int index)
{
    args[index] = calloc(1, sizeof(char**)*(t+1)); 
    int idx = 0;
    for(idx=0; idx < t; idx++){
        int len = strlen(buffer[idx]);
        args[index][idx] = calloc(1, sizeof(char) * (len+1));
        strncpy(args[index][idx], buffer[idx], len);
    }

}


void free_args(char **args[MAX_LINE], int ncommands) {
    int index = 0, jdx = 0;
    for (index = 0; index < ncommands; index++) {
        for(jdx = 0; args[index][jdx] != NULL; jdx++) {
            free(args[index][jdx]);
            args[index][jdx] = NULL;
        }
        free(args[index]);
        args[index] = NULL;
    }
}

/////////////////////////////////////////////////
int main(void)
{
    signal(SIGINT, ctrl_c_handler);
    signal(SIGTERM, term_handler);

    char **args[MAX_LINE];
    parent_pid = getpid();

    while(run){
        int number_of_commands = 0;
        char commands[MAX_LINE] = {0};
        memset(commands, 0, sizeof(commands));
        printf("AMHF> ");
        if(fgets(commands, MAX_LINE, stdin)==NULL)
            run = 0;
        else if(commands[0]=='\n')
            continue;
        else if(strncmp(commands, "exit\n", 5)==0)
            break;
        else if(strlen(commands)==3 && commands[0]=='!' && commands[1]=='!' && commands[2]=='\n'){
            char *history_tmp = getHistory();
            if(history_tmp==NULL){
                printf("History is empty!\n");
                continue;
            }
            else{
                printf("%s", history_tmp);
                strncpy(commands, history_tmp, strlen(history_tmp));
            }
        }
        
        addHistory(commands);
        string_parse(commands, &number_of_commands, args);
        execute(number_of_commands, args, 0, NULL, NULL, NULL, NULL);
        free_args(args, number_of_commands);
    }

    return 0;


}






