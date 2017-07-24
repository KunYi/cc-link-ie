Slave_sample: SLMP.o SOCKET.o TIMER.o CCIEF_BASIC_SLAVE.o SLMP_SERVER.o USER_SAMPLE.o
	$(CC) -o Slave_sample SLMP.o SOCKET.o TIMER.o CCIEF_BASIC_SLAVE.o SLMP_SERVER.o USER_SAMPLE.o

SLMP.o: ../../library/src/SLMP.c
	$(CC) -I ../../library/include -c ../../library/src/SLMP.c
SOCKET.o: ../../sample/src/SOCKET.c
	$(CC) -I ../../sample/include -c ../../sample/src/SOCKET.c
TIMER.o: ../../sample/src/TIMER.c
	$(CC) -I ../../sample/include -c ../../sample/src/TIMER.c
CCIEF_BASIC_SLAVE.o: ../../sample/src/CCIEF_BASIC_SLAVE.c
	$(CC) -I ../../sample/include -I ../../library/include -c ../../sample/src/CCIEF_BASIC_SLAVE.c
SLMP_SERVER.o: ../../sample/src/SLMP_SERVER.c
	$(CC) -I ../../sample/include -I ../../library/include -c ../../sample/src/SLMP_SERVER.c
USER_SAMPLE.o: ../../sample/src/USER_SAMPLE.c
	$(CC) -I ../../sample/include -I ../../library/include -c ../../sample/src/USER_SAMPLE.c
clean:
	rm  -f SLMP.o SOCKET.o TIMER.o CCIEF_BASIC_SLAVE.o SLMP_SERVER.o USER_SAMPLE.o Slave_sample
