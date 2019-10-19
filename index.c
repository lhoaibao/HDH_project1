#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#define MAX_LINE 80
#define CLEAR "\e[1;1H\e[2J"

void clrscr() // Clear the Screen
{
    printf("%s", CLEAR);
}

void shell_read_input(char *buffer)
{
    fgets(buffer, MAX_LINE, stdin);
    int length = strlen(buffer);
    buffer[length - 1] = '\0';
}

int shell_get_command(char *args[], char *input)
{
    char *parsed;
    int index = 0;
    parsed = strtok(input, " ");
    while (parsed != NULL)
    {
        args[index] = parsed;
        index++;
        parsed = strtok(NULL, " ");
    }
    args[index] = NULL;
    return index - 1;
}

int shell_executable(char *args[])
{
    __pid_t pid;
    pid = fork();
    int status;
    if (pid == 0)
    {
        if (execvp(args[0], args) == -1)
        {
            printf("%s: command not found\n", args[0]);
        }
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("could not fork");
    }
    return status;
}

void set_history(char *history, char *inputBuffer)
{
    strcpy(history, inputBuffer);
}

int redirect_check(char *input)
{
    char *out = strstr(input, ">");
    char *in = strstr(input, "<");
    if ((out != NULL) && (in != NULL))
    {
        //both inut and output redirection
        return 3;
    }
    else if (out != NULL)
    {
        //output redirection only
        return 2;
    }
    else if (in != NULL)
    {
        //input redirection only
        return 1;
    }
    return 0;
}

int check_ampersand_end(char *args[], int n)
{
    if (strcmp(args[n], "&") == 0)
    {
        args[n] = NULL;
        return n - 1;
    }
}

int redirect(char *args[], int state)
{
    char *command[MAX_LINE / 2 + 1];
    char *file;
    int pos = 0, ret;
    int fd;
    __pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        if (state == 2)
        {
            while (args[pos] != NULL)
            {
                if (strcmp(args[pos], ">") == 0)
                {
                    break;
                }
                command[pos] = args[pos];
                pos++;
            }
            command[pos] = NULL;
            file = args[pos + 1];
            remove(file);
            fd = open(file, O_CREAT | O_WRONLY, 0777);
            ret = dup2(fd, 1);
            execvp(command[0], command);
            close(1);
            close(fd);
        }

        if (state == 1)
        {
            while (args[pos] != NULL)
            {
                if (strcmp(args[pos], "<") == 0)
                {
                    break;
                }
                command[pos] = args[pos];
                pos++;
            }
            command[pos] = NULL;
            file = args[pos + 1];
            fd = open(file, O_RDONLY);
            ret = dup2(fd, 0);
            close(fd);
            execvp(command[0], command);
            close(0);
        }
        if (state == 3)
        {
            printf("we dont develop this feature\n");
        }
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("could not fork");
    }
    return 0;
}

// function check if command have | or not
int check_pipe(char *input)
{
    char *pipe = strstr(input, "|");
    if (pipe != NULL)
    {
        return 1;
    }
    return 0;
}

void pipe_feature(char *args[], int n)
{
    int pipefd[2], status;
    __pid_t pid, pid1;
    char *command1[20];
    char *command2[20];
    int index = 0, i = 0;
    while (1)
    {
        if (strcmp(args[index], "|") == 0)
        {
            break;
        }
        command1[i] = args[index];
        index++;
        i++;
    }
    command1[i] = NULL;
    index++;
    i = 0;
    while (index <= n)
    {
        command2[i] = args[index];
        index++;
        i++;
    }
    command2[i] = NULL;
    pid = fork();
    if (pid == 0)
    {
        pipe(pipefd);
        pid1 = fork();
        if (pid1 == 0)
        {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
            if (execvp(command1[0], command1) == -1)
            {
                exit(1);
            }
        }
        else if (pid1 > 0)
        {
            wait(NULL);
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);

            close(pipefd[0]);
            if (execvp(command2[0], command2) == -1)
            {
                exit(1);
            }
        }
        else
        {
            perror("could not fork");
        }
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("could not fork");
    }
}

// cd command in terminal
void build_in_cd(char *args[])
{
    if (args[2] != NULL)
    {
        printf("cd: too many arguments\n");
        return;
    }
    if (chdir(args[1]) != -1)
    {
        return;
    }
    printf("cd: %s: No such file or directory\n", args[1]);
}

// pwd command in terminal
void build_in_pwd(char *args[])
{
    char cwd[100];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    return;
}

// history feature
void history_feature(char *inputBuffer, char *history)
{
    if (strcmp(inputBuffer, "!!") == 0)
    {
        if (strlen(history) == 0)
        {
            printf("history is empty\n");
            return;
        }
        strcpy(inputBuffer, history);
        printf("%s\n", inputBuffer);
    }
}

void shell_loop()
{
    // clear screen for beauty ^^
    clrscr();

    char inputBuffer[MAX_LINE];
    char *args[MAX_LINE / 2 + 1];
    int should_run = 1;
    char history[MAX_LINE];
    int status_redirect = 0, execute_status, status_pipe = 0;
    do
    {
        // check posible feature
        execute_status = 0;

        printf("osh> ");
        shell_read_input(inputBuffer);
        status_redirect = redirect_check(inputBuffer);
        status_pipe = check_pipe(inputBuffer);

        // check and using history after that set a new history
        history_feature(inputBuffer, history);
        set_history(history, inputBuffer);

        // split input into list command
        int n = shell_get_command(args, inputBuffer);
        n = check_ampersand_end(args, n);

        if (strcmp(args[0], "exit") == 0)
        {
            should_run = 0;
            continue;
        }

        if (strcmp(args[0], "pwd") == 0)
        {
            build_in_pwd(args);
            continue;
        }

        if (strcmp(args[0], "cd") == 0)
        {
            build_in_cd(args);
            continue;
        }

        if (status_pipe)
        {
            pipe_feature(args, n);
            continue;
        }

        if (status_redirect)
        {  
            redirect(args, status_redirect);
            continue;
        }
        shell_executable(args);
    } while (should_run);
}

int main()
{
    shell_loop();
    return 0;
}