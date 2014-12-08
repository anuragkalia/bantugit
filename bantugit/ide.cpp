#include "ide.h"
#include "io_asm.h"
#include "debug.h"

namespace myos{

	enum port_offset
	{
		DATA = 0,
		RD_ERROR = 1, WR_FEATURES = 1,
		SEC_COUNT = 2,
		SEC_NUM = 3, LBA_LOW = 3,
		CYL_LOW = 4, LBA_MID = 4,
		CYL_HIGH = 5, LBA_HI = 5,
		DRIVE_HEAD_SEL = 6,
		RD_REG_STATUS = 7, WR_CMD = 7,
		RD_ALT_STATUS = 0x206, WR_DEV_CTRL = 0x206,
		RD_DRIVE_ADDR = 0x207
	};

	enum cmd_type
	{
		IDENTIFY_DRIVE = 0xEC,
		READ_SECTORS = 0x20,
		READ_SECTORS_NORETRY = 0x21,
		WRITE_SECTORS = 0x30,
		WRITE_SECTORS_NOTRY = 0x31,
	};

	enum reg_status_bit
	{
		ERR = 0x01,	// error
		IDX = 0x02,	// index mark
		CORR = 0x04,	// data corrected
		DRQ = 0x08,	// Data Transfer Requested
		DSC = 0x10,	// seek complete
		DF = 0x20,	// Device Fault
		DRDY = 0x40,	// Device Ready
		BSY = 0x80,	// Busy
	};

	enum error_bit
	{
		BBK = 0x80,	// Bad Block
		UNC = 0x40,	// Uncorrectable data error
		MC = 0x20,	// Media Changed
		IDNF = 0x10,	// ID mark Not Found
		MCR = 0x08,	// Media Change Requested
		ABRT = 0x04,	// command aborted
		TK0NF = 0x02,	// Track 0 Not Found
		AMNF = 0x01,	// Address Mark Not Found
	};

	enum dev_ctrl_bit
	{
		nIEN = 0x01, 	// Set this to stop the current device from sending interrupts.
		SRST = 0x02,	// Set this to do a "Software Reset" on all ATA drives on a bus, if one is misbehaving.
		HOB = 0x80,	// Set this to read back the High Order Byte of the last LBA48 value sent to an IO port
	};

	void set_drive_reg(bus_base bus, drive_type drive);
	void set_seccount_reg(bus_base bus, uint8_t seccount);
	void set_LBA_reg(bus_base bus, uint32_t lba);
	void set_command_reg(bus_base bus, cmd_type cmd);

	bool chk_BSY_till_cleared(bus_base bus);
	bool chk_DRQ_till_set(bus_base bus);
	port_t ide_port(bus_base bus, port_offset poff);

	port_t ide_port(bus_base bus, port_offset poff)
	{
		return bus + poff;
	}


	uint16_t buffer[256];

	const uint16_t * cmd_identify_drive(bus_base bus, drive_type drive)
	{
		//log(OK, "In cmd_identify_drive()\n");
		//select drive
		set_drive_reg(bus, drive);

		//make these registers zero
		outportb(ide_port(bus, SEC_COUNT), 0);
		outportb(ide_port(bus, LBA_LOW), 0);
		outportb(ide_port(bus, LBA_MID), 0);
		outportb(ide_port(bus, LBA_HI), 0);
		outportb(ide_port(bus, WR_CMD), IDENTIFY_DRIVE);

		//issued command
		uint8_t status = inportb(ide_port(bus, RD_REG_STATUS));

		//magic_breakpoint_msg("issued command\n");
		uint16_t * ret_val;
		if (status == 0)
		{
			ret_val = nullptr;	
			magic_breakpoint_msg("drive does not exist\n");
		}
		else
		{
			//magic_breakpoint_msg("checking BSY bit\n");
			chk_BSY_till_cleared(bus);
			//magic_breakpoint_msg("BSY bit is cleared\n");


			if (inportb(ide_port(bus, LBA_MID)) || inportb(ide_port(bus, LBA_HI)))
			{
				magic_breakpoint_msg("NOT ATA!\n");
				ret_val = nullptr;	//drive is not ATA
			}

			//drive is ATA: read 256 bytes of data

			//magic_breakpoint_msg("checking DRQ\n");
			chk_DRQ_till_set(bus);
			//magic_breakpoint_msg("DRQ is set\n");
			//magic_breakpoint_msg("reading 256 words\n");
			rep_inportw(ide_port(bus, DATA), buffer, 256);

			status = inportb(ide_port(bus, RD_REG_STATUS));
			if (status & ERR)
			{
				magic_breakpoint_msg("Error in reading identify block\n");
			}
			else
			{
				//magic_breakpoint_msg("reading done\n");
			}
			ret_val = buffer;
		}
		//log(OK, "Out cmd_identify_drive()\n");
		return ret_val;
	}

