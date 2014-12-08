#include "filesystem.h"
#include "ide.h"
#include "debug.h"
#include "stdfunc.h"
#include "clock.h"

namespace myos
{
	union U_block
	{
		uint16_t buf[256];
		data_block D;
		superblock S;
		inode_block I;
	};
	
	U_block tmp_buf;

	bus_base bus_base_used = SECONDARY;
	drive_type bus_drive_used = MASTER;

	const int INODE_BLK_BASE = 1, DATA_BLK_BASE = 6, NUM_MAX_BLKS = 256;
	
	const int NULL_INODE = 0;
	const int NULL_BLOCK_ADDRESS = 0;
	
	inode_id make_inode(filetype type , size_t filesize);
	void create_superblock();
	void sync_superblock();
	inode_id create_root_directory();
	void format();
	void fs_init();
	inode_id find_free_inodeid();
	baseaddr_t find_free_datablock();
	baseaddr_t assign_datablocks(int blks);
	void write_ientry_to_iblock(inode_struct modified_ientry);
	//inode_id find_inode_in_dentry_array(dir_entry_struct dentry_arr [], int sz, const char *name);
	inode_id find_inode_in_dir(inode_id dir_id, char *name);
	inode_id get_leaf_id(const char * const path);
	bool is_valid_dblock(baseaddr_t dblk);
	int unassign_datablocks(baseaddr_t beg_dblock);
	void unmake_inode(inode_id iid);
	baseaddr_t get_block_num(baseaddr_t first, uint8_t num);
	//int append_bytes_to_block(data_block &dblock, unsigned int used_sz, const uint8_t * to_be_appended, int num_appended);
	//bool append_dir_entry_to_block(data_block &dblock, unsigned int used_sz, const dir_entry_struct &to_be_appended_dentry);
	bool copy_data_at_offset(baseaddr_t dblock, size_t dblock_no, uint8_t *data, size_t data_sz, size_t offset);
	uint8_t get_block_count(uint16_t size_in_bytes);
	bool add_file_entry(inode_id pid, inode_id fid, const char * const fname);
	inode_id create_file(const char * const filename, const char * const path);
	inode_id get_file_id_from_path(const char *filepath);
	baseaddr_t put_into_dblocks(baseaddr_t beg_dblock, const uint8_t *data, size_t data_sz);
	bool write_over_file(const char *filepath, const char *data, size_t data_sz);
	bool read_from_file(const char *filepath, char *data, size_t data_sz);
	bool is_file_exists(const char *filepath);
	int get_file_size(const char *filepath);
	
	union
	{
		superblock sblk;
		uint16_t buf[256];
	} buf_sblk;

	globalinfo & fsinfo = buf_sblk.sblk.ginfo;
	uint8_t * const fsimap = buf_sblk.sblk.inode_bitmap32;
	uint8_t * const fsdmap = buf_sblk.sblk.data_bitmap32;

	bool from_map(uint8_t * const map, unsigned int i)
	{
		return map[i / 8] & (0x1 << i % 8);
	}

	void to_map(uint8_t * const map, unsigned int i, bool val)
	{
		if (val == true)
		{
			map[i / 8] |= (0x1 << i % 8);
		}
		else
		{
			map[i / 8] &= ~(0x1 << i % 8);
		}
	}

	bool imap(int i)
	{
		return from_map(fsimap, i);
	}

	void set_imap(int i)
	{
		to_map(fsimap, i, true);
	}

	void clr_imap(int i)
	{
		to_map(fsimap, i, true);
	}

	bool dmap(int i)
	{
		return from_map(fsdmap, i);
	}

	void set_dmap(int i)
	{
		to_map(fsdmap, i, true);
	}

	void clr_dmap(int i)
	{
		to_map(fsdmap, i, true);
	}

	void sync_superblock()
	{
		cmd_write_sector(bus_base_used, bus_drive_used, 0, buf_sblk.buf);
	}

	inode_id create_root_directory()
	{
		//magic_breakpoint_msg("In create_root_directory()\n");
		inode_id ret_id = make_inode(DIR, 0);
		//magic_breakpoint_msg("Out create_root_directory() -> ", ret_id, '\n');
		return ret_id;
	}

