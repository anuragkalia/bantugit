#include <stdint.h>
#include <stddef.h>
#include "clock.h"

namespace myos
{
	enum filetype {FILE, DIR, EXE};

	typedef uint8_t baseaddr_t;
	typedef uint8_t inode_id;

	struct inode_struct
	{
		inode_id iid;
		uint8_t rsv1;
		baseaddr_t first_data_blk;
		uint8_t num_blks;
		uint8_t rsv2[2];
		uint16_t num_bytes;
		timestamp_t modified;
		filetype type;
	};

	struct globalinfo
	{
		char magic_number[32];

		uint8_t reserved1;
		baseaddr_t inode_blocks_base;
		uint8_t reserved2[3];
		uint8_t num_inode_blocks;
		uint8_t reserved3;
		uint8_t num_free_inodes;

		uint8_t reserved4;
		baseaddr_t data_blocks_base;
		uint8_t reserved5[3];
		uint8_t num_data_blocks;
		uint8_t reserved6;
		uint8_t num_free_datablocks;

		inode_id root_inode_id;
	};

	struct superblock
	{
		//first 256 bytes
		globalinfo ginfo;
		uint8_t reserved1[256 - sizeof(globalinfo)];
		//next 128 bytes
		uint8_t inode_bitmap32[32];
		uint8_t reserved2[128 - 32];
		//next 128 bytes
		uint8_t data_bitmap32[32];
		uint8_t reserved3[128 - 32];
	};

	struct inode_block
	{
		inode_struct inode_entry[32];
	};

	struct data_block
	{
		uint8_t data[500];
		uint8_t rsv[11];
		baseaddr_t nextblk;
	};

	struct dir_entry_struct
	{
		char name[20];
		char rsv[4];
		inode_id file_inode;
		uint32_t name_length;
	};

	const int MAX_ALLOWED_BLKS = 4;
	//inode_id create_root_directory();
	void fs_init();
	void format();
	inode_id create_file(const char * const filename, const char * const path);
	bool write_over_file(const char *filepath, const char *data, size_t data_sz);
	bool read_from_file(const char *filepath, char *data, size_t data_sz);
	bool is_file_exists(const char *filepath);
	int get_file_size(const char *filepath);

	void list_directory_contents(const char *path);
	/*
	bool imap(int i);
	bool dmap(int i);
	void set_imap(int i);
	void set_dmap(int i);
	void clr_imap(int i);
	void clr_dmap(int i);
	//*/
	//void fs_test();
	//void print_ginfo(myos::globalinfo ginfo);
	//void print_map(uint8_t * const map, int mapsize);
	//void print_ientry(inode_struct ientry);


}
/*

Filesystem capabilities:
========================

//1. Create node structure: file, directory or executable. last modified. unique id.
//2. Create a superblock in sector 0: It stores all the global properties of filesystem. Number of file blocks. Base of inode block and data block etc etc. inode of root directory.
3. possible to create new file.
4. possible to create new directory.
5. possible to read from a file.
6. possible to write to a file.
7. possible to delete a file.
8. possible to delete a directory.
*/


/*------directory format:
1. number of child files
2. corresponding number of 
//*/