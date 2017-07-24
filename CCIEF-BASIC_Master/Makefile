Master_sample: SLMP.o SOCKET.o TIMER.o CCIEF_BASIC_SLAVES.o CCIEF_BASIC_MASTER.o USER_SAMPLE.o
	gcc -o Master_sample SLMP.o SOCKET.o TIMER.o CCIEF_BASIC_SLAVES.o CCIEF_BASIC_MASTER.o USER_SAMPLE.o

SLMP.o: library/src/SLMP.c
	gcc -I library/include -c library/src/SLMP.c
SOCKET.o: sample/src/SOCKET.c
	gcc -I sample/include -c sample/src/SOCKET.c
TIMER.o: sample/src/TIMER.c
	gcc -I sample/include -c sample/src/TIMER.c
CCIEF_BASIC_SLAVES.o: sample/src/CCIEF_BASIC_SLAVES.c
	gcc -I sample/include -c sample/src/CCIEF_BASIC_SLAVES.c
CCIEF_BASIC_MASTER.o: sample/src/CCIEF_BASIC_MASTER.c
	gcc -I sample/include -Ilibrary/include -c sample/src/CCIEF_BASIC_MASTER.c
USER_SAMPLE.o: sample/src/USER_SAMPLE.c
	gcc -I sample/include -Ilibrary/include -c sample/src/USER_SAMPLE.c
clean:
	rm SLMP.o SOCKET.o TIMER.o CCIEF_BASIC_SLAVES.o CCIEF_BASIC_MASTER.o USER_SAMPLE.o