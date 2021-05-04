#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>

#define TAILLE_MOT 6
#define TAILLE_MSG 24

int p[TAILLE_MSG] = { 0, 6, 12, 18, 1, 7, 13, 19, 2, 8, 14, 20, 
                      3, 9, 15, 21, 4, 10, 16, 22, 5, 11, 17, 23 };
int s[16] = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};


/* ****************************
 *    VERSION OPTIMISÉE       *
 *****************************/

/**
 * @brief Couche non linéaire du chiffrement : substitution 4 bits par 4 selon la boîte-S.
 * 
 * x    0 1 2 3 4 5 6 7 8 9 a b c d e f
 * S[x] c 5 6 b 9 0 a d 3 e f 8 4 7 1 2
 * 
 * @param etat  Registre de 24 bits contenant l'état (chaine binaire)
 * @param s     La Boîte-S de substitution afin de déchiffrer avec la boîte inversée
 * @return      Registre de 24 bits contenant l'état substitué 
 */ 
uint32_t SUBSTITUTION_OPTI(uint32_t etat, int *s)  {
    uint32_t tmp;
    tmp = etat >> 20; // Extraire les 4 premiers bits de high
    etat = etat & 0x0FFFFF; // Passer les 4 premiers bits de High à 0
    etat = etat | ((uint64_t)s[tmp] << 20); // Remplacer les 4 premiers bits par tmp

    // Extraire les 4 deuxièmes :
    tmp = (etat >> 16) & 0x0F;
    etat = (etat & 0xF0FFFF) | ((uint64_t)s[tmp] << 16);
    
    // Extraire les 4 3eme
    tmp = (etat >> 12) & 0x00F;
    etat = (etat & 0xFF0FFF) | ((uint64_t)s[tmp] << 12);

    // Extraire les 4 4eme
    tmp = (etat >> 8) & 0x000F;
    etat = (etat & 0xFFF0FF) | ((uint64_t)s[tmp] << 8);

     // Extraire les 4 5eme
    tmp = (etat >> 4) & 0x0000F;
    etat = (etat & 0xFFFF0F) | ((uint64_t)s[tmp] << 4);

    // Extraire les 4 derniers
    tmp = etat & 0x00000F;
    etat = (etat & 0xFFFFF0) | s[tmp];

    return etat;
}

/**
 * @brief Couche linéaire du chiffrement : permutation bit-à-bit selon la table P.
 * 
 *  i    0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18 19 20 21 22 23
 *  P(i) 0  6 12 18  1  7 13 19  2  8 14 20  3  9 15 21  4 10 16 22  5 11 17 23
 * 
 * Initialiser le nouvel état après permutation à 0
 * Pour chaque bit à permuter : 
 *  Placer le bit à permuter en position faible avec un shift à droite et le récupérer avec un masque
 *  Placer le bit à sa nouvelle position avec un shift à gauche
 *  Replacer le bit dans l'état avec un bitwise OR dans le nouvel état
 * 
 * @param etat  Registre de 24 bits contenant l'état (chaine binaire)
 * @param p     La table P de permutation afin de déchiffrer avec la table inversée
 * @return      Registre de 24 bits contenant l'état permuté bit-à-bit 
 */ 
uint32_t PERMUTATION_OPTI(uint32_t etat, int *p)  {
    uint64_t permutation = 0;
    int i;
    for (i=0; i<24; i++){
        int distance = 23 - i;
        permutation = permutation | ((etat >> distance & 0x1) << (23 - p[i]));
    }
    return permutation;
}


/**
 * @brief Cadencement de clé : génère 11 sous-clés de 24 bits chacune à partir d'une clé maître
 * 
 * @param m_key         Registre de 24 bits représentant la clé maître sans les 0 ajoutés
 * @param sous_cles     Tableau 2-Dimensions pour contenir 11 sous clés de 24 bits chacune 
 */
