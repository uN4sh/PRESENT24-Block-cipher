exe: compil_main
	./dm_crypto

debug: compil_main
	valgrind --track-origins=yes ./dm_crypto

attaque: compil_attaque
	./dm_crypto_attaque

debug_attaque: compil_attaque
	valgrind --track-origins=yes ./dm_crypto_attaque

# Compilation 
compil_main: main.o chiffrement.o dechiffrement.o 
	gcc -O3 -Wall -pg -g -o dm_crypto main.o chiffrement.o dechiffrement.o 

compil_attaque: attaque.o chiffrement.o dechiffrement.o 
	gcc -O3 -Wall -pg -g -o dm_crypto_attaque attaque.o chiffrement.o dechiffrement.o 


# Compilation objet
attaque.o: src/attaque.c src/chiffrement.c src/dechiffrement.c
	gcc -O3 -Wall -pg -g -c -o attaque.o src/attaque.c 

main.o: src/main.c src/chiffrement.c src/dechiffrement.c
	gcc -O3 -Wall -pg -g -c -o main.o src/main.c 

chiffrement.o: src/chiffrement.c 
	gcc -O3 -Wall -pg -g -c -o chiffrement.o src/chiffrement.c

dechiffrement.o: src/dechiffrement.c src/chiffrement.c
	gcc -O3 -Wall -pg -g -c -o dechiffrement.o src/dechiffrement.c


# Utilitaire
clean:
	rm -f dm_crypto
	rm -f dm_crypto_attaque
	rm -f *.o
	rm -f gmon.out
