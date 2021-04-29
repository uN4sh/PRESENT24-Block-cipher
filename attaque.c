#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define TAILLE_MOT 7
#define NB_K 1000000 // 2^24 = 16777216

int CHIFFREMENT(char *etat_hex, char *cle_maitre_hex, char *cipher);
int DECHIFFREMENT(char *chiffre_hex, char *cle_maitre_hex, char *clair);
int CHIFFREMENT_DOUBLE(char *message, char *cle_k1, char *cle_k2, char *cipher);
int CHIFFREMENT2(char *etat_hex, char *cle_maitre_hex, char *cipher);

/**
 * Attaque par le milieu :
 * On a (m, c) et on cherche (k1, k2)
 * 
 * Nombre de clés : 
 * // DES : longueur de la clé : 56 bits, nombre de clés : 2^56
 * // PRESENT-24 : 6 caractères, alphabet de 16 possibles, 24 bits -> 2^24 ?
 * 
 *  1. Construire deux listes Lc et Lm tel que :
 *      Pour tout k parmi 2^24, stocker le résultat du   chiffrement de m dans Lm  (chiffré | clé) 
 *      Pour tout k parmi 2^24, stocker le résultat du déchiffrement de c dans Lc  (clair | clé) 
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



void genere_listes(char *clair, int **lm_int, char *chiffre, int **lc_int)  { 
    char buf[TAILLE_MOT+1];
    char k[TAILLE_MOT+1];
    for (int i = 0; i < NB_K; i++)  {
        lm_int[i][1] = i; 
        // lc_int[i][1] = i;
        snprintf ( k, TAILLE_MOT+1, "%06x", i );
        lm_int[i][0] =   CHIFFREMENT2(clair  , k, buf);
        // lc_int[i][0] = DECHIFFREMENT(chiffre, k, buf);
    }
}

// Tri des listes ? Par valeurs m / c ? Sont actuellement triées par clés
void quicksort_lists(int **list, int first, int last)  {
    int i, j, pivot;
    int *temp;

    if(first<last){
        pivot=first;
        i=first;
        j=last;

        while(i<j){
            while(list[i][0]<=list[pivot][0]&&i<last)
                i++;
            while(list[j][0]>list[pivot][0])
                j--;
            if(i<j)  {
                temp = list[i];
                list[i] = list[j];
                list[j] = temp;
            }
        }

        temp=list[pivot];
        list[pivot]=list[j];
        list[j]=temp;
        quicksort_lists(list,first,j-1);
        quicksort_lists(list,j+1,last);
    }
}


void research_int(int **lm, int **lc, char *m2, char *c2)  {
    int i = 0, j = 0;
    int collisions = 0;
    char output[TAILLE_MOT-1], k1[TAILLE_MOT-1], k2[TAILLE_MOT-1];

    while(i < NB_K && j < NB_K)  {
        if (lc[i][0] < lm[j][0])  {
            i++;
        }
        else if(lm[j][0] < lc[i][0])  {
            j++;
        }
        else  {
            // printf("Élément commun: %06x | Clé candidate: (%06x, %06x)\n", lm[j][0], lm[j][1], lc[i][1]);
            snprintf ( k1, TAILLE_MOT+1, "%06x", lm[i][1] );
            snprintf ( k2, TAILLE_MOT+1, "%06x", lc[i][1] );
            CHIFFREMENT_DOUBLE(m2, k1, k2, output);
            collisions++;
            if (!strcmp(output, c2))  {
                printf("\033[32mClé fonctionnelle trouvée: %06x (%06x, %06x)\033[0m\n", lm[j][0], lm[j][1], lc[i][1]);
            }
            i++;
            j++;
        }
    }
    printf("Fin de la recherche, %d collisions rencontrées.\n", collisions);
}

// ELYN (m1,c1) = (16a0e6, dcc916) (m2,c2) = (332962,cfeee9)
// FM   (m1,c1) = (02c315, 88b6ed) (m2,c2) = (1d2dec,c4bb7a)
// TEST (m1, c1) =  (198ad6, d4fbec), (k1, k2) = (000023, 0000aa) 

int main(int argc, char const *argv[])  {
    // 2 listes en entiers
    int **lm_int = (int ** ) malloc(NB_K * sizeof(int * ));
    int **lc_int = (int ** ) malloc(NB_K * sizeof(int * ));
    for(int i = 0 ; i < NB_K ; i++ )  { 
        lm_int[i] = (int * ) malloc(2 * sizeof(int));
        lc_int[i] = (int * ) malloc(2 * sizeof(int));
    }


    char couple_elyn[4][7] = {"16a0e6", "dcc916", "332962", "cfeee9"};
    // char couple_test[2][7] = {"198ad6", "d4fbec"};

    char *clair = couple_elyn[0];
    char *chiffre = couple_elyn[1];
    
    // Mode listes d'entiers
    clock_t begin = clock();

    genere_listes(clair, lm_int, chiffre, lc_int);
    printf("              Clair  |   Chiffré\n");
    printf("    \033[33;1mCouple :  \033[33;1m%s |    %s\033[0m\n\n", clair, chiffre);
    printf("    \033[34m   Clé |\033[0m Chiffré | Déchiffré\033[0m\n");
    for (int i = 0; i < 10; i++)
        printf("\033[32m%03d: \033[34m%06x |\033[32m  %06x |    %06x\033[00m\n", i, lm_int[i][1], lm_int[i][0], lc_int[i][0]);
    printf("-\n");
    printf("\033[32m%02d: \033[34m%06x |\033[32m  %06x |    %06x\033[00m\n", NB_K-1, lm_int[NB_K-1][1], lm_int[NB_K-1][0], lc_int[NB_K-1][0]);

    // Mesure du temps écoulé
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Temps écoulé: %fs for %d keys\n", time_spent, NB_K);

    /*
    // Quicksort
    begin = clock();
    printf("\nQuicksort des deux listes:\n");
    quicksort_lists(lm_int, 0, NB_K-1);
    quicksort_lists(lc_int, 0, NB_K-1);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Temps écoulé: %fs for %d keys\n", time_spent, NB_K);

    
    begin = clock();
    printf("\nBegin research:\n");
    research_int(lm_int, lc_int, couple_elyn[2], couple_elyn[3]);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Temps écoulé: %fs for %d keys\n", time_spent, NB_K);
    */

    // Free des listes d'entiers
    for(int i = 0 ; i < NB_K ; i++ )  {  
        free(lm_int[i]);
        free(lc_int[i]);
    }
    free(lm_int);
    free(lc_int);
    
    return 0;
}

