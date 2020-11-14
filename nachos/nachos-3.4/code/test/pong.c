#include "syscall.h"
void main()
{
	int i;
	for (i = 0; i < 100; ++i)
	{
		PrintString("B");
	}
	Halt();
}
