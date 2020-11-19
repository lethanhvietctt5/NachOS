#include "syscall.h"

void main()
{
	int pingID, pongID;
	pingID = Exec("./test/ping");
	pongID = Exec("./test/pong");
}
