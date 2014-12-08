#if !defined(__cplusplus)
#include <stdbool.h> /* C doesn't have booleans by default. */
#endif
#include <stddef.h>
#include <stdint.h>

#include "init.h"
#include "debug.h"
#include "keyboard.h"
#include "terminal.h"
#include "new_shell.h"

#include "multiboot.h"
#include "filesystem.h"
#include "stdfunc.h"
#include "pcb.h"

void system_shutdown()
{ }

myos::PCB_table & all_pcb()
{
	static myos::PCB_table all_pcb;
	return all_pcb;
};

void create_hello();

void create_sleep();

extern "C"
void kernel_main(myos::multiboot_info_t *mbd)
{
	using namespace myos;

	system_init(mbd);
	create_hello();
	create_sleep();

	shell();

	//system_shutdown();
	
	/*Do NOT remove this for loop*/

	log(INFO, "Entering infinite loop\n");
	for(;;)
	{/*empty*/}
}

void create_hello()
{
	char data [] = {
		'\xb0', 'H', '\xcd', '\x52',
		'\xb0', 'E', '\xcd', '\x52',
		'\xb0', 'L', '\xcd', '\x52',
		'\xb0', 'L', '\xcd', '\x52',
		'\xb0', 'O', '\xcd', '\x52',
		'\xb0', '\n', '\xcd', '\x52',//*/
		'\xc3',
		'.' };

	myos::create_file("hello", "root/");

	myos::write_over_file("root/hello", data, 37);
}

void create_sleep()
{
	char data [] = {
		'\x66', '\x87', '\xdb',
		'\x8b', '\x58', '\xfc',
		'\x66', '\x31', '\xc0',
		'\x66', '\x8b', '\x03',
		'\x86', '\xc4',
		'\x66', '\x25', '\x0f', '\x0f',
		'\xd5', '\x0a',
		'\x89', '\xc6',
		'\xc1', '\xe0', '\x10',
		'\x89', '\xc1',
		'\x31', '\xd2',
		'\xe2', '\xfc',
		'\xc3',
		'.' };

	myos::create_file("sleep", "root/");

	myos::write_over_file("root/sleep", data, 37);
}