	void create_superblock()
	{
		//magic_breakpoint_msg("In create superblock()\n");

		char magicstring[32] = "KITTO                          ";
		memcpy<uint8_t>(reinterpret_cast<uint8_t *>(buf_sblk.buf), reinterpret_cast<const uint8_t *>(magicstring), 32);

		fsinfo.inode_blocks_base = INODE_BLK_BASE;
		fsinfo.num_inode_blocks = DATA_BLK_BASE - INODE_BLK_BASE;
		fsinfo.num_free_inodes = fsinfo.num_inode_blocks * 32 - 1;
		fsinfo.data_blocks_base = DATA_BLK_BASE;
		fsinfo.num_data_blocks = NUM_MAX_BLKS - DATA_BLK_BASE;
		fsinfo.num_free_datablocks = fsinfo.num_data_blocks;

		//illegal values for dmap
		for (int i = 0; i < fsinfo.data_blocks_base; i++)
		{
			set_dmap(i);
		}
		//illegal values for imap
		set_imap(NULL_INODE);
		
		//*/
		fsinfo.root_inode_id = create_root_directory();
		/*
		log(OK, "fsinfo.root_inode_id = ", fsinfo.root_inode_id, '\n');

		magic_breakpoint_msg("TEST BEGIN - superblock content\n");
		print_ginfo(buf_sblk.sblk.ginfo);
		print_map(buf_sblk.sblk.inode_bitmap32, 32);
		print_map(buf_sblk.sblk.data_bitmap32, 32);
		magic_breakpoint_msg("TEST END\n");
		//*/
		bool success = cmd_write_sector(bus_base_used, bus_drive_used, 0, buf_sblk.buf);

		if (success == false)
		{
			magic_breakpoint_msg("Superblock could not be written\n");
		}
		//magic_breakpoint_msg("Out create superblock()\n");
	}

	void format()
	{
		create_superblock();
	}


	void fs_init()
	{
		static_assert(sizeof(superblock) == 512, "Superblock size != 512 Bytes\n");
		static_assert(sizeof(inode_struct) == 16, "Inode size != 16 Bytes\n");
		static_assert(sizeof(inode_block) == 512, "Inode block size != 512 Bytes\n");
		static_assert(sizeof(data_block) == 512, "Data block size != 512 Bytes\n");
		static_assert(sizeof(dir_entry_struct) == 32, "Dir_entry size != 32 Bytes\n");
		//memset<uint16_t>(buf_sblk.buf, 0xA19F, 256);
		//cmd_write_sector(bus_base_used, bus_drive_used, 0, buf_sblk.buf);

		//set some initial values
		
		cmd_read_sector(bus_base_used, bus_drive_used, 0, buf_sblk.buf);

		char magicstring[32] = "KITTO                          ";

		if (strcmp(buf_sblk.sblk.ginfo.magic_number, magicstring) != 0)
		{ 
			log(INFO, "hard disk is not formatted with kitto... format it.\n");
			format();
		}
	}

	inode_id find_free_inodeid()
	{
		//magic_breakpoint_msg("In find_free_inodeid() : ");
		inode_id res = NULL_INODE;
		for (inode_id i = 0; i < fsinfo.num_inode_blocks * 32; i++)
		{
			//log(INFO, i, " ");
			if (imap(i) == 0)
			{
				res = i;
				break;
			}
		}

		if (res == NULL_INODE)
		{
			log(ERROR, "No empty inodeid found\n");
		}

		//magic_breakpoint_msg("\nOut find_free_inodeid() returning ", res, '\n');
		return res;
	}


	baseaddr_t find_free_datablock()
	{
		for (baseaddr_t i = fsinfo.data_blocks_base; i < fsinfo.data_blocks_base + fsinfo.num_data_blocks; i++)
		{
			if (dmap(i) == 0)
			{
				return i;
			}
		}
		return NULL_BLOCK_ADDRESS;
	}

	baseaddr_t assign_datablocks(int blks)
	{
		//magic_breakpoint_msg("In assign_dblocks(", uint32_t(blks), ")\n");

		baseaddr_t curr;

		if ((curr = find_free_datablock()) == NULL_BLOCK_ADDRESS)
		{
			log(ERROR, "assign blocks error: no free datablocks\n");
			//magic_breakpoint_msg("Out assign_dblocks\n");
			return NULL_BLOCK_ADDRESS;
		}
		else
		{
			//book it
			set_dmap(curr);
		}

		baseaddr_t next = NULL_BLOCK_ADDRESS;
		if (blks > 1)
		{
			next = assign_datablocks(blks - 1);

			if (next == NULL_BLOCK_ADDRESS)
			{
				clr_dmap(curr);
				log(ERROR, "assign blocks error: not sufficient data blocks \n");
				//magic_breakpoint_msg("Out assign_dblocks\n");
				return NULL_BLOCK_ADDRESS;
			}
		}

		//all the reqd blocks are there. just do 'it' now.

		
		memset<uint16_t>(tmp_buf.buf, 0, 256);
		tmp_buf.D.nextblk = next;
		cmd_write_sector(bus_base_used, bus_drive_used, curr, tmp_buf.buf);
		fsinfo.num_free_datablocks--;
		//terminal_writestring("Out assign_datablocks(");
		//terminal_writenumber(blks);
		//terminal_writestring(")\n");
		//magic_breakpoint_msg("Out assign_dblocks\n");
		return curr;
	}

