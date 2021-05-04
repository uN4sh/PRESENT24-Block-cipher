#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <inttypes.h>

#define TAILLE_MOT 7
#define NB_K 16777216 // 2^24 = 16777216

void CADENCEMENT_CLE_OPTI(uint32_t m_key, uint32_t *sous_cles);
int  CHIFFREMENT_OPTI(uint32_t etat, uint32_t *sous_cles);
int  DECHIFFREMENT_OPTI(uint32_t etat, uint32_t *sous_cles);
int  CHIFFREMENT_DOUBLE_OPTI(uint32_t message, uint32_t k1, uint32_t k2);

/**
 * Attaque par le milieu :
 * On a (m, c) et on cherche (k1, k2)
 * 
 * Nombre de clés : 
 *   PRESENT-24 : longueur de la clé : 24 bits -> 2^24 = 16777216
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
 * @param m1        Message clair   m1 du couple en données       
 * @param c1        Message chiffré c1 du couple en données
 * @param l_lm      Tableau à deux dimensions pour contenir la liste Lm
 * @param l_lc      Tableau à deux dimensions pour contenir la liste Lc
 * @return          Les deux listes sont modifiées par la fonction 
 */
void GENERER_LISTES(uint32_t m1, uint32_t c1, int **l_lm, int **l_lc)  { 
    uint32_t sous_cles[11];
    for (int i = 0; i < NB_K; i++)  {
        CADENCEMENT_CLE_OPTI(i, sous_cles);
        l_lm[i][1] = i; 
        l_lc[i][1] = i;
        l_lm[i][0] = CHIFFREMENT_OPTI(m1, sous_cles);
        l_lc[i][0] = DECHIFFREMENT_OPTI(c1, sous_cles);
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

typedef struct  {
    uint32_t k1, k2;
} couple;

typedef struct {
    int nbr;
    couple *cles;
} conteneur;

/**
 * @brief Recherche d'éléments communs parmi les deux listes puis teste les clés candidates 
 *        avec le second couple (m2, c2) pour retrouver les clés (k1, k2) ayant servi au chiffrement.
 * 
 * @param lm    Liste Lm des couples (chiffré, clé)
 * @param lc    Liste Lc des couples (clair,   clé)
 * @param m2    Message clair   m2 du couple en données
 * @param c2    Message chiffré c2 du couple en données
 * @return      void
 */
conteneur RECHERCHE_COMMUNS(int **lm, int **lc, uint32_t m2, uint32_t c2)  {
    int i = 0, j = 0;
    int collisions = 0;
    int res;

    // conteneur *to_return = (conteneur * ) malloc(1 * sizeof(conteneur));
    conteneur to_return;
    to_return.nbr = 0;
    to_return.cles = (couple * ) malloc(to_return.nbr * sizeof(couple));
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
                to_return.nbr += 1;
                to_return.cles = (couple *) realloc( to_return.cles, to_return.nbr * sizeof(couple) );
                to_return.cles[to_return.nbr-1].k1 = lm[j][1];
                to_return.cles[to_return.nbr-1].k2 = lc[i][1];
                printf("\033[32m    Clé secrète retrouvée: (k1, k2) = (%06x, %06x)\033[0m\n", lm[j][1], lc[i][1]);
            }
            i++;
            j++;
        }
    }
    printf("    Recherche terminée, \033[33m%d collisions\033[00m rencontrées.\n", collisions);
    return to_return;
}


void print_listes(int **l_lm, int **l_lc)  {
    printf("Lm  \033[34m   Clé |\033[0m Chiffré |  Lc  \033[34m   Clé |\033[0m Déchiffré\033[0m\n");
    for (int i = 0; i < 10; i++)
        printf("\033[00;3m%02d: \033[34m%06x |\033[00;3m  %06x |      \033[34m%06x |\033[00;3m %06x\033[00m\n", i, l_lm[i][1], l_lm[i][0], l_lc[i][1], l_lc[i][0]);
    printf("- - -\n");
    printf("\033[00;3m%02d: \033[34m%06x |\033[00;3m  %06x |    %06x\033[00m\n", NB_K-1, l_lm[NB_K-1][1], l_lm[NB_K-1][0], l_lc[NB_K-1][0]);
}