void CADENCEMENT_CLE_OPTI(uint32_t m_key, uint32_t *sous_cles)  {
    // Division en High (bits 79-40) et Low (bits 39-16)
    uint64_t master_high = (uint64_t)m_key << 16 ; // ajouter 16 0 sur la droite 
    uint64_t master_low = 0; // 40 0 
    uint64_t tmp; // Tmp pour les rotations et extractions

    for (int i = 0; i < 11; i++)  {
        // Extraction de la clé
        sous_cles[i] = master_low >> 16; // Extraction bit 39 à 16 -> 16 premiers bits de Low
        
        // Step 1: Rotation du registre de 61 bits vers la gauche -> [k18k17..k0k79..k20k19]
        tmp = master_high;
        // Pour récupérer les 19 derniers bits: Masque & avec 00...001111111111111111111
        master_high = ((master_low & 0x7FFFF) << 21) ^ (master_high >> 19); // 19 derniers bits de Low (18-0)    + 21 premiers bits de high (79-59)
        master_low  = ((tmp        & 0x7FFFF) << 21) ^ (master_low  >> 19); // 19 derniers bits de high (58-40)  + 21 premiers bits de low  (39-19)
        
        // Step 2: Application de la Boîte-S aux bits 79-76
        tmp = master_high >> 36; // Extraire les 4 premiers bits de high
        master_high = master_high & 0x0FFFFFFFFF; // Passer les 4 premiers bits de High à 0
        master_high = master_high | ((uint64_t)s[tmp] << 36); // Remplacer les 4 premiers bits par tmp substutué par boite-S

        // Step 3: XOR des bits extraits 19-15 avec la valeur binaire de i ∈ [1,11]
        master_low = master_low ^ (((uint64_t)i+1) << 15); // XOR des bits 19-15 avec i (shifté de 15)  
    }
    // Afficher les 11 sous-clés
    // for (int i = 0; i < 11; i++)  {
    //     printf("%02d - %06x\n", i+1, sous_cles[i]);
    // }
}


/**
 * @brief Algorithme de chiffrement PRESENT24
 * 
 * @param[in] etat          Message clair à chiffrer
 * @param[in] sous_cles     Tableau 2-Dimensions des 11 sous-clés de 24 bits chacune
 * @return                  Message chiffré par l'algorithme           
 */
int CHIFFREMENT_OPTI(uint32_t etat, uint32_t *sous_cles)  {
    for (size_t i = 0; i < 10; i++)  {
        
        // Etat ← Etat ⊕ K_i
        etat = etat ^ sous_cles[i];
        // Substitution et permutation de l'état
        etat = SUBSTITUTION_OPTI(etat, s);
        etat = PERMUTATION_OPTI(etat, p);

        // etat = PERMUTATION_OPTI(SUBSTITUTION_OPTI((etat ^ sous_cles[i]), s), p);
    }

    // Etat ← Etat ⊕ K_11
    // Message chiffré retourné après 10 itérations d'algorithme
    return etat ^ sous_cles[10];
}


/**
 * @brief Chiffrement double 2PRESENT24
 * 
 * @param message   Message clair à chiffrer
 * @param cle_k1    Clé k1 pour le chiffrement du message
 * @param cle_k2    Clé k2 pour le second chiffrement
 * @return          Résultat du chiffrement double 2PRESENT24
 */
int CHIFFREMENT_DOUBLE_OPTI(uint32_t message, uint32_t k1, uint32_t k2)  {
    uint32_t sous_cles_k1[11];
    uint32_t sous_cles_k2[11];
    CADENCEMENT_CLE_OPTI(k1, sous_cles_k1);
    CADENCEMENT_CLE_OPTI(k2, sous_cles_k2);
    return CHIFFREMENT_OPTI(CHIFFREMENT_OPTI(message, sous_cles_k1), sous_cles_k2);
}


















/* ****************************
 *     ANCIENNE VERSION       *
 *****************************/

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
void substitution_4_bits(char *bits, int *s)  {
    // int s[16] = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};

    // Convert binary string to int
    int decvalue = strtol( bits, NULL, 2 );
    
    // Subsitute from S-Table
    decvalue = s[decvalue];

    // Convert to binary string
    decimal_to_binary(decvalue, bits, 5);
}

void SUBSTITUTION(char *etat, int *s)  {
    char substr[5];
    for (size_t i = 0; i < TAILLE_MSG; i+=4)  {
        // Récupération des 4 bits
        for (size_t j = 0; j < 4; j++)
            substr[j] = etat[i+j];
        substr[4] = '\0';

        // Substitution des 4 bits selon la Boite-S
        substitution_4_bits(substr, s);

        // Replace in etat[] 
        for (size_t j = 0; j < 4; j++)
            etat[i+j] = substr[j];
    }
}

void PERMUTATION(char *etat, int *p)  {
    
    char tmp[TAILLE_MSG+1];
    for (size_t i = 0; i < TAILLE_MSG; i++)  {
        tmp[p[i]] = etat[i];
    }
    tmp[TAILLE_MSG] = '\0';

    for (size_t i = 0; i < TAILLE_MSG+1; i++)  {
        etat[i] = tmp[i];
    }
}

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
        substitution_4_bits(substr_s2, s);

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
}

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
        SUBSTITUTION(etat, s);
        PERMUTATION(etat, p);
    }

    // Etat ← Etat ⊕ K_11
    result = strtol(etat, NULL, 2) ^ strtol(sous_cles[10], NULL, 2);

    // Message chiffré retourné après 10 tours d'algorithme
    snprintf ( cipher, TAILLE_MOT+1, "%06x", result);
    return result;
}