/*
// Mode listes de char
int main(int argc, char const *argv[])  {

    char couple_elyn[2][7] = {"16a0e6", "dcc916"};
    char couple_test[2][7] = {"198ad6", "d4fbec"};

    char *clair = couple_test[0];
    char *chiffre = couple_test[1];

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
  
    
    clock_t begin = clock();

    genere_listes(clair, lm, chiffre, lc);
    printf("              Clair  |   Chiffré\n");
    printf("    \033[33;1mCouple :  \033[33;1m%s |    %s\033[0m\n\n", clair, chiffre);
    printf("    \033[34m   Clé |\033[0m Chiffré | Déchiffré\033[0m\n");
    for (int i = 0; i < 10; i++)
        printf("\033[32m%03d: \033[34m%s |\033[32m  %s |    %s\033[00m\n", i, lm[i][1], lm[i][0], lc[i][0]);
    printf("-\n");
    printf("\033[32m%02d: \033[34m%s |\033[32m  %s |    %s\033[00m\n", NB_K-1, lm[NB_K-1][1], lm[NB_K-1][0], lc[NB_K-1][0]);
    
    // Mesure du temps écoulé
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Temps écoulé: %fs for %d keys\n", time_spent, NB_K);


    begin = clock();
    printf("\nBegin research:\n");
    research(lm, lc, "332962", "cfeee9");
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
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
*/
