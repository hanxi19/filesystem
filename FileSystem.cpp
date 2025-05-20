#include "FileSystem.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <algorithm> 

char g_Disk[g_DiskSize] = {'\0'};
SDirectory g_RootDir = createEmptyDirectory();

bool createFile(const char* vFileName, int vDirInodeNum, char FileType, int vFileSize)
{
	if (vDirInodeNum >= g_NumInodes) return false;
	if (vFileSize > g_MaxFileSize) return false;

	SDirectory TempDirectory = loadDirectoryFromDisk(vDirInodeNum);

	bool IsSuccess = true;
	if (findFileInodeNum(vFileName, TempDirectory) != -1) IsSuccess = false;
	else
	{	
		SBitMap InodeBitMap; 
		createEmptyBitMap(InodeBitMap, g_NumInodes);
		memcpy(InodeBitMap.pMapData, g_Disk+g_BlockBitMapSize, g_InodeBitMapSize);            //������Ӳ���ж�ȡInodeλʾͼ

		int InodeIndex = findAndSetAvailableBit(InodeBitMap);
		if (InodeIndex == -1) IsSuccess = false;
		else if (!addFile2Directory(vFileName, InodeIndex, TempDirectory)) IsSuccess =  false;
		else 
		{
			
			SBitMap DataBlockBitMap;
			createEmptyBitMap(DataBlockBitMap, g_NumBlocks);
			memcpy(DataBlockBitMap.pMapData, g_Disk, g_BlockBitMapSize);                      //������Ӳ�̶�ȡ���ݿ�λʾͼ

			SInode TempInode;
			if (!allocateDisk(TempInode, vFileSize, DataBlockBitMap)) IsSuccess =  false;	
			else 
			{
				printf("Creating file %s(type:%c) size %d at %s\n", vFileName, FileType, vFileSize, TempDirectory.FileName);
				TempInode.FileType = FileType;
				saveInode2Disk(TempInode, InodeIndex);                                       //��Inode��Ϣд������Ӳ��
				memcpy(g_Disk, DataBlockBitMap.pMapData, g_BlockBitMapSize);                 //�����ݿ�λʾͼд������Ӳ��
				memcpy(g_Disk+g_BlockBitMapSize, InodeBitMap.pMapData, g_InodeBitMapSize);   //��Inodeλʾͼд������Ӳ��
				saveDirectory2Disk(vDirInodeNum, TempDirectory);
				if (FileType == 'd')
				{
					SDirectory SubDir = createEmptyDirectory();
					strncpy(SubDir.FileName, vFileName, g_MaxFileNameLen); 
					saveDirectory2Disk(InodeIndex, SubDir);
				}
			}
			delete DataBlockBitMap.pMapData;
		}
		delete InodeBitMap.pMapData;
	}
	return IsSuccess;
}

bool removeFile(const char* vFileName, int vDirInodeNum)
{
	SDirectory TempDirectory = loadDirectoryFromDisk(vDirInodeNum);

	//...       ��Ŀ¼���ҵ��ļ���Ӧ��Inode��ţ�����Inode��Ŷ���Inode���ݣ�����ļ���ɾ��
	
	removeFileFromDirectory(vFileName, TempDirectory);
	saveDirectory2Disk(vDirInodeNum, TempDirectory);                          //��Ŀ¼��������д������Ӳ��
	return true;
}

SDirectory loadDirectoryFromDisk(int vDirInodeNum)
{
	SInode DirInode = loadInodeFromDisk(vDirInodeNum);                     //ȡĿ¼Inode
	SDirectory Directory;
	memcpy(&Directory, g_Disk + g_BlockBitMapSize + g_InodeBitMapSize + sizeof(SInode)*vDirInodeNum, sizeof(SDirectory)); //ȡĿ¼

	return Directory;
}

void saveDirectory2Disk(int vDirInodeNum, const SDirectory& vDirectory)
{
	memcpy(g_Disk + g_BlockBitMapSize + g_InodeBitMapSize + sizeof(SInode)*vDirInodeNum, &vDirectory, sizeof(SDirectory)); //дĿ¼
}

