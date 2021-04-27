#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int CHIFFREMENT(char *etat_hex, char *cle_maitre_hex);
int DECHIFFREMENT(char *chiffre_hex, char *cle_maitre_hex);

int main(int argc, char const *argv[])  {    
    // Vecteurs de test pour le chiffrement
    char vecteurs_test[4][3][7] = { {"000000", "000000", "bb57e6"},
                                    {"ffffff", "000000", "739293"},
                                    {"000000", "ffffff", "1b56ce"},
                                    {"f955b9", "d1bd2d", "47a929"} };  
    char buf[10];
    printf("\033[33mChiffrement - Vecteurs de test:\n\033[00m");
    printf("        PlnMsg | MstKey | Cipher\n");
    for (size_t i = 0; i < 4; i++)  {
        snprintf ( buf, 10, "%x",  CHIFFREMENT(vecteurs_test[i][0], vecteurs_test[i][1]));
        if (!strcmp(vecteurs_test[i][2], buf))
            printf("\033[32mPassed: %s | %s | %s\033[00m\n", vecteurs_test[i][0], vecteurs_test[i][1], vecteurs_test[i][2]);
        else
            printf("\033[31mFailed: %s | %s | %s\033[00m\n", vecteurs_test[i][0], vecteurs_test[i][1], vecteurs_test[i][2]);
    }

    printf("\n\033[33mDéhiffrement - Vecteurs de test:\n\033[00m");
    printf("        Cipher | MstKey | PlnMsg\n");
    for (size_t i = 0; i < 4; i++)  {
        snprintf ( buf, 10, "%06x",  DECHIFFREMENT(vecteurs_test[i][2], vecteurs_test[i][1]));
        if (!strcmp(vecteurs_test[i][0], buf))
            printf("\033[32mPassed: %s | %s | %s\033[00m\n", vecteurs_test[i][2], vecteurs_test[i][1], vecteurs_test[i][0]);
        else
            printf("\033[31mFailed: %s | %s | %s || %s \033[00m\n", vecteurs_test[i][2], vecteurs_test[i][1], vecteurs_test[i][0], buf);
    }

    printf("\n\033[33mChiffrement puis déchiffrement de messages quelconques:\n\033[00m");
    char *plnmsg = "19234e";
    char *mstkey = "ce98f1";
    snprintf ( buf, 10, "%06x", CHIFFREMENT(plnmsg, mstkey) );
    snprintf ( buf, 10, "%06x", (DECHIFFREMENT(buf, mstkey)));
    if (!strcmp(plnmsg, buf))
        printf("\033[32mPassed: %s | %s | %s\033[00m\n", plnmsg, mstkey, buf);
    else
        printf("\033[31mFailed: %s | %s | %s\033[00m\n", plnmsg, mstkey, buf);

    return 0;
}
