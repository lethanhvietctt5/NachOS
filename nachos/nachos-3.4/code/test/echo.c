#include "syscall.h"

#define MAX_LENGTH_INPUT 255
int main()
{
	char input[255];
	PrintString("Nhap 1 chuoi: \n");
	ReadString(input, MAX_LENGTH_INPUT);
	PrintString("Chuoi vua nhap: \n");
	PrintString(input);
	return;
}
