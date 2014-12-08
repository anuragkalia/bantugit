OBJ = boot.o kernel.o terminal.o stdfunc.o gdt.o idt.o isr.o ide.o filesystem.o irq.o kbd.o timer.o init.o debug.o lock.o keyboard.o new_shell.o mmngr_phys.o  clock.o pcb.o process.o

bantu.iso : bantu.bin grub.cfg
		cp bantu.bin isodir/boot/bantu.bin
		cp grub.cfg isodir/boot/grub/grub.cfg
		grub-mkrescue --output=bantu.iso isodir

bantu.bin : linker.ld $(OBJ)
		i686-elf-g++ -T linker.ld -o $@ -ffreestanding -O2 -nostdlib $(OBJ) -lgcc



kernel.o : kernel.cpp init.h debug.h keyboard.h terminal.h new_shell.h multiboot.h filesystem.h stdfunc.h pcb.h
		i686-elf-g++ -c kernel.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

new_shell.o : new_shell.cpp new_shell.h stdfunc.h keyboard.h terminal.h process.h debug.h filesystem.h
		i686-elf-g++ -c new_shell.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
		
init.o : init.cpp init.h gdt.h idt.h isr.h irq.h io_asm.h filesystem.h terminal.h timer.h kbd.h multiboot.h mmngr_phys.h debug.h
		i686-elf-g++ -c init.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti		
		

filesystem.o : filesystem.cpp filesystem.h stdfunc.h ide.h clock.h debug.h
		i686-elf-g++ -c filesystem.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

ide.o : ide.cpp ide.h io_asm.h debug.h
		i686-elf-g++ -c ide.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti



keyboard.o : keyboard.cpp keyboard.h kbd.h lock.h stdfunc.h debug.h
		i686-elf-g++ -c keyboard.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

kbd.o : kbd.cpp kbd.h irq.h io_asm.h lock.h debug.h
		i686-elf-g++ -c kbd.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

clock.o : clock.cpp clock.h timer.h
		i686-elf-g++ -c clock.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

timer.o : timer.cpp timer.h irq.h debug.h process.h new_shell.h
		i686-elf-g++ -c timer.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

irq.o : irq.cpp idt.h types_asm.h debug.h
		i686-elf-g++ -c irq.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

isr.o : isr.cpp isr.h idt.h types_asm.h debug.h
		i686-elf-g++ -c isr.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
		
idt.o : idt.cpp idt.h stdfunc.h debug.h
		i686-elf-g++ -c idt.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

gdt.o : gdt.cpp gdt.h
		i686-elf-g++ -c gdt.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti



debug.o : debug.cpp debug.h terminal.h
		i686-elf-g++ -c debug.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

terminal.o : terminal.cpp terminal.h io_asm.h
		i686-elf-g++ -c terminal.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

lock.o : lock.cpp lock.h lock_asm.h debug.h
		i686-elf-g++ -c lock.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti
		
stdfunc.o : stdfunc.cpp stdfunc.h terminal.h
		i686-elf-g++ -c stdfunc.cpp -o $@ -std=c++11 -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

boot.o : boot.s
		i686-elf-as boot.s -o $@


mmngr_phys.o : mmngr_phys.cpp mmngr_phys.h paging_asm.h stdfunc.h
		i686-elf-g++ -c mmngr_phys.cpp -o $@ -std=gnu++0x -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

pcb.o : pcb.cpp pcb.h clock.h mmngr_phys.h stdfunc.h filesystem.h debug.h
		i686-elf-g++ -c pcb.cpp -o $@ -std=gnu++0x -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

process.o : process.cpp process.h pcb.h debug.h filesystem.h clock.h stdfunc.h process_asm.h pqueue.h
		i686-elf-g++ -c process.cpp -o $@ -std=gnu++0x -ffreestanding -O2 -Wall -Wextra -fno-exceptions -fno-rtti

		
		
clean: 
		rm *.o bantu.*