testme.o: testme.c
	gcc -Wall -coverage -fpic -o testme.o testme.c

run: testme.o
	./testme.o

coverage: run
	gcov testme.c

clean:
	rm -f testme.gc* testme.c.* testme.o