	void write_ientry_to_iblock(inode_struct modified_ientry)
	{
		//magic_breakpoint_msg("In write_ientry_to_iblock()\n");
		
		int reqd_blknum = modified_ientry.iid / 32 + fsinfo.inode_blocks_base;
		
		cmd_read_sector(bus_base_used, bus_drive_used, reqd_blknum, tmp_buf.buf);
		//magic_breakpoint_msg("write_ientry_to_block(): read_sector\n");
		//memcpy<uint16_t>(U.buf, blockp, 256);

		tmp_buf.I.inode_entry[modified_ientry.iid % 32] = modified_ientry;
		//magic_breakpoint_msg("Before bug - \n");
		cmd_write_sector(bus_base_used, bus_drive_used, reqd_blknum, tmp_buf.buf);
		//magic_breakpoint_msg("write_ientry_to_block(): write_sector\n");
		//magic_breakpoint_msg("Out write_ientry_to_iblock()\n");
	}

	inode_id make_inode(filetype type, size_t filesize)
	{
		//magic_breakpoint_msg("In make_inode(", filesize, ")\n");
		
		// 'modified' field is always 0; as a placeholder to actual time
		int needed_blks = filesize / 500 + (filesize % 500 == 0 ? 0 : 1);
		//log(INFO, "needed blocks = ", needed_blks, '\n');

		if (fsinfo.num_free_inodes > 0 && fsinfo.num_free_datablocks >= needed_blks)
		{
			//log(OK, "Yes, assign. \n");
			inode_struct ientry;

			memset<uint8_t>(reinterpret_cast<uint8_t *>(&ientry), 0, sizeof(inode_struct));

			//magic_breakpoint_msg("Before entering find_free_inodeid()\n");
			const inode_id id = find_free_inodeid();
			//magic_breakpoint();
			//magic_breakpoint_msg("Returning from find_free_inodeid() with id = ", id, '\n');
			
			
			if (id == NULL_INODE)
			{
				log(FAULT, "Why this kolaveri di? Should have never reached here (because of if condition)\n");
			}
			else {
				set_imap(id);
				//log(OK, "imap(", id, ") is set\n");
				//magic_breakpoint();
			}

			ientry.modified = now();
			ientry.type = type;
			ientry.num_blks = needed_blks;
			ientry.num_bytes = filesize;
			ientry.iid = id;

			if (needed_blks > 0)
			{
				//log(OK, "needed_blks = ", needed_blks, " > 0\n");

				if ((ientry.first_data_blk = assign_datablocks(needed_blks)) == NULL_BLOCK_ADDRESS)
				{
					log(ERROR, "Content blocks could not be assigned\n");
					return NULL_INODE;
				}
			}
			//magic_breakpoint();
			//info.num_free_datablocks -= needed_blks;
			fsinfo.num_free_inodes--;

			
			//write ientry to respective inode_block

			//magic_breakpoint_msg("Ientry before writing to hard disk - \n");
			//print_ientry(ientry);
			//magic_breakpoint();
			write_ientry_to_iblock(ientry);
			//magic_breakpoint();
			//magic_breakpoint_msg("Written to iblock \n");
			//write buf_sblk to superblock

			cmd_write_sector(bus_base_used, bus_drive_used, 0, buf_sblk.buf);
			//magic_breakpoint();
			//magic_breakpoint_msg("Out make_inode(), iid = ", ientry.iid, '\n');
			return ientry.iid;
		}
		else
		{
			log(ERROR, "if condition failed in make_inode().\n");
			return NULL_INODE;
		}
		/*
		//1. check whether inode as well as data blocks are available
		//2. find free inodeid
		//3. find free data blocks and make a linked list out of them
		//4. make a new inode_entry struct and put the head of linkedlist in it.
		//5. Fill other info in the inode_entry
		//6. store inode_entry in inode_block of inodeid
		//7. store datablocks in respective block addresses (link addresses et al)
		//8. update free inodes, free data blocks, inode_map and datablk_map in superblock
		//9. return inodeid
		//*/
	}

