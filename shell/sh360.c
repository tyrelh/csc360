/*
 * sh360 Shell
 * 
 * Tyrel Hiebert
 * 
 * Assignment 1
 * CSC 360 Summer 2018
 * 
 * This program will run commands with up to 7 arguments passed to it
 * Requires a file ".sh360rc" for path locations.
 * Can pipe i/o using 1 or 2 pipes using the syntax "PP cmd1 <args> -> cmd2 <args>".
 * Can redirect output to a file using the syntax "OR cmd <args> -> file".
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

/* CONSTANTS */
#define MAX_INPUT_LINE 80
#define MAX_PATH_DIRS 10
#define MAX_NUM_ARGS 8
#define MAX_LENGTH_PROMPT 10
#define MAX_NUM_TOKENS 27

/* GLOBAL  */
char path_dirs[MAX_PATH_DIRS][MAX_INPUT_LINE];// = {NULL};
char prompt[MAX_LENGTH_PROMPT];
char tokens[MAX_NUM_TOKENS][MAX_INPUT_LINE];// = {NULL};
int num_path_dirs = 0;
int num_input_tokens = 0;
int num_pipes = 0;


/*
 * Adapted from the supplied appendix_f.c file
 * Will return 0 if command not found, or not executable
 * Will return 1 if command is found, and is executable
 */
int checkForFile(char *filename) {
    struct stat file_stat;
    if (stat(filename, &file_stat) != 0) return 0; // file does not exist
    else {
        // check if file is readable/executable
        if (file_stat.st_mode & S_IROTH && file_stat.st_mode & S_IXOTH) return 1;
        return 0;
    }
    return 0;
}


/*
 * Adapted from the supplied appendix_f.c file
 * Will return 0 if command not found, or not executable
 * Will return 1 if command is found, and is executable
 */
int checkForCmd(char *cmd) {
    char full_cmd_path[MAX_INPUT_LINE];
    int i;
    for (i = 0; i < num_path_dirs; i++) {
        // ensure memory of new path string is clear
        memset(full_cmd_path,0,strlen(full_cmd_path));
        // assemble new path string
        snprintf(full_cmd_path, MAX_INPUT_LINE, "%s/%s", path_dirs[i], cmd);
        // check if file exists and is executable
        if (checkForFile(full_cmd_path)) {return i;}
    }
    return -1;
}


/*
 * Run two child threads and pipe output from child(head) to input of child(tail).
 * Adapted from appendix_d.c provided at
 * linux.csc.uvic.ca:\home\zastre\csc360\a1
 */
void PP1Pipe() {
    // index 0 must be "PP" to have gotten here, so start at index 1
    int cmd_head_start = 1;
    int cmd_head_end = 1;
    int cmd_tail_start = 1;
    int cmd_tail_end = num_input_tokens - 1;
    int cursor = 1;
    // find indicies for each command
    for (;;cursor++) {
        if (!strcmp(tokens[cursor], "->")) {
            cmd_head_end = cursor - 1;
            cmd_tail_start = cursor + 1;
            break;
        }
    }
    // build head args
    char *cmd_head[] = {0, 0, 0, 0, 0, 0, 0, 0};
    int k, l; 
    for (k = cmd_head_start, l = 0; k <= cmd_head_end; k++, l++) {
        cmd_head[l] = tokens[k];
    }
    // build tail args
    char *cmd_tail[] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (k = cmd_tail_start, l = 0; k <= cmd_tail_end; k++, l++) {
        cmd_tail[l] = tokens[k];
    }
    // find path to cmd_head and build path string
    int loc_head = checkForCmd(cmd_head[0]);
    char cmd_head_path[MAX_INPUT_LINE];
    if (loc_head < 0) {
        fprintf(stderr, "\"%s\" not found.\n", cmd_head[0]);
        fflush(stderr);
        return;
    } else {
        snprintf(cmd_head_path, MAX_INPUT_LINE, "%s/%s", path_dirs[loc_head], cmd_head[0]);
        cmd_head[0] = cmd_head_path;
    }
    // find path to cmd_tail and build path string
    int loc_tail = checkForCmd(cmd_tail[0]);
    char cmd_tail_path[MAX_INPUT_LINE];
    if (loc_tail < 0) {
        fprintf(stderr, "\"%s\" not found.\n", cmd_tail[0]);
        fflush(stderr);
        return;
    } else {
        snprintf(cmd_tail_path, MAX_INPUT_LINE, "%s/%s", path_dirs[loc_tail], cmd_tail[0]);
        cmd_tail[0] = cmd_tail_path;
    }
    // now that commands are setup, begin pipe and threads
    char *envp[] = {0};
    int status;
    int pid_head, pid_tail; // process identifiers
    int fd[2]; // file descriptors
    // start pipe
    pipe(fd);
    // thread 0 for cmd_head
    if ((pid_head = fork()) == 0) {
        dup2(fd[1], 1);
        close(fd[0]);
        execve(cmd_head[0], cmd_head, envp);
        // something went wrong if execve returns here
        fprintf(stderr, "Something went wrong.\n");
        fflush(stderr);
        close(fd[1]);// tidy up for clean exit
        exit(1);
    }
    // thread 1 for cmd_tail
    if ((pid_tail = fork()) == 0) {
        dup2(fd[0], 0);
        close(fd[1]);
        execve(cmd_tail[0], cmd_tail, envp);
        // something went wrong if execve returns here
        fprintf(stderr, "Something went wrong.\n");
        fflush(stderr);
        close(fd[0]);// tidy up for clean exit
        exit(1);
    }
    // parent thread business
    close(fd[0]);
    close(fd[1]);
    waitpid(pid_head, &status, 0);
    waitpid(pid_tail, &status, 0);
}


