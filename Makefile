
assembler: assembler.o pre_processor.o labels.o table.o util.o
	gcc -o assembler assembler.o pre_processor.o labels.o table.o util.o

assembler.o: assembler.c assembler.h pre_processor.h labels.h table.h util.h
	gcc -c -Wall -ansi -pedantic assembler.c

pre_processor.o: pre_processor.c pre_processor.h util.h
	gcc -c -Wall -ansi -pedantic pre_processor.c

labels.o: labels.c labels.h util.h
	gcc -c -Wall -ansi -pedantic labels.c

table.o: table.c table.h
	gcc -c -Wall -ansi -pedantic table.c
