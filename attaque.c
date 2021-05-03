#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

#define TAILLE_MOT 7
#define NB_K 16777216 // 2^24 = 16777216

// int CHIFFREMENT(char *etat_hex, char *cle_maitre_hex, char *cipher);
// int DECHIFFREMENT(char *chiffre_hex, char *cle_maitre_hex, char *clair);
// int CHIFFREMENT_DOUBLE(char *message, char *cle_k1, char *cle_k2, char *cipher);

void CADENCEMENT_CLE_OPTI(uint32_t m_key, uint32_t *sous_cles);
int CHIFFREMENT_OPTI(uint32_t etat, uint32_t *sous_cles);
int DECHIFFREMENT_OPTI(uint32_t etat, uint32_t *sous_cles);
int CHIFFREMENT_DOUBLE_OPTI(uint32_t message, uint32_t k1, uint32_t k2);

/**
 * Attaque par le milieu :
 * On a (m, c) et on cherche (k1, k2)
 * 
 * Nombre de clés : 
 * // DES : longueur de la clé : 56 bits, nombre de clés : 2^56
 * // PRESENT-24 : longueur de la clé : 24 bits -> 2^24 = 16777216
 * 
 *  1. Construire deux listes Lc et Lm tel que :
 *      Pour tout k parmi 2^24, stocker le résultat du   chiffrement de m dans Lm  (chiffré | clé) 
 *      Pour tout k parmi 2^24, stocker le résultat du déchiffrement de c dans Lc  (clair   | clé) 
 * 
 *  2. Chercher les éléments communs dans les deux listes t tel quel :
 *      t = PRESENT   (m1)
 *      t = PRESENT-1 (c1)
 *      Méthode :
 *          - Trier les deux listes - O(2 * n*log n)
 *          - Chercher les éléments communs - O(n)
 *          Avec n = 2^24
 * 
 *  3. Pour chaque élément commun des deux listes :
 *      - Récupérer le couple de clé k1, k2
 *      - Chiffrer m2 avec 2-PRESENT24 et vérifier s'il est égal à c2
 *      - Si oui : le couple de clé k1, k2 est le couple fonctionnel
 *      - Si non : c'est une collision
 * 
 *  Taille en mémoire : 2 listes de 2^24 -> 2^25 en mémoire
 *  Taille en temps :   2 * 2^24 * log(2^24) = 2 * 24 * 2^24 = 48 * 2^24
 */


/**
 * @brief Génère les deux listes Lm et Lc pour tout k parmi 2^24.
 * 
 *         [0]   [1]
 * Lm  Chiffré | Clé
 * Lc    Clair | Clé
 * 
 * @param clair
 * @param lm_int
 * @param chiffré
 * @param lc_int
 * @return              Les deux listes sont modifiées par la fonction 
 */
void GENERER_LISTES_OPTI(uint32_t clair, int **lm_int, uint32_t chiffre, int **lc_int)  { 
    uint32_t sous_cles[11];
    for (int i = 0; i < NB_K; i++)  {
        CADENCEMENT_CLE_OPTI(i, sous_cles);
        lm_int[i][1] = i; 
        lc_int[i][1] = i;
        lm_int[i][0] = CHIFFREMENT_OPTI(clair, sous_cles);
        lc_int[i][0] = DECHIFFREMENT_OPTI(chiffre, sous_cles);
    }
}


/**
 * @brief Algorithme Quicksort: trie le tableau à double entrée par sa valeur (Position 1).
 *        Les listes sont initialement triées par clés (Position 0) puisque générées par une boucle.
 * 
 * @param list      Tableau à trier
 * @param first     Première position pour l'algorithme diviser pour régner
 * @param last      Dernière position pour l'algorithme diviser pour régner
 * @return          Le tableau en paramètre est modifié dans la fonction
 */
void QUICKSORT(int **list, int first, int last)  {
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
        QUICKSORT(list,first,j-1);
        QUICKSORT(list,j+1,last);
    }
}


/**
 * @brief Recherche d'éléments communs parmi les deux listes puis teste les clés candidates 
 *        avec le second couple (m2, c2) pour retrouver les clés (k1, k2) ayant servi au chiffrement.
 * 
 * @param lm
 * @param lc
 * @param m2
 * @param c2
 * @return      void
 */
void RECHERCHE_COMMUNS(int **lm, int **lc, uint32_t m2, uint32_t c2)  {
    int i = 0, j = 0;
    int collisions = 0;
    int res;
    while(i < NB_K && j < NB_K)  {
        if (lc[i][0] < lm[j][0])  {
            i++;
        }
        else if(lm[j][0] < lc[i][0])  {
            j++;
        }
        else  {
            res = CHIFFREMENT_DOUBLE_OPTI(m2, lm[j][1], lc[i][1]);
            collisions++;
            if (res == c2)  {
                printf("\033[32mClé fonctionnelle trouvée: %06x (%06x, %06x)\033[0m\n", lm[j][0], lm[j][1], lc[i][1]);
            }
            i++;
            j++;
        }
    }
    printf("Fin de la recherche, %d collisions rencontrées.\n", collisions);
}

void print_listes(int **lm_int, int **lc_int, uint32_t m1, uint32_t c1)  {
    printf("              Clair  |   Chiffré\n");
    printf("    \033[33;1mCouple :  \033[33;1m%06x |    %06x\033[0m\n\n", m1, c1);
    printf("Lm  \033[34m   Clé |\033[0m Chiffré |  Lc  \033[34m   Clé |\033[0m Déchiffré\033[0m\n");
    for (int i = 0; i < 10; i++)
        printf("\033[32m%02d: \033[34m%06x |\033[32m  %06x |      \033[34m%06x |\033[32m %06x\033[00m\n", i, lm_int[i][1], lm_int[i][0], lc_int[i][1], lc_int[i][0]);
    printf("\n");
    printf("\033[32m%02d: \033[34m%06x |\033[32m  %06x |    %06x\033[00m\n", NB_K-1, lm_int[NB_K-1][1], lm_int[NB_K-1][0], lc_int[NB_K-1][0]);
}


