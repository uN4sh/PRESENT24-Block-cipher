#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TAILLE_MSG 24

/**
 * @brief Convertit un entier décimal en entrée en chaîne binaire
 * 
 * @param dec       Entier décimal à convertir en binaire
 * @param res       Chaîne résultat pour contenir le nombre binaire
 * @param lenght    Taille de la chaîne binaire (nombre de bits)
 */
void decimal_to_binary(int dec, char *res, int lenght)  {
    res[lenght-1] = '\0';
    res[lenght-2] = (dec & 1) + '0';
    for (int i = lenght-3; i > -1; i--)  {
        res[i] = ((dec >> (lenght-2-i)) & 1) + '0';
    }
}


/**
 * @brief Convertit une chaîne hexadécimale en une chaîne binaire de 24 bits
 * 
 * @param hex   Chaîne hexadécimale à convertir
 * @param res   Chaîne résultat pour contenir le nombre binaire de 24 bits
 */
void hexa_to_binary(char *hex, char *res)  {
    int decvalue = strtol(hex, NULL, 16);
    decimal_to_binary(decvalue, res, 25);
}


/**
 * @brief Substitue 4 bits en entrée via sa notation hexadécimale dans la Boîte-S 
 * 
 * @param bits  Chaîne de 4 bits à substituer selon la Boîte-S
 */
void substitution_4_bits(char *bits)  {
    int s[16] = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};

    // Convert binary string to int
    int decvalue = strtol( bits, NULL, 2 );
    
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
void SUBSTITUTION(char *etat)  {
    char substr[4];
    for (size_t i = 0; i < TAILLE_MSG; i+=4)  {
        // Récupération des 4 bits
        for (size_t j = 0; j < 4; j++)
            substr[j] = etat[i+j];
        
        substitution_4_bits(substr);

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
void PERMUTATION(char *etat)  {
    int p[TAILLE_MSG] = { 0, 6, 12, 18, 1, 7, 13, 19, 2, 8, 14, 20, 
                        3, 9, 15, 21, 4, 10, 16, 22, 5, 11, 17, 23 };
    
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
 * @brief Cadencement de clé : génère 11 sous-clés de 24 bits chacune à partir d'une clé maître
 * 
 * @param cle_maitre    Registre de 24 bits représentant la clé maître sans les 0 ajoutés
 * @param sous_cles     Tableau 2-Dimensions pour contenir 11 sous clés de 24 bits chacune 
 */
void cadencement_cle(char *cle_maitre, 
                    char sous_cles[][25])  {
    // Registre K de 80 bits contenant la clé maître complétée par des 0
    char K[80+1];
    snprintf(K, 81, "%s%056d", cle_maitre, 0);
    
    char substr_s1_1[19+1];
    char substr_s1_2[61+1];
    char substr_s2[4+1];
    char substr_s3[5+1];
    int decvalue;
    int k;

    // 11 tours pour générer les sous-clés
    for (size_t i = 0; i < 11; i++)  {
    
        // Extraction des bits 39 à 16 pour la sous-clé Ki
        k = 0;
        for (size_t j = 40; j <= 63; j++)  {
            sous_cles[i][k] = K[j];
            k++;
        }
        sous_cles[i][24] = '\0';


        // Mise à jour du registre K en 3 étapes
            // Step 1: Rotation du registre de 61 bits vers la gauche -> [k18k17..k0k79..k20k19]
        k = 0;
        for (int j = 61; j <= 79; j++)  {  // Récupérer k18-k0 
            substr_s1_1[k] = K[j];  
            k++;
        }
        substr_s1_1[k] = '\0';

        k = 0;
        for (size_t j = 0; j <= 60; j++)  { // Récupérer k79-k19
            substr_s1_2[k] = K[j];
            k++;
        }
        substr_s1_2[k] = '\0';
        
        strcpy( K, substr_s1_1 );
        strcat( K, substr_s1_2 );
        
            
            // Step 2: Application de la Boîte-S aux bits 79-76
        for (size_t j = 0; j < 4; j++)
            substr_s2[j] = K[j];
        
        substr_s2[k] = '\0';
        substitution_4_bits(substr_s2);
        
        for (size_t j = 0; j < 4; j++)
            K[j] = substr_s2[j];
        

            // Step 3: XOR des bits extraits 19-15 avec la valeur binaire de i ∈ [1,11]
        k = 0;
        for (size_t j = 60; j <= 64; j++)  { // k19-k15
            substr_s3[k] = K[j];
            k++;
        }

        decvalue = strtol(substr_s3, NULL, 2) ^ (i+1);   
        decimal_to_binary(decvalue, substr_s3, 6);
        k = 0;
        for (size_t j = 60; j <= 64; j++)  {
            K[j] = substr_s3[k];
            k++;
        }
    }
    // Afficher les 11 sous-clés
    // for (int i = 0; i < 11; i++)  {
    //     decvalue = strtol(sous_cles[i], NULL, 2);
    //     printf("%02d - %s | %06x\n", i+1, sous_cles[i], decvalue);
    // }
}


/**
 * @brief Algorithme de chiffrement PRESENT24
 * 
 * @param etat_hex          Message clair hexadécimal
 * @param cle_maitre_hex    Clé maître hexadécimale pour le chiffrement
 * 
 * @return                  Message chiffré par l'algorithme           
 */
int CHIFFREMENT(char *etat_hex, char *cle_maitre_hex)  {
    // Convert etat_hex and cle_maitre_hex to binary
    char etat[TAILLE_MSG+1];
    char cle_maitre[TAILLE_MSG+1];
    hexa_to_binary(etat_hex, etat);
    hexa_to_binary(cle_maitre_hex, cle_maitre);

    // Génération des 11 sous clés de 24 bits chacune via algo. cadencement de clés
    char sous_cles[11][24+1];
    cadencement_cle(cle_maitre, sous_cles);
    
    int result;
    for (size_t i = 0; i < 10; i++)  {
        // Etat ← Etat ⊕ K_i
        result = strtol(etat, NULL, 2) ^ strtol(sous_cles[i], NULL, 2);
        decimal_to_binary(result, etat, 25);
        // Substitution et permutation de l'état
        SUBSTITUTION(etat);
        PERMUTATION(etat);
    }

    // Etat ← Etat ⊕ K_11
    result = strtol(etat, NULL, 2) ^ strtol(sous_cles[10], NULL, 2);
    
    return result;
}