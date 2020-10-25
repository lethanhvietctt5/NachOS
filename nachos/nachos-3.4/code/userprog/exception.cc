// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

#define MaxFileLength 32
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------


char* User2System(int virtAddr, int length)
{
	char* buffer = NULL;
	buffer = new char[length + 1];
	
	if (buffer == NULL)
		return buffer;
	
	memset(buffer, 0, length + 1);
	
	int index;
	int character;
	for(index = 0; index < length; ++index)
	{
		machine->ReadMem(virtAddr + index, 1, &character);
		buffer[index] = (char)character;
		
		if(character == 0)
			break;
	}
	
	return buffer;
}

int System2User(int virtAddr, int length, char* buffer)
{
	if (length < 0)
		return -1;
		
	if (length == 0)
		return length;
		
	int index = 0;
	int character = 0;
	
	do {
		character = (int)buffer[index];
		machine->WriteMem(virtAddr + index, 1, character);
		index++;
	}while (index < length && character != 0);
	
	return index;
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
	
	switch(which)
	{
		case NoException:
			return;

		case SyscallException:
			switch(type)
			{
				case SC_Halt:
					printf("Shutdown, initiated by user program.\n");
					interrupt->Halt();
					return;
					
				case SC_Create:
				{
					int virtAddr;
					char* filename;
					
					printf("SC_Create ...\n");
					printf("Reading virtual address of filename.\n");
					
					virtAddr = machine->ReadRegister(4);
					printf("Reading file name...\n");
					filename = User2System(virtAddr, MaxFileLength + 1);
					if (filename == NULL)
					{
						printf("Not enough memory in system.\n");
						machine->WriteRegister(2,-1);
						delete[] filename;
						break;
					}
					
					printf("Finish reading filename.\n");
					printf("File name.\n");
					
					if (!fileSystem->Create(filename, 0))
					{
						printf("Error create file.\n");
						machine->WriteRegister(2,-1);
						delete[] filename;
						break;
					}
					
					machine->WriteRegister(2,0);
					delete[] filename;
					break;
				}
				case SC_Open:
				{
					int nameAddr = machine->ReadRegister(4);
					int _type = machine->ReadRegister(5);
					char* name;
					
					if (fileSystem->index >= 10)
					{
						machine->WriteRegister(2, -1);
						printf("\nCan not open more then 10 files.\n");
						break;
					}
					
					name = User2System(nameAddr, MaxFileLength + 1);
					if (strcmp(name,"stdin") == 0)
					{
						machine->WriteRegister(2, 0);
						delete[] name;
						break;
					}
					
					if (strcmp(name,"stdout") == 0)
					{
						machine->WriteRegister(2, 1);
						delete[] name;
						break;
					}
					
					int temp_index = fileSystem->index;
					
					fileSystem->openfile[temp_index] = fileSystem->Open(name, _type);
					if (fileSystem->openfile[temp_index] != NULL)
					{
						machine->WriteRegister(2, temp_index);
						printf("\nOpen successful.\n");
					}
					else
					{
						machine->WriteRegister(2,-1);
					}
					delete[] name;
					break;
				}
					
				case SC_Close:
				{
					int indexClose = machine->ReadRegister(4);
					int indexCrr = fileSystem->index;
					
					if (indexClose > indexCrr)
					{
						printf("\nClose failed.\n");
						machine->WriteRegister(2,-1);
						break;
					}
					printf("\nClose successful.\n");
					fileSystem->openfile[indexClose] = NULL;
					machine->WriteRegister(2,0);
					break;
				}

				case SC_Read:
				{
					// int Read(char *buffer, int charcount, OpenFileID id)
					// Input: buffer: chua ket qua doc duoc, charcount: so ki tu, id: id cua File
					// Output: So byte doc duoc, -1: loi, -2: cham den cuoi file

					int bufferAddr = machine->ReadRegister(4);	// doc dia chi buffer
					int charcount = machine->ReadRegister(5);	// doc so ki tu charcount
					int fileId = machine->ReadRegister(6);		// doc id cua file

					// id file khong hop le (0 <= fileId <= 10)
					if (fileId < 0 || fileId >= 10) {
						printf("\nInvalid file id.\n");
						machine->WriteRegister(2, -1);
						// Halt();
						break;
					}

					// kiem tra su ton tai cua file
					if (fileSystem->openfile[fileId] == NULL) {
						printf("\nFile doesn't exist.\n");
						machine->WriteRegister(2, -1);	// tra ve gia tri -1
						// Halt();
						break;
					}

					// truong hop doc tu stdout
					if (fileSystem->openfile[fileId]->type == 3) {
						printf("\nCan't read stdout.\n");
						machine->WriteRegister(2, -1);
						// Halt();
						break;
					}

					char* result = User2System(bufferAddr, charcount); // chua ket qua doc duoc
					int pos = fileSystem->openfile[fileId]->getPosition(); // vi tri con tro file hien tai

					// truong hop doc tu stdin
					if (fileSystem->openfile[fileId]->type == 2) {
						int byteRead = gSynchConsole->Read(result, charcount); // doc thong tin tu stdin
						System2User(bufferAddr, charcount, result);
						machine->WriteRegister(2, byteRead);
						delete result;
						// Halt();
						break;
					}

					// truong hop doc duoc file co noi dung
					if (fileSystem->openfile[fileId]->Read(result, charcount) > 0) {
						int newPos = fileSystem->openfile[fileId]->getPosition(); // vi tri con tro file moi
						int byteRead = newPos - pos;
						System2User(bufferAddr, byteRead, result);
						machine->WriteRegister(2, byteRead);
					}
					else // truong hop doc file rong
					{
						machine->WriteRegister(2, -2);
					}

					delete result;
					// Halt();
					break;
				}
			}

		default:
			printf("Unexpected user mode exception %d %d\n", which, type);
			interrupt->Halt();
			break;	
	}
}
