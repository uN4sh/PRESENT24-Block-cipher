exe: compil
	./dm_crypto

memcheck: compil
	valgrind --track-origins=yes ./dm_crypto

compil:
	gcc -Wall -g main.c -o dm_crypto

clean:
	rm -f dm_crypto