	void print_ginfo(myos::globalinfo g)
	{
		using namespace myos;
		
		log(INFO, "Inodes: base = ", g.inode_blocks_base, ", ");
		log(INFO, "total = ", g.num_inode_blocks, ", ");
		log(INFO, "free = ", g.num_free_inodes, '\n');

		log(INFO, "Datablocks: base = ", g.data_blocks_base, ", ");
		log(INFO, "total = ", g.num_data_blocks, ", ");
		log(INFO, ", free = ", g.num_free_datablocks, '\n');

		log(INFO, "Root = ", g.root_inode_id, '\n');
	}

	void print_map(uint8_t * const map, int mapsize)
	{
		
		using namespace myos;

		set_log_int_base(16);

		for (int i = 0; i < mapsize; i++)
		{
			log(INFO, uint32_t(map[i]), ',');
		}
		set_log_int_base(10);
		log(INFO, '\n');
	}
	
	void print_ientry(inode_struct ientry)
	{
		
		log(INFO, "inode_struct: id = ", ientry.iid, ", ");
		log(INFO, "modified = ", ientry.modified, ", ");
		log(INFO, "num_bytes = ", ientry.num_bytes, ", ");
		log(INFO, "num_blocks = ", ientry.num_blks, ", ");
		log(INFO, "type = ", ientry.type, ", ");
		log(INFO, "content block = ", ientry.first_data_blk, '\n');
		//*/
	}

	inode_id find_inode_in_dentry_array(dir_entry_struct dentry_arr[], int sz, const char *name)
	{
		for (int i = 0; i < sz; i++)
		{
			if (strcmp(dentry_arr[i].name, name) == 0)
				return dentry_arr[i].file_inode;
		}
		return NULL_INODE;
	}

	inode_struct get_inode_entry(inode_id iid)
	{
		baseaddr_t reqd_iblock = iid / 32 + fsinfo.inode_blocks_base;
		cmd_read_sector(bus_base_used, bus_drive_used, reqd_iblock, tmp_buf.buf);
		return tmp_buf.I.inode_entry[iid % 32];
	}

	bool read_data_from_offset(baseaddr_t dblock, uint16_t offset, uint8_t *readbuf, size_t readsize)
	{

		cmd_read_sector(bus_base_used, bus_drive_used, dblock, tmp_buf.buf);

		if (offset + readsize > sizeof(data_block::data))
		{
			if (tmp_buf.D.nextblk == NULL_BLOCK_ADDRESS)
			{
				return false;
			}
			else
			{
				if (offset < sizeof(data_block::data))
				{
					int to_be_copied = sizeof(data_block::data) - offset;
					memcpy(readbuf, tmp_buf.D.data + offset, to_be_copied);
					return read_data_from_offset(tmp_buf.D.nextblk, 0, readbuf + to_be_copied, readsize - to_be_copied);
				}
				else
				{
					return read_data_from_offset(tmp_buf.D.nextblk, offset - sizeof(data_block::data), readbuf, readsize);
				}
			}
		}
		else
		{
			memcpy(readbuf, tmp_buf.D.data + offset, readsize);
			return true;
		}
	}

	inode_id find_inode_in_dir(inode_id dir_id, char *name)
	{
		//magic_breakpoint_msg("Find_inode_in_dir\n");
		static_assert(sizeof(uint8_t) == sizeof(char), "char is not 8 bytes\n");
		inode_struct dir_ientry = get_inode_entry(dir_id);

		if (dir_ientry.type != DIR)
		{
			log(ERROR, "inode ", dir_id, " is not directory\n");
			return NULL_INODE;
		}
		else
		{
			int dir_entry_count = dir_ientry.num_bytes / sizeof(dir_entry_struct);

	//		log(OK, dir_entry_count, " = number of entries in directory \n");

			int offset = 0;
			baseaddr_t curr_dblock = dir_ientry.first_data_blk;
			int read_data_sz = sizeof(dir_entry_struct);

			for (int i = 0; i < dir_entry_count; i++)
			{
				union
				{
					dir_entry_struct dentry;
					uint8_t buf[sizeof(dir_entry_struct)];
				} U;

				read_data_from_offset(curr_dblock, offset, U.buf, read_data_sz);
				
				if (strcmp(U.dentry.name, name) == 0)
				{
					return U.dentry.file_inode;
				}
				else
				{
					offset += sizeof(dir_entry_struct);
				}
			}

			return NULL_INODE;
		}
	}

