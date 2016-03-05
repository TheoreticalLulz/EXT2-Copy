#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <cstring>
#include <string>

using namespace std;

class ext2windows
{
public:
		ext2windows();
		
		void selectFile();
		void readSuperblock();
		void readGroupDescriptor();
		void readRootInode();
		void readInode();
		void readDirectory();
		void fetchBlock();
		void fetchBlockType();
		void copyData();
		
private:

	struct SUPERstruct //Size: 1024 bytes
	{
		//Superblock Data
       		uint32_t s_inodes_count;                        /* Inodes count */
        	uint32_t s_blocks_count;                        /* Blocks count */
        	uint32_t s_r_blocks_count;                      /* Reserved blocks count */
        	uint32_t s_free_blocks_count;                   /* Free blocks count */
        	uint32_t s_free_inodes_count;                   /* Free inodes count */
        	uint32_t s_first_data_block;                    /* First data block */
        	uint32_t s_log_block_size;                      /* Block size indicator */
        	uint32_t s_log_frag_size;                       /* Fragment size indicator */
        	uint32_t s_blocks_per_group;                    /* Number of blocks in each block group */
        	uint32_t s_frags_per_group;                     /* Number of fragments in each block group */
        	uint32_t s_inodes_per_group;                    /* Number of inodes per group */
        	uint32_t s_mtime;                               /* Time of the last filesystem mounting */
        	uint32_t s_wtime;                               /* Time of the last filesystem writing */
        	uint16_t s_mnt_count;                           /* Number of times the filesystem was mounted */
        	uint16_t s_max_mnt_count;                       /* Number of times the filesystem can be mounted */
        	uint16_t s_magic;                               /* Magic number indicating ext2fs */
        	uint16_t s_state;                               /* Flags indicating file system state */
        	uint16_t s_errors;                              /* Flags indicating procedures for error reporting */
        	uint16_t s_minor_rev_level;                     /* Value identifying the minor revision level */
        	uint32_t s_lastcheck;                           /* Time of the last file system check */
        	uint32_t s_checkinterval;                       /* Maximum time allowed between file system checks */
        	uint32_t s_creator_os;                          /* Identifier of the OS which created the filesystem */
        	uint32_t s_rev_level;                           /* Value indicating the revision level of the filesystem */
        	uint16_t s_def_resuid;                          /* Default user id for reserved blocks */
        	uint16_t s_def_resgid;                          /* Default group id for reserved blocks */
        	uint32_t s_first_ino;                           /* Index to the first inode usable for standard files */
        	uint16_t s_inode_size;                          /* Size of the inode structure */
        	uint16_t s_block_group_nr;                      /* Block group number hosting the superblock structure */
        	uint32_t s_feature_compat;                      /* Bitmask of compatible features */
        	uint32_t s_feature_incompat;                    /* Bitmask of incompatible features */
        	uint32_t s_feature_ro_compat;                   /* Bitmask of 'read-only' features */
        	uint8_t  s_uuid[16];                            /* Volume id */
        	char     s_volume_name[16];                     /* Volume name */
        	char     s_last_mounted[64];                    /* Directory path where the filesystem was last mounted */
        	uint32_t s_algo_bitmap;                         /* Compression method for compression algorithms */
        	uint8_t  s_prealloc_blocks;                     /* Number of blocks attempted to pre-allocate when creating a new file */
        	uint8_t  s_prealloc_dir_blocks;                 /* Number of blocks attempted to pre-allocate when creating a new directory */
        	uint16_t s_padding1;                            /* Padding */
        	uint32_t s_reserved[204];                       /* System reserved space */
	
		
		// Note: The superblock is a constant 1024 bytes. To guard against file corruption,
		//       each group contains a copy of the master superblock.
	};
	
	
	struct GROUPstruct //Size: 32 bytes
	{
		//Group Descriptor Data
		uint32_t bg_block_bitmap;               /* Block id of the first block of the block bitmap */
		uint32_t bg_inode_bitmap;               /* Block id of the first block of the inode bitmap */
		uint32_t bg_inode_table;                /* Block id of the first block of the inode table */
		uint16_t bg_free_blocks_count;          /* Total number of free blocks for the group */
		uint16_t bg_free_inodes_count;          /* Total number of free inodes for the group */
		uint16_t bg_used_dirs_count;            /* Number of inodes allocated to directories for the represented group */
		uint16_t bg_pad;                        /* Padding */
		uint8_t  bg_reserved[12];               /* System reserved space */
		
		
		// Note: The group descriptor table contains entries for every group, and thus may
		//       encompass multiple blocks depending on how many group descriptor entries
		//       are present within the filesystem. Now, like the superblock, a copy of the
		//       group descriptor table exists within each group.
		// Note: This code shall initially assume that only one block exists for the group
		//       descriptor table. Likewise, the code is initially meant to work for a
		//       single group within the file system. Eventually, both options shall be expanded.
	};

	
	struct INODEstruct //Size: 128 bytes
	{
		//Inode Table Data
		uint16_t i_mode;                        /* Indicator of the format of the described file and access rights */
		uint16_t i_uid;                         /* User id associated with the file */
		uint32_t i_size;                        /* File size */
		uint32_t i_atime;                       /* Second count since last inode access */
		uint32_t i_ctime;                       /* Second count since inode was created */
		uint32_t i_mtime;                       /* Second count since last inode modification */
		uint32_t i_dtime;                       /* Second count since inode was deleted */
		uint16_t i_gid;                         /* Group id with inode access */
		uint16_t i_links_count;                 /* Number of times the inode is linked to */
		uint32_t i_blocks;                      /* Number of blocks reserved to contain file data */
		uint32_t i_flags;                       /* Indicator of implementation behavior when accessing inode data */
		uint32_t i_osdl;                        /* OS dependent value */
		uint32_t i_block[15];                   /* Block numbers pointing to the blocks containing data for this inode */
		uint32_t i_generation;                  /* File version */
		uint32_t i_file_acl;                    /* Block number containing extended attributes */
		uint32_t i_dir_acl;                     /* Value containing high 32 bits of 64bit file size */
		uint32_t i_faddr;                       /* Location of the file fragment */
		uint32_t i_osd2[3];                     /* OS dependant structure */
		
		
		// Note: Each inode within the filesystem is given a unique inode table. The first inode
		//       table is pointed to from within the group descriptor table. Understanding that
		//       there are multiple inodes, it follows that the inode tables may encompass multiple
		//       blocks.
		// Note: This code shall initially assume that every file is contained within a single inode
		//       table. Understanding the potential file size allotted by each inode, it is unlikely
		//       that a modification shall occur, lest we are instructed to modify the code.
	};

