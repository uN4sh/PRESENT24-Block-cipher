exe: compil
	./dm_crypto

memcheck: compil
	valgrind --track-origins=yes ./dm_crypto

compil: main.o chiffrement.o
	gcc -Wall -g -o dm_crypto main.o chiffrement.o dechiffrement.o

main.o: main.c chiffrement.c dechiffrement.c
	gcc -Wall -g -c -o main.o main.c

chiffrement.o: chiffrement.c 
	gcc -Wall -g -c -o chiffrement.o chiffrement.c

dechiffrement.o: dechiffrement.c 
	gcc -Wall -g -c -o dechiffrement.o dechiffrement.c

clean:
	rm -f dm_crypto
	rm -f *.o
