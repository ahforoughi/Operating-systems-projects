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

#define MAX_LINE 80 * 2
#define HLENGTH 256
int should_run = 1;
char *hist[HLENGTH] = {0};
int cur = 0;
pid_t parent_pid = 0;

void string_parse(char *strs, int *ncommands, char **args[MAX_LINE])
{
    const char *DELIM = "\t\r\n\a ";
    int index = 0, t = 0;
    char buffer[64][256] = {{0}};
    char *token = strtok(strs, DELIM);
    while (token != NULL) {
        if (*token != '|') {
            memcpy(buffer[t], token, strlen(token));
            t+=1;
        } else {
            assign_args(args, t, buffer, index);
            index += 1;
            memset(buffer, 0, sizeof(buffer));
            t = 0;
        }
        token = strtok(NULL, DELIM);
    }

    assign_args(args, t, buffer, index);
    index+=1;
    *ncommands = index; 
}


void ctrl_c_handler(int sig) {
    int status = 0;
    if (getpid() == parent_pid) {
        kill(-parent_pid, SIGTERM);
        waitpid(parent_pid, &status, WNOHANG);
    } else {
        exit(0);
    }
}

void addHistory(char *strs) {
    if (hist[cur] != NULL) {
        free(hist[cur]);
        hist[cur] = NULL;
    }

    hist[cur] = strndup(strs, strlen(strs));
    cur = (cur + 1) % HLENGTH;
    return;
}

char* getHistory() {
    char *t = hist[(cur-1)%HLENGTH];  
    if (hist[(cur-1)%HLENGTH] == NULL) {
        return NULL;
    } 
    return t;
}
void assign_args(char **args[MAX_LINE], int t, char buffer[64][256], int index) {
    args[index] = calloc(1, sizeof(char**)*(t+1)); 
    int idx = 0;
    for (idx=0; idx < t; idx++) {
        int len = strlen(buffer[idx]);
        args[index][idx] = calloc(1, sizeof(char) * (len+1));
        strncpy(args[index][idx], buffer[idx], len);
    }
}



void parse_sub_command(char **args[MAX_LINE], int index, int *out_to_file, int *in_to_file, int *eout_to_file, 
                       int **ifile, int **ofile, int **efile, int *out_combine, int *err_combine, 
                       int *back_ground, int *internal_command) {
    int t = 0, i_append = false, o_append = false;
    while (args[index][t] != NULL) {
        char *token = args[index][t];
        if (t == 0 && (strncmp(token, "cd", 2) == 0 || strncmp(token, "fg", 2) == 0)){
            *internal_command = 1;
        }
        if (strlen(token) == 1 && token[0] == '&'){
            *back_ground = true;
        }
        if (*in_to_file == true && *ifile == NULL){
            *ifile = (int*)calloc(1, sizeof(int));
            **ifile = open(token, O_RDONLY);
            free(args[index][t]);
            token = args[index][t] = NULL;
        }
        if (*out_to_file == true && *ofile == NULL){
            *ofile = (int*)calloc(1, sizeof(int));
            if (o_append) {
                **ofile = open(token, O_WRONLY | O_APPEND, 0666);
            } else {
                **ofile = open(token, O_CREAT | O_WRONLY, 0666);
            }
            o_append= false;
            free(args[index][t]);
            token = args[index][t] = NULL;
        }
        if (*eout_to_file == true && *efile == NULL) {
            *efile = (int*)calloc(1, sizeof(int));
            if (o_append) {
                **efile = open(token, O_WRONLY | O_APPEND, 0666);
            } else {
                **efile = open(token, O_CREAT | O_WRONLY, 0666);
            }
            o_append = false;
            free(args[index][t]);
            token = args[index][t] = NULL;
        }
        if (token && (token[0] == '<' || token[0] == '>' || 
            (strlen(token) >= 2 && strlen(token) <=3 && 
             token[0] == '2' && token[1] == '>'))) {
            *in_to_file = *token == '<';
            *out_to_file = *token == '>';
            i_append = strlen(token) == 2 && token[1] == '<';
            o_append = strlen(token) == 2 && token[0] =='>' && token[1] == '>';
            *eout_to_file = (strlen(token) == 2 && token[0] == '2' && token[1] == '>'); 
            *eout_to_file |= (strlen(token) == 3 && token[0] == '2' && token[1] == '>' && token[2] == '>'); 
            o_append = o_append | (*eout_to_file && 
                    (strlen(token) == 3 && token[0] == '2' && token[1] == '>' && token[2] == '>'));   
            free(args[index][t]);
            token = args[index][t] = NULL;
        }
        if (token && strlen(token) == 4 && strncmp(token, "2>&1", 4) == 0) {
            *out_combine = true;
            free(args[index][t]);
            token = args[index][t] = NULL;
        }
        if (token && strlen(token) == 4 && strncmp(token, "1>&2", 4) == 0) {
            *err_combine = true;
            free(args[index][t]);
            token = args[index][t] = NULL;
        }
        t++;
    }
}

