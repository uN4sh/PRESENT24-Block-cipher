#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TAILLE_MSG 24


/**
 * @brief Substitue 4 bits en entrée via sa notation hexadécimale dans la Boîte-S  pour le déchiffrement
 * 
 * @param bits  Chaîne de 4 bits à substituer selon la Boîte-S de déchiffrement
 */
void d_substitution_4_bits(char *bits)  {
    // int s[16] = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};

    // int s[16] = {0,  1,  2, 3,  4, 5, 6,  7,  8, 9, a, b, c, d, e, f};
    // printf("d_substit_4_b");
    // int s[16] = {5, 14, 15, 8, 12, 1, 27, 130, 110, 40, 60, 3, 0, 7, 9, 10};

  // x    0 1 2 3 4 5 6 7 8 9 a b c d e f
  // S[x] c 5 6 b 9 0 a d 3 e f 8 4 7 1 2

  // x    0 1 2 3 4 5 6 7 8 9 a b c d e f
  // S[x] 5 e f 8 c 1 2 d b 4 6 3 0 7 9 a
    int s[16] = {5, 14, 15, 8, 12, 1, 2, 13, 11, 4, 6, 3, 0, 7, 9, 10}; 

    // Convert binary string to int
    int decvalue = strtol(bits, NULL, 2);

    // Subsitute from S-Table
    decvalue = s[decvalue];

    // Convert to binary string
    decimal_to_binary(decvalue, bits, 5);
}

/**
 * @brief Couche non linéaire du chiffrement : substitution 4 bits par 4 selon la boîte-S.
 * 
 * x    0 1 2 3 4 5 6 7 8 9 a b c d e f
 * S[x] c 5 6 b 9 0 a d 3 e f 8 4 7 1 2
 */
void D_SUBSTITUTION(char *etat)
{
    char substr[4];
    for (size_t i = 0; i < TAILLE_MSG; i+=4)  {
        // Récupération des 4 bits
        for (size_t j = 0; j < 4; j++)
            substr[j] = etat[i+j];

        d_substitution_4_bits(substr);

        // Replace in etat[]
        for (size_t j = 0; j < 4; j++)
            etat[i+j] = substr[j];
    }
}


/**
 * @brief Couche linéaire du chiffrement : permutation bit-à-bit selon la table P.
 * 
 *  i    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
 *  P(i) 0  6 12 18  1  7 13 19  2  8 14 20  3  9 15 21  4 10 16 22  5 11 17 23
 * 
 * @param etat  Registre de 24 bits contenant l'état (chaine binaire)
 * @return      La variable état est modifiée, permuté bit-à-bit 
 */
void D_PERMUTATION(char *etat)  {
    // int p[TAILLE_MSG] = {0, 6, 12, 18, 1, 7, 13, 19, 2, 8, 14, 20,
    //                      3, 9, 15, 21, 4, 10, 16, 22, 5, 11, 17, 23};
    int p[TAILLE_MSG] = {0, 4, 8, 12, 16, 20, 1, 5, 9, 13, 17, 21, 2, 6, 10, 14, 18, 22, 3, 7, 11, 15, 19, 23};

    char tmp[TAILLE_MSG+1];
    for (size_t i = 0; i < TAILLE_MSG; i++)  {
        tmp[p[i]] = etat[i];
    }
    tmp[TAILLE_MSG] = '\0';

    for (size_t i = 0; i < TAILLE_MSG+1; i++)  {
        etat[i] = tmp[i];
    }
}

/**
 * @brief Algorithme de chiffrement PRESENT24
 * 
 * @param etat_hex          Message clair hexadécimal
 * @param cle_maitre_hex    Clé maître hexadécimale pour le chiffrement
 * 
 * @return                  Message chiffré par l'algorithme           
 */
int DECHIFFREMENT(char *chiffre_hex, char *cle_maitre_hex)
{
    // Convert chiffre_hex and cle_maitre_hex to binary
    char etat[TAILLE_MSG + 1];
    char cle_maitre[TAILLE_MSG + 1];
    hexa_to_binary(chiffre_hex, etat);
    hexa_to_binary(cle_maitre_hex, cle_maitre);

    // Génération des 11 sous clés de 24 bits chacune via algo. cadencement de clés
    char sous_cles[11][24 + 1];
    cadencement_cle(cle_maitre, sous_cles);

    int result;
    // etat ← etat ⊕ K_11
    result = strtol(etat, NULL, 2) ^ strtol(sous_cles[10], NULL, 2);
    
    for (int i = 9; i > -1; i--)
    {   
        // Etat ← Etat ⊕ K_i
        decimal_to_binary(result, etat, 25);
        // Permutation_déchiffr puis subsitution_déchiffr du chiffré
        D_PERMUTATION(etat);
        D_SUBSTITUTION(etat);


        result = strtol(etat, NULL, 2) ^ strtol(sous_cles[i], NULL, 2);
    }


    return result;
}
