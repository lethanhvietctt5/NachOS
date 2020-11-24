#include "syscall.h"

void main()
{
	int pingID, pongID;
	PrintString("");
	pingID = Exec("./test/ping");
	pongID = Exec("./test/pong");
	pingID = Exec("./test/ping");
	pongID = Exec("./test/pong");
	pingID = Exec("./test/ping");
	pongID = Exec("./test/pong");
	pingID = Exec("./test/ping");
	pongID = Exec("./test/pong");
	pingID = Exec("./test/ping");
	pongID = Exec("./test/pong");
}