	struct DIRECstruct //Size: Variable - Read first eight bytes, and use name_len to determine the entire struct size
	{
		//Directory Data
		uint32_t inode;                         /* Inode number of the file entry */
		uint16_t rec_len;                       /* Displacement to the next directory from the start of the current directory entry */
		uint8_t  name_len;                      /* Number of bytes of character data contained in the name */
		uint8_t  file_type;                     /* File type */
		char     name[1];                       /* Entry name */
		
		
		// Note: It should be noted that directories are unique to inodes with a 'directory' type.
	};
	
		SUPERstruct SB;
		GROUPstruct GB;
		INODEstruct IB;
		DIRECstruct DB;
	
		//Miscellaneous variables
		int        bSIZE;                       	/* Block size */
		int        m_size;                      	/* Number of blocks in directory inode */
		int        count;                     		/* Count of used blocks */
		int        m_groups;                    	/* Number of groups */
		int        m_usedinodes;                	/* Number of used inodes within the group */
		int        m_type;                      	/* Type of reading process */
         DIRECstruct *     pDent;
       unsigned char *     p;
				char *     m_filename;              /* Name for the ext2 file system */
                char *     m_buffer;                /* File buffer */
       unsigned char *     m_dirarr;                /* Array for Directory or File */
	    uint32_t *     m_single;                    /* Single Indirect Buffer */
	    uint32_t *     m_double;                    /* Double Indirect Buffer */
	    uint32_t *     m_triple;                    /* Triple Indirect Buffer */
	      string       m_tmpfilename;               /* Temporary name for ext2 file system */
	    ifstream       m_filestream;                /* Data from the opened file system */
};
