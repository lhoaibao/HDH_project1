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
    if (pid == 0)
    {
        execvp(args[0], args);
    }
    else if (pid > 0)
    {
        wait(NULL);
    }
    else
    {
        perror("could not fork");
    }
    return 1;
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
    char *command[MAX_LINE/2 +1];
    char *file;
    int pos = 0,ret;
    int fd;
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
        fd = open(file, O_CREAT | O_WRONLY,0777);
        ret = dup2(fd,1);
        shell_executable(command);
        close(1);
        close(fd);
    }

    if (state == 1)
    {
        file = args[pos];
        pos+=2;
        while (args[pos] != NULL)
        {
            command[pos] = args[pos];
            pos++;
        }
        fd = open(file, O_RDONLY);
        ret = dup2(fd,0);
        shell_executable(command);
        close(0);
        close(fd);
    }
}

void shell_loop()
{
    clrscr();
    char inputBuffer[MAX_LINE];
    char *args[MAX_LINE / 2 + 1];
    int should_run = 1;
    char history[MAX_LINE];
    int status_redirect = 0;
    do
    {
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
        status_redirect = redirect_check(inputBuffer);
        shell_get_command(args, inputBuffer);
        if (status_redirect)
        {
            redirect(args, status_redirect);
            }
        else
        {
            should_run = shell_executable(args);
        }
        set_history(history, inputBuffer);
    } while (should_run);
}

int main()
{
    shell_loop();
    return 0;
}