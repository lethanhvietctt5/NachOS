#include "syscall.h"
#define MAX_LENGTH_INPUT 255

void main() {
    char filename[MAX_LENGTH_INPUT];
    int openFileId;
    int i;
    char c[1];

    PrintString("Nhap vao ten file: ");
    ReadString(filename, MAX_LENGTH_INPUT);

    openFileId = Open(filename, 1);

    if (openFileId == -1) {
        PrintString("Co loi khi mo file!\n");
    } else {
        int fileSize = Seek(-1, openFileId);
        Seek(0, openFileId);
        PrintString("---- Noi dung file ---- \n\n");
        for (i = 0; i < fileSize; i++) {
            Read(c, 1, openFileId);
            PrintString(c);     
        }

        Close(openFileId);
        PrintString("\n\n----  End of file  ----\n\n");
    }
}