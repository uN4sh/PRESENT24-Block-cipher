#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

void CADENCEMENT_CLE_OPTI(uint32_t m_key, uint32_t *sous_cles);
int  CHIFFREMENT_OPTI(uint32_t etat, uint32_t *sous_cles);
int  DECHIFFREMENT_OPTI(uint32_t etat, uint32_t *sous_cles);
int  CHIFFREMENT_DOUBLE_OPTI(uint32_t message, uint32_t k1, uint32_t k2);

int main(int argc, char const *argv[])  { 
    // Vecteurs de test pour le chiffrement
    uint32_t vecteurs_int[4][3] = { {0x000000, 0x000000, 0xbb57e6},
                                     {0xffffff, 0x000000, 0x739293},
                                     {0x000000, 0xffffff, 0x1b56ce},
                                     {0xf955b9, 0xd1bd2d, 0x47a929} };
        

    // Sortie des chiffrements et déchiffrements
    uint32_t res_chiffre, res_clair;
    uint32_t sous_cles[11];
    printf("\033[33mChiffrement - Vecteurs de test:\n\033[00m");
    printf("         Clair |    Clé | Cipher\n");
    for (int i = 0; i < 4; i++)  {
        // Génération des 11 sous clés de 24 bits chacune via algo. cadencement de clés
        CADENCEMENT_CLE_OPTI(vecteurs_int[i][1], sous_cles);
        res_chiffre = CHIFFREMENT_OPTI(vecteurs_int[i][0], sous_cles);
        if (res_chiffre == vecteurs_int[i][2])
            printf("\033[32mPassed: %06x | %06x | %06x\033[00m\n", vecteurs_int[i][0], vecteurs_int[i][1], vecteurs_int[i][2]);
        else
            printf("\033[31mFailed: %06x | %06x | %06x || %06x\033[00m\n", vecteurs_int[i][0], vecteurs_int[i][1], vecteurs_int[i][2], res_chiffre);
    }


    printf("\n\033[33mDéhiffrement - Vecteurs de test:\n\033[00m");
    printf("        Cipher |  Clé   | Clair\n");
    for (size_t i = 0; i < 4; i++)  {
        CADENCEMENT_CLE_OPTI(vecteurs_int[i][1], sous_cles);
        res_clair = DECHIFFREMENT_OPTI(vecteurs_int[i][2], sous_cles);
        if (res_clair == vecteurs_int[i][0])
            printf("\033[32mPassed: %06x | %06x | %06x\033[00m\n", vecteurs_int[i][0], vecteurs_int[i][1], vecteurs_int[i][2]);
        else
            printf("\033[31mFailed: %06x | %06x | %06x || %06x\033[00m\n", vecteurs_int[i][0], vecteurs_int[i][1], vecteurs_int[i][2], res_clair);
    }
    
    
    printf("\n\033[33mChiffrement puis déchiffrement de messages quelconques:\n\033[00m");
    uint32_t clairs[][3] = {{0x19234e, 0xce98f1, 0x314159},
                            {0xabc123, 0x456def, 0xabcdef},
                            {0x198ad6, 0x000023, 0x0000aa}
                           };

    printf("        Clair  |  Clé   | \033[34mCipher\033[0m | \033[35mDéchiffré\033[0m\n");
    for (size_t i = 0; i < 2; i++)  {
        CADENCEMENT_CLE_OPTI(clairs[i][1], sous_cles);
        res_chiffre = CHIFFREMENT_OPTI(clairs[i][0],  sous_cles);
        res_clair   = DECHIFFREMENT_OPTI(res_chiffre, sous_cles);

        if (clairs[i][0] == res_clair)
            printf("\033[32mPassed: %06x | %06x | \033[34m%06x | \033[35m%06x\033[00m\n", clairs[i][0], clairs[i][1], res_chiffre, res_clair);
        else
            printf("\033[31mFailed: %06x | %06x | \033[34m%06x | \033[35m%06x\033[00m\n", clairs[i][0], clairs[i][1], res_chiffre, res_clair);
    }
    
    return 0;
}
