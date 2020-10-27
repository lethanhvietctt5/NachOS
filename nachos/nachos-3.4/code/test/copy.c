#include "syscall.h"
#define MAX_LENGTH_INPUT 255

void main() {
    char desFilename[MAX_LENGTH_INPUT];
    char srcFilename[MAX_LENGTH_INPUT]; 
    int desFileId, srcFileId;
    char c[1];
    int i, fileSize;

    PrintString("Nhap vao ten file nguon: ");
    ReadString(srcFilename, MAX_LENGTH_INPUT);
    
    srcFileId = Open(srcFilename, 1);

    if (srcFileId == -1) {
        PrintString("Co loi khi mo file nguon!\n");
    } else {
        PrintString("Nhap vao ten file dich: ");
        ReadString(desFilename, MAX_LENGTH_INPUT);

        desFileId = Open(desFilename, 0);

        if (desFileId == -1) {
            Create(desFilename);
            desFileId = Open(desFilename, 0);
        }

        fileSize = Seek(-1, srcFileId);
        Seek(0, srcFileId);

        for (i = 0; i < fileSize; i++) {
            Read(c, 1, srcFileId);
            Write(c, 1, desFileId);
        }

        PrintString("\nCopy thanh cong!\n\n");
        
        Close(desFileId);
        Close(srcFileId);
    }
}