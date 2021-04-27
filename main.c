#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TAILLE_MSG 24


/**
 * TODO:
 * 1. Implantez le chiffrement PRESENT24 en C. Vérifiez en utilisant les vecteurs de test ci-dessus que
 * votre code fonctionne correctement.
 *      OK Fct Substitution
 *      OK Fct Permutation
 *      c. Key-schedule
 *      d. Algorithme de chiffrement
 * 
 * 2. Implantez la fonction de déchiffrement en C dans un fichier séparé. Pour que le déchiffrement fonctionne correctement, 
 * il faudra considérer les fonctions inverses de la boîte-S et de la permutation bit-à-bit P. 
 * Noter les inverses de ces fonctions dans le fichier texte qui accompagne votre projet. 
 * Vérifiez que votre d´echiffrement marche bien, en chiffrant un message quelconque avec PRESENT24 et en déchiffrant par la suite le chiffré obtenu.
 */


void decimal_to_binary(int dec, char *res, int lenght)  {
    res[lenght-1] = '\0';
    res[lenght-2] = (dec & 1) + '0';
    for (int i = lenght-3; i > -1; i--)  {
        res[i] = ((dec >> lenght - 2 - i) & 1) + '0';
    }

    // res[4] = '\0';
    // res[3] = (dec & 1) + '0';
    // res[2] = ((dec >> 1) & 1) + '0';
    // res[1] = ((dec >> 2) & 1) + '0';
    // res[0] = ((dec >> 3) & 1) + '0';
}

void hexa_to_binary(char *hex, char *res)  {
    int decvalue = (int) strtol(hex, NULL, 16);
    decimal_to_binary(decvalue, res, 25);
}

void substitution_boite_s(char *bits)  {
    int s[16] = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};

    // Convert binary string to int
    int decvalue = (int) strtol( bits, NULL, 2 );
    
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
    // int s[16] = {12, 5, 6, 11, 9, 0, 10, 13, 3, 14, 15, 8, 4, 7, 1, 2};
    char substr[4];
    for (size_t i = 0; i < TAILLE_MSG; i+=4)  {
        // Récupération des 4 bits
        for (size_t j = 0; j < 4; j++)
            substr[j] = etat[i+j];
        
        substitution_boite_s(substr);
        // // Convert binary string to int
        // int intvalue = (int) strtol( substr, NULL, 2 );
        
        // // Subsitute from S-Table
        // intvalue = s[intvalue];

        // // Convert to binary string
        // decimal_to_binary(intvalue, substr);

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
 * @brief
 * 
 * @param cle_maitre    Registre de 80 bits représentant la clé maître
 * @param sous_cles     11 sous clés de 24 bits chacune
 */
void cadencement_cle(char *cle_maitre, char sous_cles[11][25])  {
    char K[81];

    for (size_t i = 0; i < 24; i++)  
        K[i] = cle_maitre[i];
    
    for (size_t i = 24; i < 80; i++)  
        K[i] = '0';
    
    K[80] = '\0';  

    char substr_s1_1[19+1];
    char substr_s1_2[61+1];
    char substr_s2[4+1];
    char substr_s3[5+1];
    char binary_i[5+1];
    int decvalue;
    int k;

    // 11 tours pour générer les sous-clés
    for (size_t i = 0; i < 11; i++)  {
    
        // Au tour i, la sous-clé Ki de 24 bits est constituée des bits 39 à 16.
        k = 0;
        for (size_t j = 40; j <= 63; j++)  {
            sous_cles[i][k] = K[j];
            k++;
        }
        sous_cles[i][24] = '\0';


        // Mise à jour du registre K
            // 1. [k79k78...k1k0] = [k18k17...k0k79...k20k19]
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
        
        k = 0;
        for (int j = 0; j < 19; j++)
            K[j] = substr_s1_1[j];
        for (int j = 19; j < 80; j++)  {
            K[j] = substr_s1_2[k];
            k++;
        }
        K[80] = '\0';
        // strcpy( K, substr_s1_1 );
        // strcat( K, substr_s1_2 );
            
            // 2. [k79k78k77k76] = S[k79k78k77k76]
        // k = 0;
        for (size_t j = 0; j < 4; j++)  {
            substr_s2[j] = K[j];
            // k++;
        }
        substr_s2[k] = '\0';
        substitution_boite_s(substr_s2);
        // k = 0;
        for (size_t j = 0; j < 4; j++)  {
            K[j] = substr_s2[j];
            // k++;
        }

            // 3. [k19k18k17k16k15] = [k19k18k17k16k15] ⊕ i
        k = 0;
        for (size_t j = 60; j <= 64; j++)  { // k19-k15
            substr_s3[k] = K[j];
            k++;
        }

        decvalue = (int) strtol(substr_s3, NULL, 2);
        decvalue = decvalue ^ (i+1);    
        decimal_to_binary(decvalue, substr_s3, 6);
        k = 0;
        for (size_t j = 60; j <= 64; j++)  {
            K[j] = substr_s3[k];
            k++;
        }
    }
    // Afficher les 11 sous-clés
    // for (int i = 0; i < 11; i++)  {
    //     decvalue = (int) strtol(sous_cles[i], NULL, 2);
    //     printf("%02d - %s | %06x\n", i+1, sous_cles[i], decvalue);
    // }
}


int CHIFFREMENT(char *etat_hex, char *cle_maitre_hex)  {
    
    // Convert etat_hex and cle_maitre_hex to binary
    char etat[TAILLE_MSG+1];
    hexa_to_binary(etat_hex, etat);

    char cle_maitre[TAILLE_MSG+1];
    hexa_to_binary(cle_maitre_hex, cle_maitre);


    // 11 sous clés de 24 bits chacune
    char sous_cles[11][24+1];

    cadencement_cle(cle_maitre, sous_cles);
    
    int decvalue;
    for (size_t i = 0; i < 10; i++)  {
        decvalue = (int) strtol(etat, NULL, 2) ^ (int) strtol(sous_cles[i], NULL, 2);
        decimal_to_binary(decvalue, etat, 25);
        SUBSTITUTION(etat);
        PERMUTATION(etat);
    }

    decvalue = (int) strtol(etat, NULL, 2) ^ (int) strtol(sous_cles[10], NULL, 2);
    decimal_to_binary(decvalue, etat, 25);
    
    return decvalue;
}


int main(int argc, char const *argv[])  {
    // Vecteurs de test pour le chiffrement
    char vecteurs_test[4][3][7] = { {"000000", "000000", "bb57e6"},
                                    {"ffffff", "000000", "739293"},
                                    {"000000", "ffffff", "1b56ce"},
                                    {"f955b9", "d1bd2d", "47a929"} };  

    char buf[10];
    for (size_t i = 0; i < 4; i++)  {
        snprintf ( buf, 10, "%x",  CHIFFREMENT(vecteurs_test[i][0], vecteurs_test[i][1]));
        if (!strcmp(vecteurs_test[i][2], buf))
            printf("\033[32mPassed: %s | %s | %s\033[00m\n", vecteurs_test[i][0], vecteurs_test[i][1], vecteurs_test[i][2]);
        else
            printf("\033[31mFailed: %s | %s | %s\033[00m\n", vecteurs_test[i][0], vecteurs_test[i][1], vecteurs_test[i][2]);
    }
    
    return 0;
}

/**
 * Déchiffrement :
 * - Inverse de Substitution (Boite S) -> inverser simplement le tableau
 * - Inverse de Permutation bit-à-bit
 * - Clés à l'envers
 */
