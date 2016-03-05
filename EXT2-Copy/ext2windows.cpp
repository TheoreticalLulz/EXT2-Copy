#include "ext2windows.h"

void ext2windows::selectFile()
{
	//Get file name
	printf("Input file name: ");
	getline(cin, m_tmpfilename);
	m_filename = new char[m_tmpfilename.length()];
	for(int i=0; i<m_tmpfilename.length(); i++)
	{
		m_filename[i] = m_tmpfilename[i];
	}
	
	//Open the file system as a binary stream
	m_filestream.open(m_filename, fstream::in | fstream::binary);
	
	//Check whether the file system is valid
	if(m_filestream.is_open())
	{
		printf("\nFile opened successfully. Reading...\n\n");
	}
	else
	{
		printf("\nERROR: Could not open file.\n");
		exit(0);
	}
}

void ext2windows::readSuperblock()
{
	//Skip the boot block of the file system
	m_filestream.seekg(1024, ios_base::beg);
	m_filestream.read(m_buffer, 1024);
	//Read the Superblock into the SUPERstruct
	memcpy(&SB, m_buffer, 1024);
	
	//Calculate the block size and group size
	bSIZE    = 1024*pow(2, SB.s_log_block_size);
	m_groups = SB.s_blocks_count / SB.s_blocks_per_group;
	m_buffer = new char[bSIZE];
	
	//Print some basic information
	cout << "File System Information (Read from the Superblock):"
	     << "\n\tTotal system size:\t"   << ((SB.s_blocks_count * bSIZE) + 2048)
	     << "\n\tFree space:\t\t"        << (SB.s_free_blocks_count*bSIZE)
	     << "\n\tUsed space:\t\t"        << ((SB.s_blocks_count - SB.s_free_blocks_count)*bSIZE)
	     << "\n\tBlock size:\t\t"        << bSIZE
	     << "\n\tInodes:\t\t\t"          << SB.s_inodes_count
	     << "\n\tBlocks:\t\t\t"          << SB.s_blocks_count
	     << "\n\tFree Inodes:\t\t"       << SB.s_free_inodes_count
	     << "\n\tFree Blocks:\t\t"       << SB.s_free_blocks_count
	     << "\n\tNumber of Groups:\t"    << m_groups
	     << "\n\tInodes per group:\t"    << SB.s_inodes_per_group
	     << "\n\nContinuing read...\n\n";
}

void ext2windows::readGroupDescriptor()
{
	//Relocate seek pointer past the Superblock
	if(bSIZE != 1024){m_filestream.seekg(bSIZE, ios_base::beg);}
	m_filestream.read(m_buffer, bSIZE);
	//Read the Group Descriptor table into the GROUPstruct
	memcpy(&GB, m_buffer, 32);
	
	//Calculate the number of inodes used by the group
	m_usedinodes = SB.s_inodes_per_group - GB.bg_free_inodes_count;
	
	//Print some basic information
	cout << "Group Information (Read from the Group Descriptor Table):"
	     << "\n\tBlock number of Block Bitmap: \t\t"    << GB.bg_block_bitmap
	     << "\n\tBlock number of Inode Bitmap: \t\t"    << GB.bg_inode_bitmap
	     << "\n\tBlock number of Inode Table: \t\t"     << GB.bg_inode_table
	     << "\n\tNumber of free group blocks: \t\t"     << GB.bg_free_blocks_count
	     << "\n\tNumber of free group blocks: \t\t"     << GB.bg_free_inodes_count
	     << "\n\tNumber of used group directories: \t"  << GB.bg_used_dirs_count
	     << "\n\nContinuing read...\n\n";
		 
	
	//// Note: This code operates on a single block group at a time. Thus, we do not need    ////
	////       to read in the Group Descriptor for every entry in the table. Provided the    ////
	////	   successful operation of this code for a single block group, it may be easily  ////
	////	   modified to account for multiple groups.                                      ////
}

void ext2windows::readRootInode()
{
	//Relocate seek pointer to the root inode
	m_filestream.seekg((bSIZE*GB.bg_inode_table) + 128, ios_base::beg);
	m_filestream.read(m_buffer, bSIZE);
	//Read root inode into struct
	memcpy(&IB, m_buffer, 128);
	
	//Print some basic information
	cout << "Root Inode Information (Read from the Inode Table):"
	     << "\n\tFile format and access rights: \t" << IB.i_mode
	     << "\n\tUser identification: \t\t"         << IB.i_uid
	     << "\n\tFile size: \t\t\t"                 << IB.i_size
	     << "\n\tSeconds since last access: \t"     << IB.i_atime
	     << "\n\tGroup Identification: \t\t"        << IB.i_gid
	     << "\n\tNumber of reserved blocks: \t"     << IB.i_blocks
	     << "\n\nContinuing read... \n\n";
}

