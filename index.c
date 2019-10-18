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

void shell_get_command(char *args[], char *input)
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
}

int shell_executable(char *args[])
{
    __pid_t pid;
    pid = fork();
    int status;
    if (pid == 0)
    {
        status = execvp(args[0], args);
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
    char *out = strpbrk(input, ">");
    char *in = strpbrk(input, "<");
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

int check_pipe(char *input)
{
    char *pipe = strpbrk(input, "|");
    if (pipe != NULL)
    {
        return 1;
    }
    return 0;
}

void pipe_feature(char *args[])
{
    int pipefd[2], status;
    __pid_t pid, pid1;
    char **command1, **command2;
    int index = 0, i = 0;
    while (strcmp(args[index], "|") == 0)
    {
        command1[i] = args[index];
        index++;
        i++;
    }
    command1[i] = NULL;
    index++;
    i = 0;
    while (args[index] != NULL)
    {
        command2[i] = args[index];
        index++;
        i++;
    }
    command2[i] = NULL;
    pipe(pipefd);
    pid1 = fork();
    if (pid1 == 0)
    {
        pid = fork();
        if (pid)
        {
            wait(&pipefd[1]);
            dup2(pipefd[0], 0);
            close(pipefd[0]);
            close(pipefd[1]);
            execvp(command2[0], command2);
        }
        if (pid == 0)
        {
            dup2(pipefd[1], 1);
            close(pipefd[0]);
            close(pipefd[1]);
            execvp(command1[0], command1);
        }
    }
    else
    {
        wait(NULL);
    }
}

void shell_loop()
{
    clrscr();
    char inputBuffer[MAX_LINE];
    char *args[MAX_LINE / 2 + 1];
    int should_run = 1;
    char history[MAX_LINE];
    int status_redirect = 0, execute_status = 0, status_pipe = 0;
    do
    {
        execute_status = 0;
        printf("osh> ");
        shell_read_input(inputBuffer);
        if (strcmp(inputBuffer, "!!") == 0)
        {
            if (strlen(history) == 0)
            {
                printf("history is empty\n");
                continue;
            }
            strcpy(inputBuffer, history);
            printf("%s\n", inputBuffer);
        }
        set_history(history, inputBuffer);
        status_redirect = redirect_check(inputBuffer);
        status_pipe = check_pipe(inputBuffer);
        shell_get_command(args, inputBuffer);

        if (strcmp(args[0], "exit") == 0)
        {
            should_run = 0;
            continue;
        }
        else if (status_pipe)
        {
            pipe_feature(args);
        }
        else if (status_redirect)
        {
            redirect(args, status_redirect);
        }
        else
        {
            execute_status = shell_executable(args);
        }

        if (execute_status == -1)
        {
            printf("command not found\n");
        }
    } while (should_run);
}

int main()
{
    shell_loop();
    return 0;
}