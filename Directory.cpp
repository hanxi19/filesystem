#include "Directory.h"
#include <memory.h>
#include <string.h>

SDirectory createEmptyDirectory()
{
	SDirectory Directory;
	memset(Directory.FileName, 0, g_MaxFileNameLen+1);
	for (short i=0; i<g_MaxNumFiles; ++i)
	{
		Directory.FileSet[i].InodeNum = -1;
		Directory.FileSet[i].IsInUse = false;
		memset(Directory.FileSet[i].FileName, 0, g_MaxFileNameLen+1);
	}

	return Directory;
}

int findFileInodeNum(const char* vFileName, const SDirectory& vDirectory)
{
	int FileIndex = findFileIndex(vFileName, vDirectory);

	return FileIndex != -1 ? vDirectory.FileSet[FileIndex].InodeNum : -1;
}

int findFileIndex(const char* vFileName, const SDirectory& vDirectory)
{
    // 遍历目录项，查找匹配的文件名
    for (short i = 0; i < g_MaxNumFiles; ++i)
    {
        if (vDirectory.FileSet[i].IsInUse && 
            strncmp(vDirectory.FileSet[i].FileName, vFileName, g_MaxFileNameLen) == 0)
        {
            return i; // 找到文件，返回索引
        }
    }

    return -1; // 未找到文件
}

bool addFile2Directory(const char* vFileName, short vInodeNum, SDirectory& voDirectory)
{
    // 检查文件名长度是否有效
    if (strlen(vFileName) > g_MaxFileNameLen)
    {
        return false; // 文件名过长
    }

    // 遍历目录项，寻找空闲的目录项
    for (short i = 0; i < g_MaxNumFiles; ++i)
    {
        if (!voDirectory.FileSet[i].IsInUse)
        {
            // 填充目录项
            voDirectory.FileSet[i].IsInUse = true;
            voDirectory.FileSet[i].InodeNum = vInodeNum;
            strncpy(voDirectory.FileSet[i].FileName, vFileName, g_MaxFileNameLen);
            voDirectory.FileSet[i].FileName[g_MaxFileNameLen] = '\0'; // 确保字符串以 '\0' 结尾
            return true;
        }
    }

    return false; // 目录已满，无法添加文件
}

bool removeFileFromDirectory(const char *vFileName, SDirectory& vioDirectory)
{ 
	int FileIndex = findFileIndex(vFileName, vioDirectory);

	if (FileIndex == -1) return false;

	vioDirectory.FileSet[FileIndex].InodeNum = -1;
	vioDirectory.FileSet[FileIndex].IsInUse = false;
	memset(vioDirectory.FileSet[FileIndex].FileName, 0, g_MaxFileNameLen+1);
	return true;
}