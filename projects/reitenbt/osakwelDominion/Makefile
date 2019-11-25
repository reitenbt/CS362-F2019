CFLAGS= -Wall -fpic -coverage -lm -std=c99

rngs.o: rngs.h rngs.c
	gcc -c rngs.c -g  $(CFLAGS)

dominion.o: dominion.h dominion.c rngs.o
	gcc -c dominion.c -g  $(CFLAGS)

playdom: dominion.o playdom.c
	gcc -o playdom playdom.c -g dominion.o rngs.o $(CFLAGS)
#To run playdom you need to entere: ./playdom <any integer number> like ./playdom 10*/

testDrawCard: testDrawCard.c dominion.o rngs.o
	gcc  -o testDrawCard -g  testDrawCard.c dominion.o rngs.o $(CFLAGS)

interface.o: interface.h interface.c
	gcc -c interface.c -g  $(CFLAGS)

runtests: testDrawCard 
	./testDrawCard &> unittestresult.out
	gcov dominion.c >> unittestresult.out
	cat dominion.c.gcov >> unittestresult.out


player: player.c interface.o
	gcc -o player player.c -g  dominion.o rngs.o interface.o $(CFLAGS)

all: playdom player 

randomtestresults: dominion.o rngs.o
	gcc -coverage -o randomtestcard1.exe randomtestcard1.c dominion.o rngs.o $(CFLAGS)
	gcc -coverage -o randomtestcard2.exe randomtestcard2.c dominion.o rngs.o $(CFLAGS)
	gcc -o randomtestcard3.exe randomtestcard3.c dominion.o rngs.o -coverage $(CFLAGS)
	./randomtestcard1.exe &>> randomtestresults.out
	gcov -b -f randomtestcard1.c &>> randomtestresults.out
	./randomtestcard2.exe &>> randomtestresults.out
	gcov -b -f randomtestcard2.c &>> randomtestresults.out
	./randomtestcard3.exe &>> randomtestresults.out
	gcov -b -f randomtestcard3.c &>> randomtestresults.out

clean:
	rm -f *.o playdom.exe playdom player player.exe  *.gcov *.gcda *.gcno *.so *.out testDrawCard testDrawCard.exe randomtestcard*.exe
	rm -f randomtestcard*.exe randomtestresults