	inode_id get_leaf_id(const char * const path)
	{
		//magic_breakpoint_msg("In get_leaf_id()\n");

		char name_buf[sizeof(dir_entry_struct::name) + 1];
		int offset = 0;
		int namesz = strncpy(name_buf, sizeof(name_buf), path, '/');
		inode_id pid;
		//log(INFO, "Breakpath: ", namesz, ", ", name_buf, '\n');
		inode_id ret_id;

		if (strcmp(name_buf, "root") != 0)
		{
			log(ERROR, "path error: bad root syntax\n");
			ret_id = NULL_INODE;
		}
		else
		{
			offset += namesz + 1;
			pid = fsinfo.root_inode_id;

			while (path[offset] != '\0')
			{
				//log(INFO, "rem: ", strlen(path + offset), ", ", path + offset, '\n');

				namesz = strncpy(name_buf, sizeof(name_buf), path + offset, '/');
				if ((pid = find_inode_in_dir(pid, name_buf)) == NULL_INODE)
				{
					log(ERROR, "path error: bad child directory name\n");
					ret_id = NULL_INODE;
					break;
				}
				else
				{
					offset += namesz + 1;
				}
			}
			ret_id = pid;
		}
		//magic_breakpoint_msg("Out get_leaf_id() -> ", ret_id, "\n");
		return ret_id;
	}

	bool is_valid_dblock(baseaddr_t dblk)
	{
		if (dblk >= fsinfo.data_blocks_base && dblk < fsinfo.data_blocks_base + fsinfo.num_data_blocks)
			return true;
		else
			return false;
	}

	int unassign_datablocks(baseaddr_t beg_dblock)
	{
		if (is_valid_dblock(beg_dblock) && dmap(beg_dblock) == 1)
		{
			

			cmd_read_sector(bus_base_used, bus_drive_used, beg_dblock, tmp_buf.buf);

			int unassigned = unassign_datablocks(tmp_buf.D.nextblk);
			clr_dmap(beg_dblock);
			fsinfo.num_free_datablocks++;
			return unassigned + 1;
		}
		else
		{
			return 0;
		}
	}

	void unmake_inode(inode_id iid)
	{
		if (imap(iid) == true && iid != fsinfo.root_inode_id && iid != NULL_INODE)
		{
			inode_struct ientry = get_inode_entry(iid);
			unassign_datablocks(ientry.first_data_blk);
			clr_imap(iid);
			fsinfo.num_free_inodes++;
		}
	}

	baseaddr_t get_block_num(baseaddr_t first, uint8_t num)
	{
		if (num == 0)
		{
			return NULL_BLOCK_ADDRESS;
		}
		else if (num == 1)
		{
			return first;
		}
		else
		{
			

			cmd_read_sector(bus_base_used, bus_drive_used, first, tmp_buf.buf);

			if (tmp_buf.D.nextblk != NULL_BLOCK_ADDRESS)
			{
				return get_block_num(tmp_buf.D.nextblk, num - 1);
			}
			else
			{
				return NULL_BLOCK_ADDRESS;
			}
		}
	}

	int append_bytes_to_block(data_block &dblock, unsigned int used_sz, const uint8_t * to_be_appended, int num_appended)
	{
		const int rem_bytes_in_dblock = sizeof(dblock.data) - used_sz;
	
		if (rem_bytes_in_dblock < num_appended)
			return false;
		else
		{
			memcpy(dblock.data + used_sz, to_be_appended, num_appended);
			return num_appended;
		}
	}
	bool append_dir_entry_to_block(data_block &dblock, unsigned int used_sz, const dir_entry_struct &to_be_appended_dentry)
	{
		return append_bytes_to_block(dblock, used_sz, reinterpret_cast<const uint8_t *>(&to_be_appended_dentry), sizeof(to_be_appended_dentry));
	}

	