void execute_internal(int ncommands, char **args[MAX_LINE], int index) {
    if (strncmp(args[index][0], "cd", 2) == 0) {
        chdir(args[index][1]);
    }
    if (strncmp(args[index][0], "fg", 2) == 0) {
    }
    return;
}

void execute(int ncommands, char **args[MAX_LINE], int index, int *pfd, 
             int *ofile, int *ifile, int *efile) {
    if (ncommands == index) return;
    int fd[2], status = 0, childpid;
    int out_to_file = false, in_to_file = false, eout_to_file = false;
    int out_combine = false, err_combine = false, back_ground = false, internal_command=false;

    parse_sub_command(args, index, &out_to_file, &in_to_file, &eout_to_file, 
                      &ifile, &ofile, &efile, &out_combine, 
                      &err_combine, &back_ground, &internal_command);
    if (internal_command) {
        execute_internal(ncommands, args, index);
        return;
    }
    if (pipe(fd) == -1) {
        exit(errno);
    }
    pid_t pid = fork();
    switch(pid) {
        case -1:
            break;
        case 0:
            if (ifile != NULL) {
                while((dup2(*ifile, STDIN_FILENO) == -1) && errno == EINTR) {}
                close(*ifile);
                ifile = NULL;
            }
            if (pfd != NULL) {
                if (ifile == NULL) {
                    while((dup2(pfd[0], STDIN_FILENO) == -1) && errno == EINTR) {}
                }
                close(pfd[0]);
                close(pfd[1]);
            }

            if (efile != NULL) {
                while((dup2(*efile, STDERR_FILENO) == -1) && errno == EINTR) {}
                if (err_combine) {
                    while((dup2(*efile, STDOUT_FILENO) == -1) && errno == EINTR) {}
                }
                close(*efile);
                efile = NULL;
            } if (index + 1 != ncommands) {
                while((dup2(fd[1], STDERR_FILENO) == -1) && errno == EINTR) {}
            }
            if (ofile != NULL) {
                while((dup2(*ofile, STDOUT_FILENO) == -1) && errno == EINTR) {}
                if (out_combine) {
                    while((dup2(*ofile, STDERR_FILENO) == -1) && errno == EINTR) {}
                }
                close(*ofile);
                ofile = NULL;
            } if (index + 1 != ncommands) {
                while((dup2(fd[1], STDOUT_FILENO) == -1) && errno == EINTR) {}
            }
            close(fd[0]);
            close(fd[1]);
            if (execvp(args[index][0], args[index]) == -1) {
                perror(strerror(errno));
                exit(errno);
            }
        default:
            if (pfd) {
                close(pfd[0]);
                close(pfd[1]);
            }
            if (ifile) {
                close(*ifile);
                free(ifile);
                ifile = NULL;
            }
            if (ofile) {
                close(*ofile);
                free(ofile);
                ofile = NULL;
            }
            if (efile) {
                close(*efile);
                free(efile);
                efile = NULL;
            }
            execute(ncommands, args, index+1, fd, ofile, ifile, efile);
            close(fd[0]);
            close(fd[1]);
            if (back_ground == true) {
                childpid = waitpid(pid, &status, WNOHANG);
            } else {
                childpid = waitpid(pid, &status, 0);//wait(&pid);
            }
    }
}

void term_handler(int sig){
    if (getpid() != parent_pid) {
        exit(0);
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

int main(void)
{
    signal(SIGINT, ctrl_c_handler);
    signal(SIGTERM, term_handler);


    char **args[MAX_LINE];
    parent_pid = getpid();
    while (should_run) {
        int ncommands = 0;
        char strs[MAX_LINE] = {0};
        memset(args, 0, sizeof(args));
        printf("AMHF# ");
        if (fgets(strs, MAX_LINE, stdin)==NULL) should_run=0;
        if (strs[0] == '\n') continue;
        if (strncmp(strs, "exit\n", 5)==0) break;
        if (strlen(strs) == 3 && 
            strs[0] == '!' && strs[1] == '!' && strs[2] == '\n') {
            char *t = getHistory();
            if (t == NULL) {
                printf("No History available\n");
                continue;
            } else {
                printf("%s", t);
                strncpy(strs, t, strlen(t)); //i dont wnna do this command in my project!!
            }
        }

        if (strncmp(strs, "history\n", 8)==0){
            char* tmp;
            int flag = 1;
            for (int i = 0; tmp = hist[i]; i++){
                printf("%s", tmp);
                flag = 0;
            }
            if(flag==1)
                printf("history is empty\n");
            continue;
        }


        addHistory(strs);
        string_parse(strs, &ncommands, args);
        execute(ncommands, args, 0, NULL, NULL, NULL, NULL);
        free_args(args, ncommands);
    }
    return 0;
}