// ELYN (m1,c1) = (16a0e6, dcc916) (m2,c2) = (332962,cfeee9)
// FM   (m1,c1) = (02c315, 88b6ed) (m2,c2) = (1d2dec,c4bb7a)
int main(int argc, char const *argv[])  {
    clock_t tot_begin = clock();

    // 2 listes en entiers
    int **lm_int = (int ** ) malloc(NB_K * sizeof(int * ));
    int **lc_int = (int ** ) malloc(NB_K * sizeof(int * ));
    for(int i = 0 ; i < NB_K ; i++ )  { 
        lm_int[i] = (int * ) malloc(2 * sizeof(int));
        lc_int[i] = (int * ) malloc(2 * sizeof(int));
    }

    uint32_t couple_elyn[4] = {0x16a0e6, 0xdcc916, 0x332962, 0xcfeee9};
    uint32_t m1 = couple_elyn[0];
    uint32_t c1 = couple_elyn[1];

    clock_t begin, end;
    double time_spent;
    
    begin = clock();
    printf("Création des listes Lc et Lm...\n");
    GENERER_LISTES_OPTI(m1, lm_int, c1, lc_int);
    print_listes(lm_int, lc_int, m1, c1);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Temps écoulé: %fs for %d keys\n", time_spent, NB_K);
    
    
    // Quicksort
    begin = clock();
    printf("\nQuicksort des deux listes:\n");
    QUICKSORT(lm_int, 0, NB_K-1);
    QUICKSORT(lc_int, 0, NB_K-1);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Temps écoulé: %fs for %d keys\n", time_spent, NB_K);    
    

    // Recherche d'éléments communs
    begin = clock();
    printf("\nBegin research:\n");
    RECHERCHE_COMMUNS(lm_int, lc_int, couple_elyn[2], couple_elyn[3]);
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Temps écoulé: %fs for %d keys\n", time_spent, NB_K);
    

    // Free des listes d'entiers
    begin = clock();
    printf("\nLibération de mémoire des deux listes:\n");
    for(int i = 0 ; i < NB_K ; i++ )  {  
        free(lm_int[i]);
        free(lc_int[i]);
    }
    free(lm_int);
    free(lc_int);
    
    end = clock();
    time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Temps écoulé: %fs for %d keys\n", time_spent, NB_K);

    printf("\n\033[31mTemps total écoulé: %fs for %d keys\n\033[0m", (double)(clock() - tot_begin) / CLOCKS_PER_SEC, NB_K);
    return 0;
}







/* ****************************
 *    MODE HASHMAP TEST       *
 *****************************/

/*
typedef struct  {
    int nbr;
    int *keys;
} lm_data;

void GENERER_LISTES_TEST(uint32_t clair, lm_data *lm)  { 
    uint32_t sous_cles[11];
    uint32_t res;
    for (int i = 0; i < NB_K; i++)  {
        CADENCEMENT_CLE_OPTI(i, sous_cles);
        res = CHIFFREMENT_OPTI(clair, sous_cles);
        lm[res].nbr += 1;
        lm[res].keys = (int *) realloc( lm[res].keys, lm[res].nbr * sizeof(int) );
        lm[res].keys[lm[res].nbr-1] = i;
    }
}

void RECHERCHE_TEST(lm_data *lm, uint32_t c1, uint32_t m2, uint32_t c2)  {
    uint32_t sous_cles[11];
    uint32_t res, res2;
    int collisions = 0;
    int end = 0;
    for (int i = 0; i < NB_K; i++)  {
        CADENCEMENT_CLE_OPTI(i, sous_cles);
        res = DECHIFFREMENT_OPTI(c1, sous_cles);
        if (lm[res].nbr > 0)  {
            for (int j = 0; j < lm[res].nbr; j++)  {
                // Chaque clé k1 de lm
                res2 = CHIFFREMENT_DOUBLE_OPTI(m2, lm[res].keys[j], i);
                collisions++;
                if (res2 == c2)  {
                    printf("\033[32mClé fonctionnelle trouvée: %06x (%06x, %06x)\033[0m\n", res, lm[res].keys[j], i);
                    // end = 1;
                    // break;
                }
            }
        }
        // if (end)
        //     break;
    }
}

int main(int argc, char const *argv[])  {
    lm_data *lm = (lm_data * ) malloc(NB_K * sizeof(lm_data));

    uint32_t couple_elyn[4] = {0x16a0e6, 0xdcc916, 0x332962, 0xcfeee9};
    uint32_t m1 = couple_elyn[0];
    uint32_t c1 = couple_elyn[1];

    printf("Création de la liste Lm...\n");
    for (size_t i = 0; i < NB_K; i++)  {
        lm[i].nbr = 0;
        lm[i].keys = (int *) malloc( sizeof(int) );
    }
    GENERER_LISTES_TEST(m1, lm);


    printf("\nDéchiffrement et recherche de clé(s) fonctionnelle(s)...\n");
    RECHERCHE_TEST(lm, c1, couple_elyn[2], couple_elyn[3]);


    for(int i = 0 ; i < NB_K ; i++ ) 
        free(lm[i].keys);
    free(lm);
}
*/