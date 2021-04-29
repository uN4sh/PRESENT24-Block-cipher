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
	gcc -Wall -pg -g -o dm_crypto main.o chiffrement.o dechiffrement.o 

compil_attaque: attaque.o chiffrement.o dechiffrement.o 
	gcc -Wall -pg -g -o dm_crypto_attaque attaque.o chiffrement.o dechiffrement.o 


# Compilation objet
attaque.o: attaque.c chiffrement.c dechiffrement.c
	gcc -Wall -pg -g -c -o attaque.o attaque.c 

main.o: main.c chiffrement.c dechiffrement.c
	gcc -Wall -pg -g -c -o main.o main.c 

chiffrement.o: chiffrement.c 
	gcc -Wall -pg -g -c -o chiffrement.o chiffrement.c

dechiffrement.o: dechiffrement.c chiffrement.c
	gcc -Wall -pg -g -c -o dechiffrement.o dechiffrement.c


# Utilitaire
clean:
	rm -f dm_crypto
	rm -f dm_crypto_attaque
	rm -f *.o
