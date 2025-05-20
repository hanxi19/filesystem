#include <memory.h>
#include <stdio.h>
#include "FileSystem.h"

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

int main(void)
{
	test();

	return 0;
} 