void formatDisk()
{
	memset(g_Disk, 0, g_DiskSize);

	SBitMap InodeDiskBitMap;
	createEmptyBitMap(InodeDiskBitMap, g_NumInodes);                                   //����Inodeλʾͼ
	markBitAt(0, InodeDiskBitMap);                                                     //ΪĿ¼����Inode��Ĭ��Ϊ��0��Inode
	memcpy(g_Disk+g_BlockBitMapSize, InodeDiskBitMap.pMapData, g_InodeBitMapSize);     //��Inodeλʾͼд������Ӳ��
	delete InodeDiskBitMap.pMapData;

	SBitMap DataBlockBitMap;
	createEmptyBitMap(DataBlockBitMap, g_NumBlocks);                                   //�������ݿ�λʾͼ

	SDirectory Directory = createEmptyDirectory();                                     //������Ŀ¼
	strcpy(Directory.FileName, "/");
	SInode DirInode;
	DirInode.FileType  = 'd';
	allocateDisk(DirInode, sizeof(SDirectory), DataBlockBitMap);                       //�����Ŀ¼���ݿ�
	
	memcpy(g_Disk + g_BlockBitMapSize + g_InodeBitMapSize, &DirInode, sizeof(SInode)); //��Ŀ¼Inode��Ϣд��                               //��Inode��Ϣд������Ӳ��
	memcpy(g_Disk, DataBlockBitMap.pMapData, g_BlockBitMapSize);                       //�����ݿ�λʾͼд������Ӳ��
	delete DataBlockBitMap.pMapData;

	saveDirectory2Disk(0, Directory);
}

bool allocateDisk(SInode& voInode, int vFileSize, SBitMap& vioBlockBitMap)
{
	voInode.FileSize  = vFileSize;
	voInode.NumLinks  = 1;
	voInode.NumBlocks = (int)ceil(vFileSize / (double)g_BlockSize);

	if (countClearBits(vioBlockBitMap) < voInode.NumBlocks) return false;		

	for (int i=0; i<voInode.NumBlocks; ++i) voInode.BlockNums[i] = findAndSetAvailableBit(vioBlockBitMap);
	for (int i=voInode.NumBlocks; i<g_MaxNumBlock; ++i) voInode.BlockNums[i] = -1;
	
	return true;
}

void deallocateDisk(const SInode& vInode, SBitMap& vioBlockBitMap)
{
    // 释放文件占用的所有数据块
    for (int i = 0; i < vInode.NumBlocks; ++i)
    {
        if (vInode.BlockNums[i] != -1)
        {
            clearBitAt(vInode.BlockNums[i], vioBlockBitMap); // 将对应的数据块标记为未使用
        }
    }
}

SInode loadInodeFromDisk(int vInodeNum)
{
	SInode DirInode;
	memcpy(&DirInode, g_Disk + g_BlockBitMapSize + g_InodeBitMapSize + sizeof(SInode)*vInodeNum, sizeof(SInode)); 

	return DirInode;
}

void saveInode2Disk(const SInode& vInode, int vInodeNum)
{
	memcpy(g_Disk + g_BlockBitMapSize + g_InodeBitMapSize + sizeof(SInode)*vInodeNum, &vInode, sizeof(SInode)); 
}

bool writeData2File(const char* vFileName, int vDirInodeNum, const char* vData, int vDataSize)
{
	if (vDirInodeNum >= g_NumInodes || vDataSize > g_MaxFileSize) return false;

	SDirectory TempDirectory = loadDirectoryFromDisk(vDirInodeNum);
	int FileInodeNum = findFileInodeNum(vFileName, TempDirectory);
	if (FileInodeNum == -1) return false;

	SInode FileInode = loadInodeFromDisk(FileInodeNum);
	if (FileInode.FileType != 'f') return false;

	if (vDataSize > FileInode.NumBlocks * g_BlockSize) return false;

	int RemainingDataSize = vDataSize;
	const char* DataPtr = vData;

	for (int i = 0; i < FileInode.NumBlocks && RemainingDataSize > 0; ++i)
	{
		int BlockNum = FileInode.BlockNums[i];
		if (BlockNum == -1) break;
		
		int WriteSize = (RemainingDataSize < g_BlockSize) ? RemainingDataSize : g_BlockSize;
		memcpy(g_Disk + g_BlockBitMapSize + g_InodeBitMapSize + g_NumInodes * sizeof(SInode) + BlockNum * g_BlockSize, DataPtr, WriteSize);

		DataPtr += WriteSize;
		RemainingDataSize -= WriteSize;
	}

	FileInode.FileSize = vDataSize;
	saveInode2Disk(FileInode, FileInodeNum);

	return true;
}