/*
 * Run three child threads and pipe output from child(head) to input of child(body),
 * then pipe output of child(body) to input of child(tail).
 * Adapted from appendix_d.c provided at
 * linux.csc.uvic.ca:\home\zastre\csc360\a1
 */
void PP2Pipes() {
    // index 0 must be "PP" to have gotten here, so start at index 1
    int cmd_head_start = 1;
    int cmd_head_end = 1;
    int cmd_body_start = 1;
    int cmd_body_end = 1;
    int cmd_tail_start = 1;
    int cmd_tail_end = num_input_tokens - 1;
    int cursor = 1;
    // find indicies for each command
    for (;;cursor++) {
        if (!strcmp(tokens[cursor], "->")) {
            cmd_head_end = cursor - 1;
            cmd_body_start = cursor + 1;
            break;
        }
    }
    cursor++;
    for (;;cursor++) {
        if (!strcmp(tokens[cursor], "->")) {
            cmd_body_end = cursor - 1;
            cmd_tail_start = cursor + 1;
            break;
        }
    }
    // build cmd_head args
    char *cmd_head[] = {0, 0, 0, 0, 0, 0, 0, 0};
    int k, l; 
    for (k = cmd_head_start, l = 0; k <= cmd_head_end; k++, l++) {
        cmd_head[l] = tokens[k];
    }
    // build cmd_body args
    char *cmd_body[] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (k = cmd_body_start, l = 0; k <= cmd_body_end; k++, l++) {
        cmd_body[l] = tokens[k];
    }
    // build cmd_tail args
    char *cmd_tail[] = {0, 0, 0, 0, 0, 0, 0, 0};
    for (k = cmd_tail_start, l = 0; k <= cmd_tail_end; k++, l++) {
        cmd_tail[l] = tokens[k];
    }
    // find path to cmd_head and build path string
    int loc_head = checkForCmd(cmd_head[0]);
    char cmd_head_path[MAX_INPUT_LINE];
    if (loc_head < 0) {
        fprintf(stderr, "\"%s\" not found.\n", cmd_head[0]);
        fflush(stderr);
        return;
    } else {
        snprintf(cmd_head_path, MAX_INPUT_LINE, "%s/%s", path_dirs[loc_head], cmd_head[0]);
        cmd_head[0] = cmd_head_path;
    }
    // find path to cmd_body and build path string
    int loc_body = checkForCmd(cmd_body[0]);
    char cmd_body_path[MAX_INPUT_LINE];
    if (loc_body < 0) {
        fprintf(stderr, "\"%s\" not found.\n", cmd_body[0]);
        fflush(stderr);
        return;
    } else {
        snprintf(cmd_body_path, MAX_INPUT_LINE, "%s/%s", path_dirs[loc_body], cmd_body[0]);
        cmd_body[0] = cmd_body_path;
    }
    // find path to cmd_tail and build path string
    int loc_tail = checkForCmd(cmd_tail[0]);
    char cmd_tail_path[MAX_INPUT_LINE];
    if (loc_tail < 0) {
        fprintf(stderr, "\"%s\" not found.\n", cmd_tail[0]);
        fflush(stderr);
        return;
    } else {
        snprintf(cmd_tail_path, MAX_INPUT_LINE, "%s/%s", path_dirs[loc_tail], cmd_tail[0]);
        cmd_tail[0] = cmd_tail_path;
    }
    // now that commands are setup, begin pipes and threads
    char *envp[] = {0};
    int status;
    int pid_head, pid_body, pid_tail;
    int fd_1[2]; // file descriptor 1
    int fd_2[2]; // file descriptor 2
    // start pipes
    pipe(fd_1);
    pipe(fd_2);
    // thread 0 for cmd_head
    if ((pid_head = fork()) == 0) {
        dup2(fd_1[1], 1); // connect stdout to pipe_1
        close(fd_1[0]);
        close(fd_2[1]);
        close(fd_2[0]);
        execve(cmd_head[0], cmd_head, envp);
        // something went wrong if execve returns here
        fprintf(stderr, "Something went wrong.\n");
        fflush(stderr);
        close(fd_1[1]);// tidy up for clean exit
        exit(1);
    }
    // thread 1 for cmd_body
    if ((pid_body = fork()) == 0) {
        close(fd_1[1]);
        dup2(fd_1[0], 0); // connect stdin to pipe_1
        dup2(fd_2[1], 1); // connect stdout to p1pe_2
        close(fd_2[0]);
        execve(cmd_body[0], cmd_body, envp);
        // something went wrong if execve returns here
        fprintf(stderr, "Something went wrong.\n");
        fflush(stderr);
        close(fd_1[0]);// tidy up for clean exit
        close(fd_2[1]);
        exit(1);
    }
    // thread 2 for cmd_tail
    if ((pid_tail = fork()) == 0) {
        close(fd_1[0]);
        close(fd_1[1]);
        close(fd_2[1]);
        dup2(fd_2[0], 0); // connect stdin to pipe_2
        execve(cmd_tail[0], cmd_tail, envp);
        // something went wrong if execve returns here
        fprintf(stderr, "Something went wrong.\n");
        fflush(stderr);
        close(fd_2[0]);// tidy up for clean exit
        exit(1);
    }
    // parent thread business
    close(fd_1[0]);
    close(fd_1[1]);
    close(fd_2[0]);
    close(fd_2[1]);
    waitpid(pid_head, &status, 0);
    waitpid(pid_body, &status, 0);
    waitpid(pid_tail, &status, 0);
}