	bool copy_data_at_offset(baseaddr_t dblock, size_t dblock_no, uint8_t *data, size_t data_sz, size_t offset)
	{
		if (!is_valid_dblock(dblock) || dblock_no >= MAX_ALLOWED_BLKS)
		{
			//error
			log(ERROR, "append_data_at_offset: error in params\n");
			return false;
		}
		else
		{
			U_block U;
			cmd_read_sector(bus_base_used, bus_drive_used, dblock, U.buf);

			uint16_t max_accessed_size_for_copy = offset + data_sz;
			if (max_accessed_size_for_copy > sizeof(data_block::data))
			{
				//further datablocks are required; following are the params for that
				baseaddr_t new_dblock;
				size_t new_dblock_no;
				uint8_t *new_data;
				size_t new_data_sz;
				size_t new_offset;

				size_t bytes_copied_in_curr_dblock;
				if (sizeof(data_block::data) < offset)
				{
					bytes_copied_in_curr_dblock = 0;
				}
				else
				{
					bytes_copied_in_curr_dblock = sizeof(data_block::data) - offset;
				}

				new_data = data + bytes_copied_in_curr_dblock;
				new_data_sz = data_sz - bytes_copied_in_curr_dblock;
				new_dblock_no = dblock_no + 1;
				new_dblock = U.D.nextblk;

				bool new_dblock_flag = false;
				if (new_dblock == NULL_BLOCK_ADDRESS)
				{
					//assign new dblock
					if ((new_dblock = assign_datablocks(1)) == NULL_BLOCK_ADDRESS)
					{
						return false;
					}
					else
					{
						 
						new_dblock_flag = true;
					}
				}

				new_offset = (bytes_copied_in_curr_dblock > 0) ? 0 : (offset - sizeof(data_block::data));

				bool is_ok = copy_data_at_offset(new_dblock, new_dblock_no, new_data, new_data_sz, new_offset);

				if (is_ok)
				{
					//copy in this dblock too and update datastructs
					
					if (bytes_copied_in_curr_dblock > 0)
					{
						memcpy(U.D.data + offset, data, bytes_copied_in_curr_dblock);
					}
					U.D.nextblk = new_dblock;

					cmd_write_sector(bus_base_used, bus_drive_used, dblock, U.buf);

					return true;
				}
				else
				{
					//failed in succeeding dblocks; no need to copy here.
					if (new_dblock_flag == true)
					{
						unassign_datablocks(new_dblock);
						 
					}
					return false;
				}
			}
			else
			{
				memcpy(U.D.data + offset, data, data_sz);
				U.D.nextblk = NULL_BLOCK_ADDRESS;
				cmd_write_sector(bus_base_used, bus_drive_used, dblock, U.buf);
				return true;
			}

		}
	}

	uint8_t get_block_count(uint16_t size_in_bytes)
	{
		//magic_breakpoint_msg("In get_block_count(", size_in_bytes, ")\n");

		if (size_in_bytes == 0)
		{
			return 0;
		}
		else
		{
			return static_cast<uint8_t>(1 + (size_in_bytes - 1) / sizeof(data_block::data));
		}
	}

	bool add_file_entry(inode_id pid, inode_id fid, const char * const fname)
	{
		//magic_breakpoint_msg("In add_file_entry\n");
		dir_entry_struct new_dentry;
		new_dentry.name_length = strncpy(new_dentry.name, sizeof(new_dentry.name), fname);
		new_dentry.file_inode = fid;
		
		
		inode_struct dir_ientry = get_inode_entry(pid);

		uint16_t last_byte_after_appending = dir_ientry.num_bytes + sizeof(new_dentry) - 1;
		
		

		if (last_byte_after_appending < MAX_ALLOWED_BLKS * sizeof(data_block::data))
		{
			baseaddr_t firstblock = dir_ientry.first_data_blk;
			bool assign_flag = false;

			if (firstblock == NULL_BLOCK_ADDRESS)
			{
				if ((firstblock = assign_datablocks(1)) == NULL_BLOCK_ADDRESS)
				{
					return false;
				}
				else
				{
					uint16_t zerobuf[256];
					memset(zerobuf, uint16_t(0), 256);
					cmd_write_sector(bus_base_used, bus_drive_used, firstblock, zerobuf);
					assign_flag = true;
				}
			}

			bool is_ok = copy_data_at_offset(firstblock, 1, reinterpret_cast<uint8_t *>(&new_dentry), sizeof(new_dentry), dir_ientry.num_bytes);

			if (is_ok)
			{
				dir_ientry.first_data_blk = firstblock;
				
				if (last_byte_after_appending + 1 > dir_ientry.num_bytes)
				{
					dir_ientry.num_bytes = last_byte_after_appending + 1;
					dir_ientry.num_blks = get_block_count(dir_ientry.num_bytes);
					//log(INFO, "num_blks = ", dir_ientry.num_blks, '\n');
				}

				write_ientry_to_iblock(dir_ientry);
				return true;
			}
			else
			{
				if (assign_flag == true)
				{
					unassign_datablocks(firstblock);
					 
				}
				return false;
			}
		}
		else
		{
			log(ERROR, "add_file_entry: exceeds per directory limit for number of files\n");
			return false;
		}
	}

