#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TAILLE_MOT 7
#define NB_K 10000 // 2^24 = 16777216

int CHIFFREMENT(char *etat_hex, char *cle_maitre_hex, char *cipher);
int DECHIFFREMENT(char *chiffre_hex, char *cle_maitre_hex, char *clair);


/**
 * Attaque par le milieu :
 * On a (m, c) et on cherche (k1, k2)
 * 
 * Nombre de clés : 
 * // DES : longueur de la clé : 56 bits, nombre de clés : 2^56
 * // PRESENT-24 : 6 caractères, alphabet de 16 possibles, 24 bits -> 2^24 ?
 * 
 *  1. Construire deux listes Lc et Lm tel que :
 *      Pour tout k parmi 2^24, stocker le résultat du   chiffrement de m dans Lm  (clair | clé) 
 *      Pour tout k parmi 2^24, stocker le résultat du déchiffrement de c dans Lc  (chiffré | clé) 
 * 
 *  2. Chercher les éléments communs dans les deux listes :
 *      t tel que : .... 
 *      Méthode :
 *          - Trier les deux listes - O(2 * n*log n)
 *          - Chercher les éléments communs - O(n)
 *          Avec n = 2^24
 *          
 *  Taille en mémoire : 2 listes de 2^24 -> 2^25 en mémoire
 *  Taille en temps :   2 * 2^24 * log(2^24) = 2 * 24 * 2^24 = 48 * 2^24
 */



void genere_listes(char *clair, char ***lm, char *chiffre, char ***lc)  { 
    for (int i = 0; i < NB_K; i++)  {
        snprintf ( lm[i][1], TAILLE_MOT+1, "%06x", i );
        snprintf ( lc[i][1], TAILLE_MOT+1, "%06x", i );
        CHIFFREMENT(clair, lm[i][1], lm[i][0]);
        DECHIFFREMENT(chiffre, lc[i][1], lc[i][0]);
    }
}

// Tri des listes ? Par valeurs m / c ? Sont actuellement triées par clés
void quicksort(char ***lm, char ***lc)  {

}

// Recherche d'éléments communs dans les listes (lm[1] == lc[0]), et afficher les 2 clés k1, k2 correspondantes
void research(char ***lm, char ***lc)  {

}


// ELYN (m1,c1) = (16a0e6, dcc916) (m2,c2) = (332962,cfeee9)
// FM   (m1,c1) = (02c315, 88b6ed) (m2,c2) = (1d2dec,c4bb7a)
int main(int argc, char const *argv[])  {

    // Listes Lm et Lc : 2^24 colonnes de 2 mots chacunes : (clair | clé) et (chiffré | clé)
    char *** lm = (char *** ) malloc(NB_K * sizeof(char ** ));
    char *** lc = (char *** ) malloc(NB_K * sizeof(char ** ));

    // Allocation de mémoire pour les 2 listes
    for(int i = 0 ; i < NB_K ; i++ )  { 
        lm[i] = (char ** ) malloc(2 * sizeof(char * )) ;
        lc[i] = (char ** ) malloc(2 * sizeof(char * )) ;
        for ( int j = 0 ; j < 2 ; j++ )  { 
            lm[i][j] = (char *) malloc (TAILLE_MOT * sizeof(char));
            lc[i][j] = (char *) malloc (TAILLE_MOT * sizeof(char));
        }
    }

    char *clair = "16a0e6";
    char *chiffre = "dcc916";

    clock_t begin = clock();

    genere_listes(clair, lm, chiffre, lc);
    printf("              Clair  |   Chiffré\n");
    printf("    \033[33;1mCouple :  \033[33;1m%s |    %s\033[0m\n\n", clair, chiffre);
    printf("    \033[34m   Clé |\033[0m Chiffré | Déchiffré\033[0m\n");
    for (int i = 0; i < 100; i++)  {
        printf("\033[32m%02d: \033[34m%s |\033[32m  %s |    %s\033[00m\n", i, lm[i][1], lm[i][0], lc[i][0]);
    }
    printf("-\n");
    printf("\033[32m%02d: \033[34m%s |\033[32m  %s |    %s\033[00m\n", NB_K-1, lm[NB_K-1][1], lm[NB_K-1][0], lc[NB_K-1][0]);

    // Mesure du temps écoulé
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Temps écoulé: %fs for %d keys\n", time_spent, NB_K);
    

    // Libération de la mémoire des deux listes
    for(int i = 0 ; i < NB_K ; i++ )  {  
        for ( int j = 0 ; j < 2 ; j++ )  { 
            free(lm[i][j]);
            free(lc[i][j]);
        } 
    }

    for(int i = 0 ; i < NB_K ; i++ )  {  
        free(lm[i]);
        free(lc[i]);
    }

    free(lm);
    free(lc);
    
    return 0;
}

