#include <stdio.h>
#include <unistd.h>
#include <string.h>
#define MAX_LINE 80

void get_input(char input_buffer[MAX_LINE], char *args)
{
    scanf("%s", input_buffer);
    int command_length = strlen(input_buffer);
}

int main()
{
    char inputBuffer[MAX_LINE];
    char *args[MAX_LINE / 2 + 1];
    int should_run = 1;
    while (should_run)
    {
        printf("osh> ");
        fflush(stdout);
        get_input(inputBuffer, *args);
        fflush(stdin);
    }
    return 0;
}