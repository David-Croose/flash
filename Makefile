all:
	gcc -Wall -O0 -g -o demo demo.c flash.c port.c
clean:
	rm -f demo.exe