	inode_id create_file(const char * const filename, const char * const path)
	{
	//	magic_breakpoint_msg("In create_file()\n");
		inode_id parent_id, ret_id;
		if ((parent_id = get_leaf_id(path)) == NULL_INODE)
		{
			log(ERROR, "Path is not valid\n");
			ret_id = NULL_INODE;
		}
		else
		{
			inode_id file_id;
			if ((file_id = make_inode(FILE, 0)) == NULL_INODE)
			{
				log(ERROR, "File inode could not be created\n");
				ret_id = NULL_INODE;
			}
			else
			{
		//		log(INFO, "file_id is ", file_id, '\n');

				if (add_file_entry(parent_id, file_id, filename) == false)
				{
					log(ERROR, "Directory could not accomodate file entry\n");
					unmake_inode(file_id);
					ret_id = NULL_INODE;
				}
				else
				{
		//			log(OK, "File is created\n");
					ret_id = file_id;
				}
			}
		}
		sync_superblock();
	//	magic_breakpoint_msg("Out create_file() ->", ret_id, "\n");
		return ret_id;
	}
	
	inode_id get_file_id_from_path(const char *filepath)
	{
		//magic_breakpoint_msg("In get_file_id_from_path \n");

		char pathbuf[200];

		int len = strncpy(pathbuf, 200, filepath);

		int path_breaker = len - 1;
		while (path_breaker >= 0)
		{
			
			if (pathbuf[path_breaker] == '/')
			{
				break;
			}
			else
			{
				path_breaker--;
			}
		}

	//	magic_breakpoint_msg(filepath, " broken at ", path_breaker, '\n');

		if (path_breaker < 0)
		{
			log(ERROR, "No '/' in filepath \n");
			return NULL_INODE;
		}
		else
		{
			char t = pathbuf[path_breaker + 1];
			pathbuf[path_breaker + 1] = '\0';

			inode_id dir_id = get_leaf_id(pathbuf);

			if (dir_id == NULL_INODE)
			{
				log(ERROR, "dir path ", pathbuf, " in path is invalid\n");
				return NULL_INODE;
			}
			else
			{
				pathbuf[path_breaker + 1] = t;
				inode_id file_id = find_inode_in_dir(dir_id, pathbuf + path_breaker + 1);

				if (file_id == NULL_INODE)
				{
					log(ERROR, "No file '", pathbuf + path_breaker + 1, "' in ", pathbuf, '\n');
					return NULL_INODE;
				}
				else
				{
					//magic_breakpoint_msg("Out: get_file_id_from_path\n");
					return file_id;
				}
			}
		}

	}

	baseaddr_t put_into_dblocks(baseaddr_t beg_dblock, const uint8_t *data, size_t data_sz)
	{
		bool assign_flag = false;
		if (beg_dblock == NULL_BLOCK_ADDRESS)
		{
			assign_flag = true;
			beg_dblock = assign_datablocks(get_block_count(data_sz));

			if (beg_dblock == NULL_BLOCK_ADDRESS)
			{
				//reqd number of blocks are not there
				return NULL_BLOCK_ADDRESS;
			}
		}


		U_block U;
		cmd_read_sector(bus_base_used, bus_drive_used, beg_dblock, U.buf);
		

		if (data_sz > sizeof(data_block::data))
		{
			baseaddr_t rem_blocks = put_into_dblocks(U.D.nextblk, data + sizeof(data_block::data), data_sz - sizeof(data_block::data));

			if (rem_blocks == NULL_BLOCK_ADDRESS)
			{
				if (assign_flag == true)
				{
					log(ERROR, "should never reach here since assign_true implies all needed blocks have been initialized already\n");
					unassign_datablocks(beg_dblock);
				}
				return NULL_BLOCK_ADDRESS;
			}
			else
			{
				memcpy(U.D.data, data, sizeof(data_block::data));
				U.D.nextblk = rem_blocks;
			}
		}
		else
		{
			memcpy(U.D.data, data, data_sz);
			U.D.nextblk = NULL_BLOCK_ADDRESS;
		}

		cmd_write_sector(bus_base_used, bus_drive_used, beg_dblock, U.buf);
		sync_superblock();

		return beg_dblock;
	}

