#include <memory.h>
#include <stdio.h>
#include <algorithm>
#include "FileSystem.h"
#include "Directory.h"
#include "BitMap.h"

void test()
{
	formatDisk();

	SBitMap InodeBitMap;
	createEmptyBitMap(InodeBitMap, g_NumInodes);

	SBitMap DataBlockBitMap;
	createEmptyBitMap(DataBlockBitMap, g_NumBlocks);

	createFile("test.txt",  0, 'f', g_MaxFileSize);
	createFile("test1.txt", 0, 'f', g_MaxFileSize);
	createFile("test1.txt", 0, 'f', 0);

	memcpy(InodeBitMap.pMapData, g_Disk+g_BlockBitMapSize, g_InodeBitMapSize);   //������Ӳ���ж�ȡInode��λʾͼ
	memcpy(DataBlockBitMap.pMapData, g_Disk, g_BlockBitMapSize);                 //������Ӳ�̶�ȡ���ݿ�λʾͼ
	printf("Inode usage:");
	printfBitMap(InodeBitMap);
	printf("Block usage:");
	printfBitMap(DataBlockBitMap);

	removeFile("test.txt", 0);
	removeFile("test.txt", 0);
	memcpy(InodeBitMap.pMapData, g_Disk+g_BlockBitMapSize, g_InodeBitMapSize);   //������Ӳ���ж�ȡInode��λʾͼ
	memcpy(DataBlockBitMap.pMapData, g_Disk, g_BlockBitMapSize);                 //������Ӳ�̶�ȡ���ݿ�λʾͼ
	printf("\nInode usage:");
	printfBitMap(InodeBitMap);
	printf("Block usage:");
	printfBitMap(DataBlockBitMap);

	delete InodeBitMap.pMapData;
	delete DataBlockBitMap.pMapData;
}

void testFileSystem()
{
    const char* fileName = "testfile";
    const char* nameAndId = "hanxi2023141460273"; // 姓名拼音和学号
    int dirInodeNum = 0; // 根目录的 Inode 编号

    // 创建文件
    if (!createFile(fileName, dirInodeNum, 'f', strlen(nameAndId)))
    {
        printf("Failed to create file: %s\n", fileName);
        return;
    }

    // 写入数据
    if (!writeData2File(fileName, dirInodeNum, nameAndId, strlen(nameAndId)))
    {
        printf("Failed to write data to file: %s\n", fileName);
        return;
    }

    // 读取文件 Inode 信息
    SDirectory directory = loadDirectoryFromDisk(dirInodeNum);
    int fileInodeNum = findFileInodeNum(fileName, directory);
    if (fileInodeNum == -1)
    {
        printf("File not found: %s\n", fileName);
        return;
    }

    SInode fileInode = loadInodeFromDisk(fileInodeNum);

    // 输出文件 Inode 信息
    printf("File Inode Information:\n");
    printf("FileType: %c\n", fileInode.FileType);
    printf("FileSize: %d\n", fileInode.FileSize);
    printf("NumBlocks: %d\n", fileInode.NumBlocks);
    printf("BlockNums: ");
    for (int i = 0; i < fileInode.NumBlocks; ++i)
    {
        printf("%d ", fileInode.BlockNums[i]);
    }
    printf("\n");

    // 读取文件内容
    char fileContent[g_MaxFileSize] = {0};
    int remainingSize = fileInode.FileSize;
    char* contentPtr = fileContent;

    for (int i = 0; i < fileInode.NumBlocks && remainingSize > 0; ++i)
    {
        int blockNum = fileInode.BlockNums[i];
        if (blockNum == -1) break;
		
		int readSize = (remainingSize < g_BlockSize) ? remainingSize : g_BlockSize;
        memcpy(contentPtr, g_Disk + g_BlockBitMapSize + g_InodeBitMapSize + g_NumInodes * sizeof(SInode) + blockNum * g_BlockSize, readSize);

        contentPtr += readSize;
        remainingSize -= readSize;
    }

    // 输出文件内容
    printf("File Content: %s\n", fileContent);
}

int main(void)
{
	formatDisk(); // 格式化磁盘
    testFileSystem(); // 测试文件系统
    return 0;
}