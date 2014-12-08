#ifndef IDE_H
#define IDE_H

#include <stdint.h>

namespace myos
{
	enum bus_base {PRIMARY = 0x1F0, SECONDARY = 0x170};
	enum drive_type {MASTER, SLAVE};

	const uint16_t * cmd_identify_drive(bus_base bus, drive_type drive);
	bool cmd_read_sector(bus_base bus, drive_type drive, uint32_t lba, uint16_t *outbuf);
	bool cmd_write_sector(bus_base bus, drive_type drive, uint32_t lba, uint16_t *inbuf);
}
#endif //IDE_H

/********************Battle Plan*************************

1. Check existence of devices on primary/secondary buses for both master/slave drives.
2. If the devices exist, check whether they are hard drive/non-hard drive (or rather, ATA/ATAPI).
3. Check whether existing device is busy or not.
4. 


*********************************************************/