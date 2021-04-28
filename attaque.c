#include <stdio.h>
#include <stdlib.h>


/**
 * Attaque par le milieu :
 * On a (m, c) et on cherche (k1, k2)
 * 
 * Nombre de clés : 
 * // DES : longueur de la clé : 56 bits, nombre de clés : 2^56
 * // PRESENT-24 : 6 caractères, alphabet de 16 possibles, 24 bits -> 2^24 ?
 * 
 *  1. Construire deux listes Lc et Lm tel que :
 *      Pour tout k parmi 2^24, stocker le résultat du   chiffrement de m dans Lm  (clair | clé) 
 *      Pour tout k parmi 2^24, stocker le résultat du déchiffrement de c dans Lc  (chiffré | clé) 
 * 
 *  2. Chercher les éléments communs dans les deux listes :
 *      t tel que : .... 
 *      Méthode :
 *          - Trier les deux listes - O(2 * n*log n)
 *          - Chercher les éléments communs - O(n)
 *          Avec n = 2^24
 *          
 */


