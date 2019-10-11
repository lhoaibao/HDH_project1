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

//Ham thay the cua strsep ben Linux
//Gia tri tra ve: Chuoi ki tu duoc chia
//stringp: Chuoi dau vao
//delim: cac ki tu de phan biet chuoi can chia
char* mystrsep(char** stringp, const char* delim)
{
	char* start = *stringp;
	char* p;

	p = (start != NULL) ? strpbrk(start, delim) : NULL;

	if (p == NULL)
	{
		*stringp = NULL;
	}
	else
	{
		*p = '\0';
		*stringp = p + 1;
	}

	return start;
}

//Ham chia chuoi inputBuffer thanh cac chuoi trong mang args
//Gia tri tra ve: Khong co
//inputBuffer: chuoi dau tien nhan vao
//args: mang cac chuoi se duoc chia ra
void divide_args(char *inputBuffer, char **args)
{
	char *found;
	int i;
	i = 0;

	//Su dung found de lay tung chuoi ki tu tu chuoi inputBuffer va sau do gan vao args 
	while ((found = mystrsep(&inputBuffer, " ")) != NULL) //mystrsep co the thay the bang strsep khi su dung linux voi cac doi so van giu nguyen
	{
		args[i] = found;
		i++;
	}
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
		divide_args(inputBuffer, args);
    }
}

int main()
{
    loop();
    return 0;
}