int main(int argc, char const *argv[])  {
    clock_t tot_begin = clock();

    // Deux listes en entiers pour contenir les listes Lm et Lc de couples (Chiffré, Clé) et (Message, Clé)
    int **l_lm = (int ** ) malloc(NB_K * sizeof(int * ));
    int **l_lc = (int ** ) malloc(NB_K * sizeof(int * ));
    for(int i = 0 ; i < NB_K ; i++ )  { 
        l_lm[i] = (int * ) malloc(2 * sizeof(int));
        l_lc[i] = (int * ) malloc(2 * sizeof(int));
    }

    // uint32_t couple_elyn[4] = {0x16a0e6, 0xdcc916, 0x332962, 0xcfeee9};
    uint32_t couple[4] = {0x02c315, 0x88b6ed, 0x1d2dec, 0xc4bb7a};
    uint32_t m1 = couple[0], c1 = couple[1], m2 = couple[2], c2 = couple[3];
    printf("           Clair |   Chiffré\n");
    printf("\033[33;1m(m1, c1)  \033[33;1m%06x |    %06x\033[0m\n", m1, c1);
    printf("\033[33;1m(m2, c2)  \033[33;1m%06x |    %06x\033[0m\n\n", m2, c2);


    clock_t begin = clock();
    printf("Création des listes Lc et Lm...\n");
    GENERER_LISTES(m1, c1, l_lm, l_lc);
    print_listes(l_lm, l_lc);
    printf("    Temps écoulé: %fs pour %d clés\n", (double)(clock() - begin) / CLOCKS_PER_SEC, NB_K);
    
    
    // Quicksort des listes Lc et Lm
    begin = clock();
    printf("\nTri rapide de chacune des deux listes...\n");
    QUICKSORT(l_lm, 0, NB_K-1);
    QUICKSORT(l_lc, 0, NB_K-1);
    printf("    Temps écoulé: %fs pour %d clés\n", (double)(clock() - begin) / CLOCKS_PER_SEC, NB_K);
    

    // Recherche d'éléments communs
    conteneur attaque_res;
    begin = clock();
    printf("\nRecherche d'éléments communs et de clé(s) fonctionnelle(s)...\n");
    attaque_res = RECHERCHE_COMMUNS(l_lm, l_lc, m2, c2);
    
    // Vérifier en double-chiffrant m1 et m2
    uint32_t res;
    for (int i = 0; i < attaque_res.nbr; i++)  {
        printf("\nTest de la clé secrète: \033[32m(k1, k2) = (%06x, %06x)\033[0m\n", attaque_res.cles[i].k1, attaque_res.cles[i].k2);
        res = CHIFFREMENT_DOUBLE_OPTI(m1, attaque_res.cles[i].k1, attaque_res.cles[i].k2);
        if (res == c1)
            printf("\033[32mPassed: %06x -> %06x\033[00m\n", m1, res);
        else
            printf("\033[31mFailed: %06x -> %06x || %06x\033[00m\n", m1, c1, res);

        res = CHIFFREMENT_DOUBLE_OPTI(m2, attaque_res.cles[i].k1, attaque_res.cles[i].k2);
        if (res == c2)
            printf("\033[32mPassed: %06x -> %06x\033[00m\n", m2, res);
        else
            printf("\033[31mFailed: %06x -> %06x || %06x\033[00m\n", m2, c2, res);
    }
    printf("    Temps écoulé: %fs pour %d clés\n", (double)(clock() - begin) / CLOCKS_PER_SEC, NB_K);
    

    // Free des listes d'entiers
    begin = clock();
    printf("\nLibération de mémoire des deux listes...\n");
    for(int i = 0 ; i < NB_K ; i++ )  {  
        free(l_lm[i]);
        free(l_lc[i]);
    }
    free(l_lm);
    free(l_lc);
    
    printf("    Temps écoulé: %fs pour %d clés\n", (double)(clock() - begin) / CLOCKS_PER_SEC, NB_K);

    printf("\n\033[33mTemps total écoulé: %fs pour %d clés\n\033[0m", (double)(clock() - tot_begin) / CLOCKS_PER_SEC, NB_K);
    return 0;
}




/* ***********************************
 *           MODE HASHMAP            *
 * Non utilisé parce que revient à   *
 * gagner peu de temps pour trop de  *
 *         mémoire perdue.           *
 ************************************/

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