/*
 * Will direct the output of command to the given file
 * Adapted from appendix_c.c provided at
 * linux.csc.uvic.ca:\home\zastre\csc360\a1
 */
void startORLogic() {
    // check if supported amount of redirection is used
    if (num_pipes == 0 || num_pipes > 1) {
        fprintf(stderr, "Your command is malformed. Please us the format \"OR cmd <args> -> file\".\n");
        fflush(stderr);
        return;
    }
    int cmd_start = 1;
    int cmd_end = 1;
    int output_index = 1;
    int cursor = 1;
    // find indicies for command
    for (;;cursor++) {
        if (!strcmp(tokens[cursor], "->")) {
            cmd_end = cursor - 1;
            output_index = cursor + 1;
            break;
        }
    }
    // build cmd args
    char *cmd_args[MAX_NUM_ARGS] = {0};
    int k, l; 
    for (k = cmd_start, l = 0; k <= cmd_end; k++, l++) {
        cmd_args[l] = tokens[k];
    }
    // find path to cmd and build path string
    int loc_cmd = checkForCmd(cmd_args[0]);
    char cmd_path[MAX_INPUT_LINE];
    if (loc_cmd < 0) {
        fprintf(stderr, "\"%s\" not found.\n", cmd_args[0]);
        fflush(stderr);
        return;
    } else {
        snprintf(cmd_path, MAX_INPUT_LINE, "%s/%s", path_dirs[loc_cmd], cmd_args[0]);
        cmd_args[0] = cmd_path;
    }
    // now that command is set up, execute in thread
    char *envp[] = { 0 };
    int pid, fd;
    int status;
    if ((pid = fork()) == 0) {
        // open output file for writing
        fd = open(tokens[output_index], O_CREAT|O_RDWR, S_IRUSR|S_IWUSR);
        if (fd == -1) {
            // exit on error opening file
            fprintf(stderr, "Cannot open \"%s\" for writing.\n", tokens[output_index]);
            fflush(stderr);
            exit(1);
        }
        dup2(fd, 1); // connect stdout to file
        dup2(fd, 2); // connect stderr to file
        execve(cmd_args[0], cmd_args, envp);
        // something went wrong if execve returns here
        fprintf(stderr, "Something went wrong.\n");
        fflush(stderr);
        exit(1);
    }
    // parent thread
    waitpid(pid, &status, 0);
}


/*
 * Run a single command with up to 7 arguments.
 * Adapted from appendix_b.c provided at
 * linux.csc.uvic.ca:\home\zastre\csc360\a1
 */
