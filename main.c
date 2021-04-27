#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * TODO:
 * 2. Implantez la fonction de déchiffrement en C dans un fichier séparé. Pour que le déchiffrement fonctionne correctement, 
 * il faudra considérer les fonctions inverses de la boîte-S et de la permutation bit-à-bit P. 
 * Noter les inverses de ces fonctions dans le fichier texte qui accompagne votre projet. 
 * Vérifiez que votre d´echiffrement marche bien, en chiffrant un message quelconque avec PRESENT24 et en déchiffrant par la suite le chiffré obtenu.
 * 
 * - Inverse de Substitution (Boite S) -> inverser simplement le tableau
 * - Inverse de Permutation bit-à-bit
 * - Clés à l'envers
 */


int main(int argc, char const *argv[])  {

    // Chiffrement : 
    
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

    // Déchiffrement :


    return 0;
}
