exe: compil
	./dm_crypto

memcheck: compil
	valgrind ./dm_crypto

compil:
	gcc -g main.c -o dm_crypto

clean:
	rm -f dm_crypto
