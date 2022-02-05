.phony all:
all: disk

disk:
	gcc -Wall diskinfo.c -lreadline -o diskinfo -g
	gcc -Wall disklist.c -lreadline -o disklist -g
	gcc -Wall diskget.c -lreadline -o diskget -g
	gcc -Wall diskput.c -lreadline -o diskput -g 

.PHONY clean:
clean:
	-rm -rf *.o *.exe
