# DM :  Implantation de l’attaque par le milieu contre un chiffrement par bloc

Implantation en C du chiffrement par bloc PRESENT-24 et de l’attaque par le milieu sur la version double de ce chiffrement.

## Contenu de l'archive

Cette archive contient le répertoire `src/` contenant les fichiers sources du programme ; une mécanisme de construction (`Makefile`), un fichier `.pdf` détaillant les choix d'implémentation et l'attaque par le milieu, ainsi que de ce présent fichier `README.md`.

## Commandes et utilisation

1. Exécuter le main de test de chiffrements et déchiffrements :

```sh
 make
```

2. Exécuter l'attaque par le milieu avec les couples clair/chiffré (~1 minute) :

```sh
make attaque
```

3. Exécuter en mode debug (Valgrind et mem-check)

```sh
make debug
```

## Précisions

1. La compilation de chacun des fichiers utilise l'option d'optimisation `gcc -O3` pour accélérer les instructions de chiffrement et de tri rapide lors de l'exécution de l'attaque.

2. Le fichier `chiffrement.c` contient l'ancienne version inutilisée du code à partir de la ligne 180.