	void set_LBA_reg(bus_base bus, uint32_t lba)
	{
		outportb(ide_port(bus, LBA_LOW), uint8_t(lba));
		outportb(ide_port(bus, LBA_MID), uint8_t(lba >> 8));
		outportb(ide_port(bus, LBA_HI), uint8_t(lba >> 16));
	}

	void set_command_reg(bus_base bus, cmd_type cmd)
	{
		outportb(ide_port(bus, WR_CMD), cmd);
	}

	void set_seccount_reg(bus_base bus, uint8_t seccount)
	{
		outportb(ide_port(bus, SEC_COUNT), seccount);
	}

	void set_drive_reg(bus_base bus, drive_type drive)
	{
		uint8_t dhsel_reg = inportb(ide_port(bus, DRIVE_HEAD_SEL));
		switch (drive)
		{
		case MASTER:
			outportb(ide_port(bus, DRIVE_HEAD_SEL), dhsel_reg & 0xEF);
			break;
		case SLAVE:
			outportb(ide_port(bus, DRIVE_HEAD_SEL), dhsel_reg | 0x10);
			break;
		}
	}

	bool chk_BSY_till_cleared(bus_base bus)
	{
		uint8_t status;
		do
		{
			status = inportb(ide_port(bus, RD_REG_STATUS));
		} while ((status & BSY) != 0);

		return false;
	}

	bool chk_DRQ_till_set(bus_base bus)
	{
		uint8_t status;
		do
		{
			status = inportb(ide_port(bus, RD_REG_STATUS));
		} while ((status & DRQ) == 0);
		return true;
	}

	bool cmd_read_sector(bus_base bus, drive_type drive, uint32_t lba, uint16_t *outbuf)
	{
		//magic_breakpoint_msg("In cmd_read_sector()\n");

		chk_BSY_till_cleared(bus);

		//magic_breakpoint_msg("BSY bit cleared\n");
		//magic_breakpoint_msg("Setting parameters\n");
		set_LBA_reg(bus, lba);
		set_seccount_reg(bus, 1);
		
		uint8_t dhsel_regdata;

		switch (drive)
		{
		case MASTER:
			dhsel_regdata = 0xE0;
			break;
			dhsel_regdata = 0xF0;
		case SLAVE:
			break;
		}

		outportb(ide_port(bus, DRIVE_HEAD_SEL), dhsel_regdata);

		//magic_breakpoint_msg("issued command\n");
		set_command_reg(bus, READ_SECTORS);

		//magic_breakpoint_msg("Checking DRQ bit\n");

		chk_DRQ_till_set(bus);

		bool ret;

		uint8_t status = inportb(ide_port(bus, RD_REG_STATUS));

		if ((status & ERR) == 0)
		{
			rep_inportw(ide_port(bus, DATA), outbuf, 256);
			status = inportb(ide_port(bus, RD_REG_STATUS));

			if (status & ERR)
			{
				magic_breakpoint_msg("Error after reading\n");
				ret = false;
			}
			else
			{
				//log(OK, "Reading ok\n");
				ret = true;
			}
		}
		else
		{
			magic_breakpoint_msg("Error before reading\n");
			ret = false;
		}
		//log(OK, "Out cmd_read_sector()\n");
		return ret;

	}

	bool cmd_write_sector(bus_base bus, drive_type drive, uint32_t lba, uint16_t *data)
	{
		//log(OK, "In cmd_write_sector()\n");
		chk_BSY_till_cleared(bus);

		//magic_breakpoint_msg("BSY bit cleared             \n");

		//magic_breakpoint_msg("Setting parameters               \n");

		set_seccount_reg(bus, 1);
		set_LBA_reg(bus, lba);

		uint8_t dhsel_regdata;
		switch (drive)
		{
		case MASTER:
			dhsel_regdata = 0xE0;
			break;
			dhsel_regdata = 0xF0;
		case SLAVE:
			break;
		}

		outportb(ide_port(bus, DRIVE_HEAD_SEL), dhsel_regdata);

		//magic_breakpoint_msg("issued command\n");

		set_command_reg(bus, WRITE_SECTORS);

		//magic_breakpoint_msg("Checking DRQ bit\n");
		chk_DRQ_till_set(bus);
		//magic_breakpoint_msg("DRQ is now set\n");
		uint8_t status = inportb(ide_port(bus, RD_REG_STATUS));
		//magic_breakpoint_msg("Status checked\n");
		bool res = false;
		if ((status & ERR) == 0)
		{
			//magic_breakpoint_msg("Before transferring data...\n");
			rep_outportw(ide_port(bus, DATA), data, 256);
			//magic_breakpoint_msg("After transferring data...\n");
			status = inportb(ide_port(bus, RD_REG_STATUS));

			if ((status & ERR) == 0)
			{
				res = true;
				//log(OK, "Writing ok\n");
			}
			else
			{
				magic_breakpoint_msg("Error during writing\n");
				res = false;
			}
		}
		else
		{
			//magic_breakpoint_msg("Error before writing\n");
		}
		//log(OK, "Out cmd_write_sector()\n");
		return res;
	}
}