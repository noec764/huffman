#ifndef __file_h__
#define __file_h__
#include "arbre.h"
#include <string.h>
#include <math.h>

typedef char byte;

FILE *ouvrir_fichier(char *filename);
void fermer_fichier(FILE *file);

char *lire_caractere_fichier(char *file);

/// TODO
// void ecrire_bit(FILE *file, char bit);
// void lire_bit(FILE *file);

byte *char_to_byte(char *char_to_convert);
char *byte_to_char(byte byte_to_convert);

#endif