void ext2windows::readDirectory()
{
	//Initialize arrays
	string node  = ".";
	    m_size   = ceil(IB.i_size/bSIZE);
	int m_dirs   = bSIZE*m_size;
	    m_dirarr = new unsigned char[m_dirs];

	cout << "\nFile size: \t" << IB.i_size
	     << "\nBlocks Needed: \t" << m_size
	     << "\nDirectory Size: " << m_dirs << "\n";

	//Fetch Directory Listings/File Data
	m_triple = new uint32_t[bSIZE/4];
	m_double = new uint32_t[bSIZE/4];
	m_single = new uint32_t[bSIZE/4];
	count    = 0;

	fetchBlockType();
	fetchBlock();

	//Read Directory Listing
	for (p=m_dirarr; m_dirarr-p<IB.i_size;)
	{
		pDent = (DIRECstruct *)p;
		if(pDent -> inode == 0){break;}

		cout << pDent->inode << "\t" << pDent->rec_len << "\tname=[";
		for (int i=0;i<pDent->name_len;i++)
		{
			cout << pDent->name[i];
		}

		cout << "]\n";
		p += pDent->rec_len;
	}

	//Select Inode
	cout << "Select Directory/File: ";
	getline(cin, node);			//Note: Need to correct for multiple inputs

	//Process Selection
	bool same;
	for(p=m_dirarr; m_dirarr-p<IB.i_size;)
	{
		pDent = (DIRECstruct *)p;
		if(pDent -> inode == 0){break;}

		for(int i=0; i<node.length(); i++)
		{
			if(node[i] == pDent->name[i]){same = true;}
			else{same = false; break;}
		}

		p += pDent->rec_len;
		if(same == true){break;}
	}
	cout << pDent->inode <<  "\t";
	for(int i=0; i<pDent->name_len; i++){cout << pDent->name[i];}
	cout << "\n";

	//Read in Inode
	readInode();
}


void ext2windows::readInode()
{
	//Relocate seek pointer to the root inode
	m_filestream.seekg((bSIZE*GB.bg_inode_table) + (pDent->inode*128) - 128, ios_base::beg);
	m_filestream.read(m_buffer, bSIZE);
	//Read root inode into struct
	memcpy(&IB, m_buffer, 128);
	
	//Print some basic information
	cout << "\n\nInode Information (Read from the Inode Table):"
	     << "\n\tFile format and access rights: \t" << IB.i_mode
	     << "\n\tUser identification: \t\t"         << IB.i_uid
	     << "\n\tFile size: \t\t\t"                 << IB.i_size
	     << "\n\tSeconds since last access: \t"     << IB.i_atime
	     << "\n\tGroup Identification: \t\t"        << IB.i_gid
	     << "\n\tNumber of reserved blocks: \t"     << IB.i_blocks
	     << "\n\nContinuing read... \n\n";

	//Read in Directory
	char copy;
	cout << "Read or Copy? [r/c] ";
	cin  >> copy;
	if(copy == 'r'){readDirectory();}
	if(copy == 'c'){copyData();}
}

void ext2windows::copyData()
{

	//Open an empty, readable file
	FILE * file;
	file = fopen(pDent->name, "w");

	//Write directory contents to file
	fwrite(m_dirarr, IB.i_size, IB.i_size, file);
	fclose(file);
}


void ext2windows::fetchBlockType()
{
	//Triple Indirect Read
	if(m_size > (12 + (bSIZE/4) + pow(bSIZE,2))){m_type = 3;}
	//Double Indirect Read
	if(m_size > (12 + (bSIZE/4)) && m_size <= (12 + (bSIZE/4) + pow(bSIZE,2))){m_type = 2;}
	//Single Indirect Read
	if(m_size > 12 && m_size <= (12 + (bSIZE/4))){m_type = 1;}
	//Direct Read
	if(m_size <= 12){m_type = 0;}

	cout << "\nType: " << m_type << "\n";
}