void runSingleCmd() {
    // if there is a pipe token then command is malformed
    if (num_pipes > 0) {
        fprintf(stderr, "Please enter \"OR\" to redirect output to a file, or \"PP\" to redirect output to another command.\n");
        fflush(stderr);
        return;
    }
    // build args array
    char *args[MAX_NUM_ARGS] = {0};
    int k; 
    for (k = 0; k < num_input_tokens; k++) {
        args[k] = tokens[k];
    }
    char *envp[] = {0};
    int pid, status, i, found_cmd;
    // find path to cmd and build path string
    int loc_cmd = checkForCmd(args[0]);
    char full_cmd_path[MAX_INPUT_LINE];
    if (loc_cmd < 0) {
        fprintf(stderr, "\"%s\" not found.\n", args[0]);
        fflush(stderr);
        return;
    } else {
        snprintf(full_cmd_path, MAX_INPUT_LINE, "%s/%s", path_dirs[loc_cmd], args[0]);
        args[0] = full_cmd_path;
    }
    // fork and execute command
    if ((pid = fork()) == 0) {
        execve(full_cmd_path, args, envp);
        // something went wrong if execve returns here
        fprintf(stderr, "Something went wrong.\n");
        fflush(stderr);
        exit(1);
    }
    // parent thread
    waitpid(pid, &status, 0);
}

/*
 * Determine if 1 or 2 pipes are needed.
 */
void startPPLogic() {
    // one pipe
    if (num_pipes == 1) {PP1Pipe();}
    // two pipes
    else if (num_pipes == 2) {PP2Pipes();}
    // error
    else {
        fprintf(stderr, "Your command is malformed. Please us the format \"PP cmd1 <args> -> cmd2 <args>\". Up 1 or 2 pipes are supported.\n");
        fflush(stderr);
    }
}


/*
 * Tokenize the input string an place tokens into global tokens[]
 * Adapted from appendix_e.c provided at
 * linux.csc.uvic.ca:\home\zastre\csc360\a1
 */
void tokenizeInputString(char *input) {
    char *t;
    int i;
    int line_len;
    num_input_tokens = 0; // reset num
    // copy input string so original string is unaffected by strtok()
    char line_copy[MAX_INPUT_LINE];
    strncpy(line_copy, input, MAX_INPUT_LINE);
    // start tokenizing line
    t = strtok(line_copy, " ");
    // continue tokenizing until end of line (strtok returns NULL)
    while(t != NULL && num_input_tokens < MAX_NUM_TOKENS) {
        strncpy(tokens[num_input_tokens], t, MAX_INPUT_LINE);
        num_input_tokens++;
        t = strtok(NULL, " ");
    }
}


/*
 * Input loop
 * adapted from appendix_a.c provided at
 * linux.csc.uvic.ca:\home\zastre\csc360\a1
 */
void inputPrompt() {
    char input[MAX_INPUT_LINE];
    int line_len;
    // run input loop until "exit" command received
    for (;;) {
        // print prompt and wait for user input
        fprintf(stdout, "%s", prompt);
        fflush(stdout);
        fgets(input, MAX_INPUT_LINE, stdin);
        if (input[strlen(input) - 1] == '\n') input[strlen(input) - 1] = '\0';
        // quit program on "exit"
        if (!strcmp(input, "exit")) {exit(0);}
        // tokenize input and store in tokens[]
        tokenizeInputString(input);
        // count number of pipes
        int i;
        num_pipes = 0;
        for (i = 0; i < num_input_tokens; i++) {
            if (!strcmp(tokens[i], "->")) num_pipes++;}
        // check if any commands were input
        if (num_input_tokens == 0) {
            fprintf(stderr, "Please enter a valid command.\n");
            fflush(stderr);
        }
        // check for OR command
        else if (!strcmp(tokens[0], "OR") || !strcmp(tokens[0], "or")) {
            startORLogic();
        }
        // check for PP command
        else if (!strcmp(tokens[0], "PP") || !strcmp(tokens[0], "pp")) {
            startPPLogic();
        }
        // otherwise just run single command
        else {
            runSingleCmd();
        }
    }
}


/*
 * Load and store data from the .sh360rc file
 */
void getMetaData() {
    FILE *fp;
    char buffer[MAX_INPUT_LINE];
    char *filename = ".sh360rc";
    // open file containing metadata
    fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Could not open %s", filename);
        exit(1);
    }
    // get prompt string from file
    fgets(prompt, MAX_INPUT_LINE, fp);
    if (prompt[strlen(prompt) - 1] == '\n') prompt[strlen(prompt) - 1] = '\0';
    // store given path directories
    while (fgets(buffer, MAX_INPUT_LINE, fp) != NULL && num_path_dirs < MAX_PATH_DIRS) {
        if (buffer[strlen(buffer) - 1] == '\n') buffer[strlen(buffer) - 1] = '\0';
        strncpy(path_dirs[num_path_dirs], buffer, MAX_INPUT_LINE);
        num_path_dirs++;
    }
    fclose(fp);
    // also add "./" to the path
    strncpy(path_dirs[num_path_dirs], ".\0", 2);
    num_path_dirs++;
}


int main(int argc, char *argv[]) {
    // get prompt and path data
    getMetaData();
    // start input prompt loop
    inputPrompt();
    // input prompt is looping and has exit logic
    // so return error if ever get here
    return 1;
}