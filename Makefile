
EXEC=bin2bin

CPP=g++
CFLAGS=-Wall -O3
LFLAGS=-static

OBJ= \
	bin2bin.o \

bin2bin:	$(OBJ)
	$(CPP) $(LFLAGS) -o $(EXEC) $(OBJ) 

bin2bin.o	:	bin2bin.cpp 
	$(CPP) $(CFLAGS) -c bin2bin.cpp

clean:
	-rm -f $(OBJ) 
	-rm -f $(EXEC) 
