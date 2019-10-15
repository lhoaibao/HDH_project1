#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
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
        wait(0);
    }
    else
    {
        perror("could not fork");
    }
    return 1;
}

void set_history(char* history, char* inputBuffer){
    strcpy(history,inputBuffer);
}

void shell_loop()
{
    clrscr();
    char inputBuffer[MAX_LINE];
    char *args[MAX_LINE / 2 + 1];
    int should_run=1;
    char history[MAX_LINE];
    do
    {
        printf("osh> ");
        shell_read_input(inputBuffer);
        if (strcmp(inputBuffer, "!!")==0)
        {
            if (strlen(history)==0){
                printf("history is empty\n");
                continue;
            }
            strcpy(inputBuffer, history);
            printf("%s\n", inputBuffer);
        }
        shell_get_command(args, inputBuffer);
        should_run = shell_executable(args);
        set_history(history, inputBuffer);
    } while (should_run);
}

int main()
{
    shell_loop();
    return 0;
}