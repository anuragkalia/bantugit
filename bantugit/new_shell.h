#ifndef __NEW_SHELL_H
#define __NEW_SHELL_H

namespace myos
{
	bool shell(); //true if ok, false if crashes => can be extended by returning specific error codes

	void xfer_to_shell_buffer();
	void extract_programs_from_shell_buffer();

	extern bool SHELL_RUNNING;
}

#endif //__NEW_SHELL_H