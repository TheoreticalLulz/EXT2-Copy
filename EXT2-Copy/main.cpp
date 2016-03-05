#include "ext2windows.h"

ext2windows::ext2windows()
{
	bSIZE    = 1024;
	m_groups = 1;
	m_buffer = new char[1024];
}

int main()
{
	ext2windows copy;
	copy.selectFile();
	copy.readSuperblock();
	copy.readGroupDescriptor();
	copy.readRootInode();
	copy.readDirectory();

	return 0;
}
