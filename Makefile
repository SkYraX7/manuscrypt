SRC	=	conv.h conv.c main.c
OBJ	=	conv.o main.o
PROG	=	runnable
LFLAGS	=	-lssl -lcrypto -lm
$(PROG):	$(OBJ)
		gcc $(OBJ)	-o	$(PROG)	$(LFLAGS)

clean:
	-rm -f	*.o	$(OBJ)	runnable
