#include "new_shell.h"
#include "stdfunc.h"
#include "keyboard.h"
#include "terminal.h"
#include "process.h"
#include "debug.h"
#include "filesystem.h"

namespace myos
{
	char shell_buf[256];
	int shell_buf_len = 0;
	bool SHELL_RUNNING = false;
	void read_with_echo(char *buf, size_t bufsz, char delim)
	{
		for (size_t i = 0; i < bufsz - 1; i++)
		{
			char c = kbd_getchar();

			if (c == delim)
			{
				terminal_putchar('\n');
				buf[i] = '\0';
				break;
			}
			else
			{
				terminal_putchar(c);
				buf[i] = c;
			}
		}

		buf[bufsz - 1] = '\0';
	}

	int get_command();
	bool run_command(int cmd_code);

	bool shell_run();
	bool shell_exit();
	bool shell_donothing();
	bool shell_format();
	bool shell_readfile();
	bool shell_writefile();
	bool shell_list();
	bool shell_crash();

	bool shell()
	{
		while (1)
		{
			int user_cmd = get_command();
			bool run_ok = run_command(user_cmd);

			if (run_ok == false)
			{
				return false;
			}
		}
	}


	int get_command()
	{
		char command[21];
		terminal_writestring("\n$ ");
		read_with_echo(command, 21, '\n');
		
		//log(INFO, "command = ", command, '\n');

		if (stricmp(command, "EXIT") == 0)
		{
			return 0;
		}
		else if (stricmp(command, "NIL") == 0)
		{
			return 1;
		}
		else if (stricmp(command, "RUN") == 0)
		{
			return 2;
		}
		else if (stricmp(command, "FORMAT") == 0)
		{
			return 3;
		}
		else if (stricmp(command, "READFILE") == 0)
		{
			return 4;
		}
		else if (stricmp(command, "WRITEFILE") == 0)
		{
			return 5;
		}
		else if (stricmp(command, "LIST") == 0)
		{
			return 6;
		}
		else
		{
			return -1;
		}
	}

	bool run_command(int cmd_code)
	{
		switch (cmd_code)
		{
		case 0:
			return shell_exit();
		case 1:
			return shell_donothing();
		case 2:
			return shell_run();
		case 3:
			return shell_format();
		case 4:
			return shell_readfile();
		case 5:
			return shell_writefile();
		case 6:
			return shell_list();
		default:
			return shell_crash();
		}
	}

	bool shell_exit()
	{
		terminal_writestring("\nShell exiting normally\n");
		return false;
	}

	bool shell_crash()
	{
		terminal_writestring("\nUnidentified command - shell crashing\n");
		return true;
	}

	bool shell_donothing()
	{
		terminal_writestring("did nothing \n");
		return true;
	}

	bool shell_run()
	{
		running.pid = NULL_PID;

		add_program("root/hello");
		add_program("root/hello");
		add_program("root/sleep 12");
		add_program("root/sleep 01");

		SHELL_RUNNING = true;
		
		while (SHELL_RUNNING == true)
			;
			
		return true;
	}

	bool shell_format()
	{
		terminal_writestring("Do you want to format the hard disk? (y/n) : ");
		char c = kbd_getchar();
		terminal_putchar(c);

		if (tolower(c) == 'y')
		{
			format();
			terminal_writestring("Hard disk is formatted with 'Kitto' filesystem\n");
		}
		else
		{
			terminal_writestring("Format operation is cancelled\n");
		}

		return true;
	}

	bool shell_readfile()
	{
		char file_name[128];
		terminal_writestring("Enter filepath to read : ");
		read_with_echo(file_name, 128, '\n');

		int filesz = get_file_size(file_name);

		char buf[4096];

		if (read_from_file(file_name, buf, 4096) == false)
		{
			terminal_writestring("Read operation failed \n");
		}
		else
		{
			buf[filesz] = '\0';
			terminal_writestring("File contents are - \n");
			terminal_writestring(buf);
			terminal_writestring("\n");
		}
		return true;
	}

	bool shell_writefile()
	{
		char file_name[128];
		terminal_writestring("Enter filepath to write : ");
		read_with_echo(file_name, 128, '\n');

		if (is_file_exists(file_name))
		{
			terminal_writestring("File already exists! Do you want to overwrite? (y/n) : ");

			char c = kbd_getchar();
			terminal_putchar(c);

			if (tolower(c) != 'y')
				return true;
		}
		else
		{
			char pathbuf[200];

			int len = strncpy(pathbuf, 200, file_name);

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

			//magic_breakpoint_msg(file_name, " broken at ", path_breaker, '\n');

			if (path_breaker < 0)
			{
				log(ERROR, "No '/' in filepath \n");
				return true;
			}
			else
			{
				char actual_name[80];
				strncpy(actual_name, 80, pathbuf + path_breaker + 1, '\0');
				char t = pathbuf[path_breaker + 1];
				pathbuf[path_breaker + 1] = '\0';


				log(OK, "name : ", actual_name, '\n');
				log(OK, "path : ", pathbuf, '\n');
				bool is_ok = create_file(actual_name, pathbuf);

				if (is_ok == false)
				{
					log(ERROR, "File could not be created \n");
					return true;
				}

				pathbuf[path_breaker + 1] = t;
			}
		}

		log(OK, "file_name = ", file_name, '\n');

		char buf[4096];
		read_with_echo(buf, 4096, '.');

		if (!is_file_exists(file_name))
		{
			log(OK, file_name, " is invalid file\n");
		}
		else
		{
			log(OK, file_name, " is valid file\n");
		}
		write_over_file(file_name, buf, strlen(buf));

		return true;
	}

	bool shell_list()
	{
		char buf[80];

		terminal_writestring("Enter directory : ");
		read_with_echo(buf, 80, '\n');

		list_directory_contents(buf);


		return true;
	}

	void xfer_to_shell_buffer()
	{
		static bool newprog = true;

		if (newprog == true)
		{
			terminal_writestring("> ");
			newprog = false;
		}
		//log(INFO, "In xfer to shell buffer\n");
		while (pending_keystrokes() == true)
		{
			//log(INFO, "transfer to shell buf\n");
			char c = kbd_getchar();
			shell_buf[shell_buf_len++] = c;
			terminal_putchar(c);

			if (c == '\n')
			{
				newprog = true;
			}
		}
	}

	void extract_programs_from_shell_buffer()
	{
		int i = 0;
		while (i < shell_buf_len)
		{
			if (shell_buf[i] == '\n')
			{
				shell_buf[i] = '\0';
				int plen = i;
				
				if(stricmp(shell_buf, "shellexit") == 0)
				{
					SHELL_RUNNING = false;
				}
				
				add_program(shell_buf);
				memcpy(shell_buf, shell_buf + plen + 1, shell_buf_len - plen - 1);

				shell_buf_len -= plen + 1;
				i = 0;
			}
			else
			{
				i++;
			}
		}
	}
}