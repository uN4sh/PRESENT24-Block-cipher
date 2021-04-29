#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#define TAILLE_MOT 6
#define TAILLE_MSG 24

// Fonctions statiques définies dans le fichier chiffrement.c
void decimal_to_binary(int dec, char *res, int lenght);
void hexa_to_binary(char *hex, char *res);
void cadencement_cle(char *cle_maitre, char sous_cles[][25]);
void SUBSTITUTION(char *etat, int *s);
void PERMUTATION(char *etat, int *p);

uint32_t PERMUTATION_OPTI(uint32_t etat, int *p);
uint32_t SUBSTITUTION_OPTI(uint32_t etat, int *s);


/**
 * @brief Inversion de P:
 * 
 *  i    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
 *  P(i) 0  6 12 18  1  7 13 19  2  8 14 20  3  9 15 21  4 10 16 22  5 11 17 23
 *  P-1  0  4  8 12 16 20  1  5  9 13 17 21  2  6 10 14 18 22  3  7 11 15 19 23
 */
int _p[TAILLE_MSG] = {0, 4, 8, 12, 16, 20, 1, 5, 9, 13, 17, 21,
                     2, 6, 10, 14, 18, 22, 3, 7, 11, 15, 19, 23};

/**
 * @brief Inversion de la boîte-S :
 * x    0 1 2 3 4 5 6 7 8 9 a b c d e f
 * S    c 5 6 b 9 0 a d 3 e f 8 4 7 1 2
 * S-1  5 e f 8 c 1 2 d b 4 6 3 0 7 9 a
 */
int _s[16] = {5, 14, 15, 8, 12, 1, 2, 13, 11, 4, 6, 3, 0, 7, 9, 10};


/**
 * @brief Algorithme de déchiffrement pour PRESENT-24 inversé ainsi :
 * 
 * -----------------------------
 * | Etat ← etat ⊕ K_11
 * | Pour i = 10 jusqu'à 0 :
 *      | Etat ← Permutation bit à bit inverse (etat)
 *      | Etat ← Substitution via Boite-S inverse (etat)
 *      | Etat ← Etat ⊕ K_i
 * | m ← Etat
 * | Retourner m
 * -----------------------------
 * 
 * @param chiffre_hex       Message chiffré hexadécimal
 * @param cle_maitre_hex    Clé maître hexadécimale pour le déchiffrement
 * @param clair             Résultat du déchiffrement PRESENT24^-1
 * @return                  Message déchiffré par l'algorithme           
 */
int DECHIFFREMENT(char *chiffre_hex, char *cle_maitre_hex, char *clair)  {
    // Convertit chiffre_hex and cle_maitre_hex en chaîne binaire
    char etat[TAILLE_MSG + 1];
    char cle_maitre[TAILLE_MSG + 1];
    hexa_to_binary(chiffre_hex, etat);
    hexa_to_binary(cle_maitre_hex, cle_maitre);

    // Génération des 11 sous clés de 24 bits chacune via algo. cadencement de clés
    char sous_cles[11][24 + 1];
    cadencement_cle(cle_maitre, sous_cles);

    int result;
    // Etat ← etat ⊕ K_11
    result = strtol(etat, NULL, 2) ^ strtol(sous_cles[10], NULL, 2);
    
    for (int i = 9; i > -1; i--)  {
        // Conversion en binaire du résultat de l'opération XOR
        decimal_to_binary(result, etat, 25);

        // Permutation inverse bit à bit puis substitution selon la boîte-S inverse sur l'état chiffré
        PERMUTATION(etat, _p);
        SUBSTITUTION(etat, _s);

        // Etat ← Etat ⊕ K_i
        result = strtol(etat, NULL, 2) ^ strtol(sous_cles[i], NULL, 2);
    }

    // Message clair retourné après 10 tours d'algorithme
    snprintf ( clair, TAILLE_MOT+1, "%06x", result);
    return result;
}

/**
 * @brief Optimisation du déchiffrement: passage en bitwise 
 * 
 * @param etat          Message chiffré en notation hexadécimale
 * @param sous_cles     Tableau de 11 sous clés hexadécimales générées par le Key-schedule
 * @return              Message déchiffré par l'algorithme PRESENT24^-1
 */ 
int DECHIFFREMENT_OPTI(uint32_t etat, uint32_t *sous_cles)  {
    // Etat ← Etat ⊕ K_11
    etat = etat ^ sous_cles[10];

    for (int i = 9; i > -1; i--)  {
        // Substitution et permutation de l'état
        etat = PERMUTATION_OPTI (etat, _p);
        etat = SUBSTITUTION_OPTI(etat, _s);
        // Etat ← Etat ⊕ K_i
        etat = etat ^ sous_cles[i];
    }
    // Message chiffré retourné après 10 tours d'algorithme
    return etat;
}
