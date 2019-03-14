CC=gcc 

all: mysh2 myls

mysh2: mysh2.o 
	$(CC) -o mysh2 mysh2.o 

mysh2.o: mysh2.c 
	$(CC) -c mysh2.c 

myls: myls.o 
	$(CC) -o myls myls.o 

myls.o: myls.c 
	$(CC) -c myls.c 