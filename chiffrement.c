#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#define TAILLE_MOT 6
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
        
        // Substitution des 4 bits selon la Boite-S
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
void cadencement_cle(char *cle_maitre, char sous_cles[][25])  {
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
        // Opti.: Faire un masque avec un & pour extraire les 24 bits
        // Exp : 000000000000000000000000000000011111111111111111111000000000000

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
        // Opti.: Récupérer via masque les bits 18 à 0
        //        Shifter les bits 79-19
        //        Replacer les 18 à 0 au début (Comment ?)
        
            
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

void cadencement_cle_new(char *m_key, uint32_t *sous_cles)  {
    uint32_t master_key = strtol(m_key, NULL, 16); 
    // Division en High (bits 79-40) et Low (bits 39-16)
    uint64_t master_high = (uint64_t)master_key << 16 ; // ajouter 16 0 sur la droite 
    uint64_t master_low = 0; /* 40 0 */
    
    uint64_t tmp;
    uint64_t premiers_bits_de_low;
    uint64_t premiers_bits_de_high;
    uint64_t derniers_bits_de_low;
    uint64_t derniers_bits_de_high;

    int s[16] = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};

    for (int i = 0; i < 11; i++)  {
        // Extraction de la clé
        sous_cles[i] = master_low >> 16; // Extraction bit 39 à 16 -> 16 premiers bits de Low
        
        // Step 1: Rotation du registre de 61 bits vers la gauche -> [k18k17..k0k79..k20k19]
        premiers_bits_de_low = master_low >> 19;
        premiers_bits_de_high = master_high >> 19;
        derniers_bits_de_low = master_low & 0x7FFFF; // Masque avec 00...001111111111111111111
        derniers_bits_de_high = master_high & 0x7FFFF;
        
        master_high = (derniers_bits_de_low << 21) ^ premiers_bits_de_high; // 19 derniers bits de Low (18-0)    + 21 premiers bits de high (79-59)
        master_low  = (derniers_bits_de_high << 21) ^ premiers_bits_de_low; // 19 derniers bits de high (58-40)  + 21 premiers bits de low  (39-19)

        // Step 2: Application de la Boîte-S aux bits 79-76
        tmp = master_high >> 36; // Extraire les 4 premiers bits de high
        tmp = s[tmp]; // Subsitution Boîte-S
        master_high = master_high & 0x0FFFFFFFFF; // Passer les 4 premiers bits de High à 0
        master_high = master_high | ((uint64_t)tmp << 36); // Remplacer les 4 premiers bits par tmp

        // Step 3: XOR des bits extraits 19-15 avec la valeur binaire de i ∈ [1,11]
        master_low = master_low ^ (((uint64_t)i+1) << 15); // XOR des bits 19-15 avec i (shifté de 15)  
    }
}


/**
 * @brief Algorithme de chiffrement PRESENT24
 * 
 * @param[in] etat_hex        Message clair hexadécimal
 * @param[in] cle_maitre_hex  Clé maître hexadécimale pour le chiffrement
 * @param[out] cipher         Résultat du chiffrement PRESENT24
 * @return                    Message chiffré par l'algorithme           
 */
int CHIFFREMENT(char *etat_hex, char *cle_maitre_hex, char *cipher)  {
    // Convertit etat_hex and cle_maitre_hex en chaîne binaire
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

    // Message chiffré retourné après 10 tours d'algorithme
    snprintf ( cipher, TAILLE_MOT+1, "%06x", result);
    return result;
}

int CHIFFREMENT2(char *etat_hex, char *cle_maitre_hex, char *cipher)  {
    // Convertit etat_hex en chaîne binaire
    char etat[TAILLE_MSG+1];
    hexa_to_binary(etat_hex, etat);
    
    // Génération des 11 sous clés de 24 bits chacune via algo. cadencement de clés
    uint32_t sous_cles[11];
    cadencement_cle_new(cle_maitre_hex, sous_cles);
    
    int result;
    for (size_t i = 0; i < 10; i++)  {
        // Etat ← Etat ⊕ K_i
        result = strtol(etat, NULL, 2) ^ sous_cles[i];
        decimal_to_binary(result, etat, 25);
        // Substitution et permutation de l'état
        SUBSTITUTION(etat);
        PERMUTATION(etat);
    }

    // Etat ← Etat ⊕ K_11
    result = strtol(etat, NULL, 2) ^ sous_cles[10];

    // Message chiffré retourné après 10 tours d'algorithme
    snprintf ( cipher, TAILLE_MOT+1, "%06x", result);
    return result;
}


/**
 * @brief Chiffrement double 2PRESENT24
 * 
 * @param message   Message clair à chiffrer
 * @param cle_k1    Clé k1 pour le chiffrement du message
 * @param cle_k2    Clé k2 pour le second chiffrement
 * @param cipher    Résultat du chiffrement double 2PRESENT24
 */
int CHIFFREMENT_DOUBLE(char *message, char *cle_k1, char *cle_k2, char *cipher)  {
    CHIFFREMENT(message, cle_k1, cipher);
    return CHIFFREMENT(cipher, cle_k2, cipher);
}
