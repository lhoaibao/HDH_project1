#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define MAX_LINE 80
#define CLEAR "\e[1;1H\e[2J"

void clrscr() // Clear the Screen
{
    printf("%s", CLEAR);
}

void read_input(char *inputBuffer, int length)
{
    read(STDIN_FILENO, inputBuffer, MAX_LINE);
}

void loop()
{
    clrscr();
    char inputBuffer[MAX_LINE];
    char *args[MAX_LINE / 2 + 1];
    int should_run = 1;
    while (should_run)
    {
        strcpy(inputBuffer, "\0");
        printf("osh> ");
        fflush(stdout);
        read_input(inputBuffer, MAX_LINE);
        fflush(stdin);
    }
}

int main()
{
    loop();
    return 0;
}