	bool write_over_file(const char *filepath, const char *data, size_t data_sz)
	{
		inode_id file_id;
		//magic_breakpoint_msg("In write over file\n");
		if ((file_id = get_file_id_from_path(filepath)) == NULL_INODE)
		{
			log(ERROR, filepath, " is invalid path \n");
			return false;
		}
		else
		{
			inode_struct file_ientry = get_inode_entry(file_id);
			uint8_t reqd_dblocks = get_block_count(data_sz);

			if (reqd_dblocks <= fsinfo.num_free_datablocks + file_ientry.num_blks)
			{
				file_ientry.first_data_blk = put_into_dblocks(file_ientry.first_data_blk, reinterpret_cast<const uint8_t *>(data), data_sz);
				file_ientry.num_bytes = data_sz;
				file_ientry.num_blks = get_block_count(file_ientry.num_bytes);
				file_ientry.modified = now();

				write_ientry_to_iblock(file_ientry);
				sync_superblock();

				//magic_breakpoint_msg("Out write over file\n");
				return true;
			}
			else
			{
				log(ERROR, "Not enough free blocks\n");
				return false;
			}
		}
	}
	
	bool read_from_file(const char *filepath, char *data, size_t data_sz)
	{
		/*Another implementation might just use read_data_from_offset : less code and redundancy */

		inode_id fid;
		if ((fid = get_file_id_from_path(filepath)) == NULL_INODE)
		{
			return false;
		}
		else
		{
			inode_struct file_ientry = get_inode_entry(fid);

			if (file_ientry.num_bytes > data_sz)
			{
				return false;
			}
			else
			{
				int idx = 0;
				baseaddr_t curr = file_ientry.first_data_blk;
				size_t to_be_copied = file_ientry.num_bytes;

				while (curr != NULL_BLOCK_ADDRESS)
				{
					union
					{
						data_block D;
						uint16_t buf[256];
					} U;

					cmd_read_sector(bus_base_used, bus_drive_used, curr, U.buf);

					if (to_be_copied > sizeof(data_block::data))
					{
						memcpy((uint8_t *) (data) + idx, U.D.data, sizeof(data_block::data));
						idx += sizeof(data_block::data);
					}
					else
					{
						memcpy((uint8_t *) (data) + idx, U.D.data, to_be_copied);
						idx += to_be_copied;
					}
					curr = U.D.nextblk;
				}//end of while loop

				return true;
			}
		}
	}

	bool is_file_exists(const char *filepath)
	{
		return get_file_id_from_path(filepath);
	}

	int get_file_size(const char *filepath)
	{
		inode_id fid;
		if ((fid = get_file_id_from_path(filepath)) == NULL_INODE)
		{
			log(ERROR, "Filepath is invalid\n");
			return -1;
		}
		else
		{
			inode_struct file_ientry = get_inode_entry(fid);

			return file_ientry.num_bytes;
		}
	}
	
	void list_directory_contents(const char * dirpath)
	{
		inode_id dir_id;

		if ((dir_id = get_leaf_id(dirpath)) == NULL_INODE)
		{
			log(ERROR, "Directory does not exist \n");
			return;
		}
		else
		{
			inode_struct dir_ientry = get_inode_entry(dir_id);

			if (dir_ientry.type != DIR)
			{
				log(ERROR, "inode ", dir_id, " is not directory\n");
				return;
			}
			else
			{
				int dir_entry_count = dir_ientry.num_bytes / sizeof(dir_entry_struct);

				//log(OK, dir_entry_count, " = number of entries in directory \n");

				int offset = 0;
				baseaddr_t curr_dblock = dir_ientry.first_data_blk;
				int read_data_sz = sizeof(dir_entry_struct);

				for (int i = 0; i < dir_entry_count; i++)
				{
					union
					{
						dir_entry_struct dentry;
						uint8_t buf[sizeof(dir_entry_struct)];
					} U;

					read_data_from_offset(curr_dblock, offset, U.buf, read_data_sz);

					log(INFO, i + 1, ") ", U.dentry.name, '\n');

					offset += sizeof(dir_entry_struct);
				}
				return;
			}
		}
	} // end of list_direc_contents()

}
