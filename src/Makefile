FLAGS = -Wall -std=c99 -D_GNU_SOURCE -g

all: main.c db.o sv.o ep.o
	gcc $(FLAGS) main.c db.o sv.o ep.o `mysql_config --cflags` `mysql_config --libs`

db.o: db.c db.h
	gcc $(FLAGS) -c db.c

sv.o: sv.c sv.h
	gcc $(FLAGS) -c sv.c

ep.o: ep.c ep.h
	gcc $(FLAGS) -c ep.c

clean:
	rm -f *.o *.out
