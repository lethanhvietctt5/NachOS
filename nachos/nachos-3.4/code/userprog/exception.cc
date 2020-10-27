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
#define MAX_LENGTH_INPUT 255
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
void IncreasePC()
{
	int counter = machine->ReadRegister(PCReg);
   	machine->WriteRegister(PrevPCReg, counter);
    	counter = machine->ReadRegister(NextPCReg);
    	machine->WriteRegister(PCReg, counter);
   	machine->WriteRegister(NextPCReg, counter + 4);
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
	
	switch(which)
	{
		case NoException:
			return;
		case PageFaultException:
		DEBUG('a', "\n No valid translation found");
		printf("\n\n No valid translation found");
		interrupt->Halt();
		break;

		case ReadOnlyException:
			DEBUG('a', "\n Write attempted to page marked read-only");
			printf("\n\n Write attempted to page marked read-only");
			interrupt->Halt();
			break;

		case BusErrorException:
			DEBUG('a', "\n Translation resulted invalid physical address");
			printf("\n\n Translation resulted invalid physical address");
			interrupt->Halt();
			break;

		case AddressErrorException:
			DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
			printf("\n\n Unaligned reference or one that was beyond the end of the address space");
			interrupt->Halt();
			break;

		case OverflowException:
			DEBUG('a', "\nInteger overflow in add or sub.");
			printf("\n\n Integer overflow in add or sub.");
			interrupt->Halt();
			break;

		case IllegalInstrException:
			DEBUG('a', "\n Unimplemented or reserved instr.");
			printf("\n\n Unimplemented or reserved instr.");
			interrupt->Halt();
			break;

		case NumExceptionTypes:
			DEBUG('a', "\n Number exception types");
			printf("\n\n Number exception types");
			interrupt->Halt();
			break;

		case SyscallException:
			switch(type)
			{
				case SC_Halt:
					printf("Shutdown, initiated by user program.\n");
					interrupt->Halt();
					return;
					
				case SC_Create:
				{
					// void Create(char *name);
					// Input : ten cua file can tao
					// Output: tra ve 0 neu thanh cong, -1 neu loi
					
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
						IncreasePC();
						return;
					}
					
					printf("Finish reading filename.\n");
					printf("File name.\n");
					
					if (!fileSystem->Create(filename, 0))
					{
						printf("Error create file.\n");
						machine->WriteRegister(2,-1);
						delete[] filename;
						IncreasePC();
						return;
					}
					
					machine->WriteRegister(2,0);
					delete[] filename;
					IncreasePC();
					return;
				}
				case SC_Open:
				{
					// OpenFileId Open(char *name, int type);
					// Input: [name: ten file], [type: 0 - readwrite, 1 - readonly, 2 - stdin, 3 - stdout]
					// Output: tra ve ID cua file neu mo thanh cong, -1 neu loi
					
					int nameAddr = machine->ReadRegister(4);
					int _type = machine->ReadRegister(5);
					char* name;
					
					if (fileSystem->index >= 10) // mo toi da 10 file cung luc (id: 0->9)
					{
						machine->WriteRegister(2, -1);
						printf("\nCan not open more then 10 files.\n");
						IncreasePC();
						return;
					}
					
					name = User2System(nameAddr, MaxFileLength + 1);
					
					// mo file stdin va stdout thi khong can tang bien index
					if (strcmp(name,"stdin") == 0)
					{
						machine->WriteRegister(2, 0);
						IncreasePC();
						delete[] name;
						return;
					}
					
					if (strcmp(name,"stdout") == 0)
					{
						machine->WriteRegister(2, 1);
						IncreasePC();
						delete[] name;
						return;
					}
					
					int temp_index = fileSystem->index;
					fileSystem->openfile[temp_index] = fileSystem->Open(name, _type);
					if (fileSystem->openfile[temp_index] != NULL)
					{
						machine->WriteRegister(2, temp_index); // tra ve id cua file duoc mo thanh cong
					}
					else
					{
						machine->WriteRegister(2,-1);
					}
					IncreasePC();
					delete[] name;
					return;
				}
					
				case SC_Close:
				{
					// void Close(OpenFileId id);
					// Input: id file can dong
					// Output: tra ve 0 neu dong thanh cong, -1 neu xay ra loi
					int indexClose = machine->ReadRegister(4);
					int indexCrr = fileSystem->index;
					
					if (indexClose > indexCrr)
					{
						printf("\nClose failed.\n");
						machine->WriteRegister(2,-1);
						IncreasePC();
						return;
					}
					fileSystem->openfile[indexClose] = NULL;
					machine->WriteRegister(2,0);
					IncreasePC();	
					return;
				}

				case SC_Read:
				{
					// int Read(char *buffer, int charcount, OpenFileID id)
					// Input: buffer: chua ket qua doc duoc, charcount: so ki tu, id: id cua File
					// Output: So byte doc duoc, -1: loi, -2: cham den cuoi file

					int bufferAddr = machine->ReadRegister(4);	// doc dia chi buffer
					int charcount = machine->ReadRegister(5);	// doc so ki tu charcount
					int fileId = machine->ReadRegister(6);		// doc id cua file

					// id file khong hop le (0 <= fileId < 10)
					if (fileId < 0 || fileId >= 10) {
						printf("Invalid file id.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}

					// kiem tra su ton tai cua file
					if (fileSystem->openfile[fileId] == NULL) {
						printf("File doesn't exist.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}

					// truong hop doc tu stdout
					if (fileSystem->openfile[fileId]->type == 3) {
						printf("Can't read stdout.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}

					char* result = User2System(bufferAddr, charcount); // chua ket qua doc duoc

					// truong hop doc tu stdin
					if (fileSystem->openfile[fileId]->type == 2) {
						int byteRead = gSynchConsole->Read(result, charcount); // doc thong tin tu stdin
						System2User(bufferAddr, charcount, result);
						machine->WriteRegister(2, byteRead);
						delete result;
						IncreasePC();
						return;
						
					}

					int pos = fileSystem->openfile[fileId]->getPosition(); // vi tri con tro file hien tai

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
					IncreasePC();
					return;
					
				}

				case SC_Write:
				{
					// int Write(char* buffer, int charcount, OpenFileID id)
					// Input : char* buffer, charcount: so ki tu, id: id cua file
					// Output: So byte ghi duoc, -1: loi, -2: den cuoi file

					int bufferAddr = machine->ReadRegister(4);	// doc dia chi buffer
					int charcount = machine->ReadRegister(5);	// doc so ki tu charcount
					int fileId = machine->ReadRegister(6);		// doc id cua file

					// id file khong hop le (0 <= fileId < 10)
					if (fileId < 0 || fileId >= 10) {
						printf("Invalid file id.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}

					// kiem tra su ton tai cua file
					if (fileSystem->openfile[fileId] == NULL) {
						printf("File doesn't exist.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}

					// truong hop ghi ra stdin hoac file Read-only 
					if (fileSystem->openfile[fileId]->type == 2 || fileSystem->openfile[fileId]->type == 1) {
						printf("Can't write to stdin or Read-only.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}

					char* buffer = User2System(bufferAddr, charcount);

					// truong hop ghi ra stdout
					if (fileSystem->openfile[fileId]->type == 3) {
						int i = 0;
						while (i <= charcount && buffer[i] != '\n' && buffer[i] != 0) {
							i++;
						}
						buffer[i] = '\n';
						gSynchConsole->Write(buffer, i + 1);
						machine->WriteRegister(2, i); // tra ve so byte thuc su ghi
						delete buffer;
						IncreasePC();
						return;
					}

					// truong hop ghi ra file Read-write
					if (fileSystem->openfile[fileId]->type == 0) {
						int pos = fileSystem->openfile[fileId]->getPosition(); // vi tri con tro file hien tai
						// ghi ra file
						fileSystem->openfile[fileId]->Write(buffer, charcount);
						int newPos = fileSystem->openfile[fileId]->getPosition(); // vi tri con tro file sau khi ghi
						machine->WriteRegister(2, newPos - pos); // tra ve so byte thuc su ghi
						delete buffer;
						IncreasePC();
						return;
					}
				}

				case SC_Seek:
				{
					// int Seek(int pos, OpenFileID id)
					// Input: pos: vi tri can chi chuyen con tro toi (-1 neu den cuoi file), id: id: id file
					// Output: tra ve vi tri thuc su cua con tro, -1: loi

					int pos = machine->ReadRegister(4);		// doc vi tri
					int fileId = machine->ReadRegister(5);	// doc id file

					// id file khong hop le (0 <= fileId < 10)
					if (fileId < 0 || fileId >= 10) {
						printf("Invalid file id.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}

					// kiem tra su ton tai cua file
					if (fileSystem->openfile[fileId] == NULL) {
						printf("File doesn't exist.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}

					// kiem tra file co phai la stdin, stdout hay khong
					if (fileSystem->openfile[fileId]->type == 2 || fileSystem->openfile[fileId]->type == 3) {
						printf("Can't seek on stdin or stdout.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					}
					// neu pos = -1 thi pos = chieu dai cua file
					pos = (pos == -1) ? fileSystem->openfile[fileId]->Length() : pos; 

					// kiem tra tinh hop le cua vi tri pos (0 <= pos <= length)
					if (pos < 0 || pos > fileSystem->openfile[fileId]->Length()) {
						printf("Can't seek to this position.\n");
						machine->WriteRegister(2, -1);
						IncreasePC();
						return;
					} else {
						// neu vi tri pos hop le thi chuyen con tro den va tra ve gia tri pos
						fileSystem->openfile[fileId]->Seek(pos);
						machine->WriteRegister(2, pos);
						IncreasePC();
						return;
					}
				}
				
				case SC_ReadString:
				{
					// void ReadString(char* buffer, int length);
					// buffer: mang luu chuoi
					// length: do dai cua chuoi
					
					char* buffer = new char[MAX_LENGTH_INPUT];
					if (buffer == 0)
					{
						printf("\nNot enough memory.\n");
						delete[] buffer;
						IncreasePC();
						return;
					}
					int virtAddr = machine->ReadRegister(4);
					int length = machine->ReadRegister(5);
					
					int size = gSynchConsole->Read(buffer, length);	// kich thuoc cua choi da nhap
					System2User(virtAddr, size, buffer);	// luu chuoi da nhap vao buffer
					delete[] buffer;
					IncreasePC();
					return;
				}
				
				case SC_PrintString:
				{
					// void PrintString(char* buffer);
					// buffer: chuoi can in ra
					int virtAddr = machine->ReadRegister(4);
					int i = 0;
					char* buffer = new char[MAX_LENGTH_INPUT];
					buffer = User2System(virtAddr, MAX_LENGTH_INPUT); // doc chuoi da nhap
					while (buffer[i] != 0)
					{
						gSynchConsole->Write(buffer + i, 1);	// in ra tung ky tu
						i++;
					}
					delete[] buffer;
					IncreasePC();
					return;
				}
				default:
					break;
			}

			IncreasePC();	
	
	}
}