void ext2windows::fetchBlock()	// Note: Rewrite this method with a simpler recursive function call.
{
	//Direct Read
	if(m_type == 0)
	{
		for(int i=0; i<m_size; i++)
		{
			m_filestream.seekg((bSIZE*IB.i_block[i]), ios_base::beg);
			m_filestream.read(m_buffer, bSIZE);
			for(int j=0; j<bSIZE; j++){m_dirarr[(bSIZE*i) + j] = m_buffer[j];}
		}
	}


	//Single Indirect Read
	if(m_type == 1)
	{
		for(int i=0; i<12; i++)
		{
			m_filestream.seekg((bSIZE*IB.i_block[count]), ios_base::beg);
			m_filestream.read(m_buffer, bSIZE);
			for(int j=0; j<bSIZE; j++){m_dirarr[(bSIZE*i) + j] = m_buffer[j];}
		}

		m_filestream.seekg((bSIZE*IB.i_block[12]), ios_base::beg);
		m_filestream.read(m_buffer, bSIZE);
		memcpy(&m_single, m_buffer, bSIZE);
		for(int i=0; i<(m_size - 12); i++)
		{
			m_filestream.seekg((bSIZE*m_single[i]), ios_base::beg);
			m_filestream.read(m_buffer, bSIZE);
			for(int j=0; j<bSIZE; j++){m_dirarr[(bSIZE*(i + 12)) + j] = m_buffer[j];}
		}
	}


	//Double Indirect Read
	if(m_type == 2)
	{
		for(int i=0; i<12; i++)
		{
			m_filestream.seekg((bSIZE*IB.i_block[count]), ios_base::beg);
			m_filestream.read(m_buffer, bSIZE);
			for(int j=0; j<bSIZE; j++){m_dirarr[(bSIZE*i) + j] = m_buffer[j];}
		}

		m_filestream.seekg((bSIZE*IB.i_block[12]), ios_base::beg);
		m_filestream.read(m_buffer, bSIZE);
		memcpy(&m_single, m_buffer, bSIZE);
		for(int i=0; i<(bSIZE/4); i++)
		{
			m_filestream.seekg((bSIZE*m_single[i]), ios_base::beg);
			m_filestream.read(m_buffer, bSIZE);
			for(int j=0; j<bSIZE; j++){m_dirarr[(bSIZE*(i + 12)) + j] = m_buffer[j];}
		}

		m_filestream.seekg((bSIZE*IB.i_block[13]), ios_base::beg);
		m_filestream.read(m_buffer, bSIZE);
		memcpy(&m_double, m_buffer, bSIZE);
		int count = 12 + (bSIZE/4);
		while(count < m_size)
		{
			int cnt = 0;
			for(int i=0; i<(bSIZE/4); i++)
			{
				m_filestream.seekg((bSIZE*m_double[i]), ios_base::beg);
				m_filestream.read(m_buffer, bSIZE);
				memcpy(&m_single, m_buffer, bSIZE);
				for(int j=0; j<(bSIZE/4); j++)
				{
					m_filestream.seekg((bSIZE*m_single[j]), ios_base::beg);
					m_filestream.read(m_buffer, bSIZE);
					for(int k=0; k<bSIZE; k++)
					{
						m_dirarr[(bSIZE*count) + k] = m_buffer[k];
						count++;
					}		
				}
			}
		}
	}


	//Triple Indirect Read
	if(m_type == 3)
	{
		for(int i=0; i<12; i++)
		{
			m_filestream.seekg((bSIZE*IB.i_block[count]), ios_base::beg);
			m_filestream.read(m_buffer, bSIZE);
			for(int j=0; j<bSIZE; j++){m_dirarr[(bSIZE*i) + j] = m_buffer[j];}
		}

		m_filestream.seekg((bSIZE*IB.i_block[12]), ios_base::beg);
		m_filestream.read(m_buffer, bSIZE);
		memcpy(&m_single, m_buffer, bSIZE);
		for(int i=0; i<(bSIZE/4); i++)
		{
			m_filestream.seekg((bSIZE*m_single[i]), ios_base::beg);
			m_filestream.read(m_buffer, bSIZE);
			for(int j=0; j<bSIZE; j++){m_dirarr[(bSIZE*(i + 12)) + j] = m_buffer[j];}
		}

		m_filestream.seekg((bSIZE*IB.i_block[13]), ios_base::beg);
		m_filestream.read(m_buffer, bSIZE);
		memcpy(&m_double, m_buffer, bSIZE);
		int count =0;
		for(int i=0; i<(bSIZE/4); i++)
		{
			m_filestream.seekg((bSIZE*m_double[i]), ios_base::beg);
			m_filestream.read(m_buffer, bSIZE);
			memcpy(&m_single, m_buffer, bSIZE);
			for(int j=0; j<(bSIZE/4); j++)
			{
				m_filestream.seekg((bSIZE*m_single[j]), ios_base::beg);
				m_filestream.read(m_buffer, bSIZE);
				for(int k=0; k<bSIZE; k++)
				{
					m_dirarr[(bSIZE*count) + k] = m_buffer[k];
					count++;
				}		
			}
		}

		m_filestream.seekg((bSIZE*IB.i_block[14]), ios_base::beg);
		m_filestream.read(m_buffer, bSIZE);
		memcpy(&m_triple, m_buffer, bSIZE);
		while(count < m_size)
		{
			for(int i=0; i<(bSIZE/4); i++)
			{
				m_filestream.seekg((bSIZE*m_triple[i]), ios_base::beg);
				m_filestream.read(m_buffer, bSIZE);
				memcpy(&m_double, m_buffer, bSIZE);
				for(int j=0; j<(bSIZE/4); j++)
				{
					m_filestream.seekg((bSIZE*m_double[j]), ios_base::beg);
					m_filestream.read(m_buffer, bSIZE);
					memcpy(&m_single, m_buffer, bSIZE);
					for(int k=0; k<(bSIZE/4); k++)
					{
						m_filestream.seekg((bSIZE*m_single[k]), ios_base::beg);
						m_filestream.read(m_buffer,bSIZE);
						for(int l=0; l<bSIZE; l++)
						{
							m_dirarr[(bSIZE*count) + l] = m_buffer[l];
							count++;
						}
					}
				}
			}
		}
	}
}
