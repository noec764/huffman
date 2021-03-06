#include "../include/encodage.h"
#include "../include/file.h"

//EXTERN
void compress(char *path_to_file)
{
	char compressed_filename[256] = {0};
	char bell_key[] = {(char)7, '\0'};
	unsigned char *m = NULL;
	unsigned char *file_read = NULL;
	p_encodage p_enc = NULL;

	// Ouvre et copie le contenu du fichier dans m
	printf("Lecture du contenu du fichier..\n");
	file_read = lire_caractere_fichier(path_to_file);
	// printf("%s\n", file_read);

	// Ajout du caractère de terminaison
	m = calloc(strlen((char *)file_read) + 1 + 1, sizeof(unsigned char)); // taille de la lecture + char d'arret + char NUL
	strcat((char *)m, (char *)file_read);
	strcat((char *)m, bell_key); // Char d'arrêt de compression "bell key"

	p_enc = create_encodage();

	// Calcule les fréquences dans le message et les places dans la structure sous forme d'un arbre
	// unique qui sera par la suite notre dictionnaire pour encode/decoder
	printf("Calcul des frequences..\n");
	frequences(m, p_enc);
	printf("Calcul de l'arbre par Huffman..\n");
	huffman(p_enc);

	// Encodage dico
	printf("Création de l'encodage par le dictionnaire..\n");
	create_code_arbre(p_enc->dico, p_enc);
	printf("Dico\n");
	print_encodage(p_enc); // Affichage test

	// Encodage message
	create_code_texte(p_enc, m);
	printf("Full\n");
	print_encodage(p_enc); // Affichage test

	// Compression
	compress_encodage(p_enc);

	strcpy(compressed_filename, path_to_file);
	strcat(compressed_filename, ".huf");

	printf("Ecriture du fichier compressé..\n");
	ecrire_bytes_fichier(compressed_filename, p_enc->b_enc, p_enc->b_length - 1); // On écrit tout sauf le bit de fin de chaine de caractère

	// Libération mémoire
	printf("Désallocation de la mémoire..\n");
	destruct_encodage(p_enc);
	free(m);
	printf("Compression terminee..\n");
}
//EXTERN

// COMPRESS
void compress_encodage(p_encodage p_enc)
{
	// Taille encodage en char représentant des bytes donc 8 char pour un byte
	int length = strlen((char *)p_enc->s_enc);

	// Aligne à 8 bits la longueur du résultat
	int mod = length % 8;
	length += mod;
	length /= 8;

	// Alloue l'espace mémoire nécessaire pour créer une chaine de byte
	p_enc->b_length = length + 1;
	p_enc->b_enc = calloc(p_enc->b_length, sizeof(byte));
	int i;
	for (i = 0; i < p_enc->b_length; i++)
	{
		byte *c = char_to_byte(p_enc->s_enc + i * 8);

		// Copie de la mémoire
		p_enc->b_enc[i] = *c;
		free(c);
		c = NULL;
	}

	// Ajout du caractère de fin
	p_enc->b_enc[i - 1] = '\0';
}
// COMPRESS

// DICO
Arbre *creer_liste_arbre(p_encodage enc, int *size)
{
	*size = 0;
	Arbre *t_noeud;
	Arbre n;
	int *tab_frequence = enc->tab_frequences;
	Arbre temp_noeuds[NB_ASCII] = {NULL};

	// On calcule le nombre de valeurs non nulle
	for (int i = 0; i < NB_ASCII; i++)
	{
		if (tab_frequence[i] != 0)
		{
			n = creer_arbre((unsigned char)i, tab_frequence[i], NULL, NULL);
			temp_noeuds[size[0]++] = n;
		}
	}

	printf("size %d\n", *size);
	// Allocation de la mémoire pour le tableau de noeud
	t_noeud = malloc(sizeof(Arbre) * (*size));
	// Copie de la mémoire de temp jusqu'au final
	for (int i = 0; i < *size; i++)
	{
		t_noeud[i] = malloc(sizeof(Noeud));
		memcpy(t_noeud[i], temp_noeuds[i], sizeof(Noeud));
	}

	return t_noeud;
}

int trouver_combiner(Arbre *l, int size)
{
	int pos_a = 0, pos_b = 0;
	Arbre a = NULL;
	Arbre b = NULL;

	// trouver minimum
	for (int i = 0; i < size; i++)
	{
		if (l[i] != NULL)
		{
			if (a == NULL || (b != NULL && l[i]->poid < (a)->poid))
			{
				if (b != NULL && (a)->poid < (b)->poid)
				{
					b = a;
					pos_b = pos_a;
				}

				a = l[i];
				pos_a = i;

				assert(a != b);
			}
			else if (b == NULL || l[i]->poid < (b)->poid)
			{
				b = l[i];
				pos_b = i;
			}
		}
	}

	if (a == NULL || b == NULL)
	{
		// Indique la fin du traitement de la liste
		return 0;
	}

	// Combiner les 2 arbres
	Arbre new_a = creer_arbre(a->elt, a->poid, a->fils_gauche, a->fils_droit);
	a->elt = '\0';
	a->fils_gauche = new_a;
	a->fils_droit = b;
	a->poid = a->poid + b->poid;
	// La liste exclue le deuxième élément, on réduit la liste
	l[pos_b] = NULL;

	return 1;
}

void huffman(p_encodage enc)
{
	int size = 0;
	Arbre *t_arbre = creer_liste_arbre(enc, &size);
	enc->dico = huffman_merge(t_arbre, size);
}

Arbre huffman_merge(Arbre *l, int size)
{
	while (trouver_combiner(l, size))
	{
	}

	for (int i = 0; i < size; i++)
	{
		if (l[i] != NULL)
		{
			Arbre a = malloc(sizeof(Noeud));
			memcpy(a, l[i], sizeof(Noeud));
			return a;
		}
	}

	return NULL;
}
// DICO

// CONSTR/DESTR
p_encodage create_encodage()
{
	p_encodage enc = (p_encodage)malloc(sizeof(encodage));
	enc->s_enc = malloc(sizeof(char));
	enc->s_enc[0] = '\0';
	enc->b_enc = NULL; // Init lors de son utilisation
	enc->b_length = 0;
	enc->dico = NULL;
	enc->tab_frequences = calloc(NB_ASCII, sizeof(int));
	return enc;
}

void destruct_encodage(p_encodage enc)
{
	if (enc && enc->s_enc)
	{
		free(enc->s_enc);
		enc->s_enc = NULL;
	}

	if (enc && enc->b_enc)
	{
		free(enc->b_enc);
	}

	if (enc && enc->dico)
	{
		detruire_arbre(enc->dico);
		enc->dico = NULL;
	}

	if (enc && enc->tab_frequences)
	{
		free(enc->tab_frequences);
		enc->tab_frequences = NULL;
	}

	if (enc)
	{
		free(enc);
	}
}
// CONSTR/DESTR

// GET/SET
unsigned char *s_encodage(p_encodage enc)
{
	return enc->s_enc;
}

int *t_frequences(p_encodage enc)
{
	return enc->tab_frequences;
}

unsigned char charAt_encodage(int i, p_encodage enc)
{
	return s_encodage(enc)[i];
}

void append_encodage(unsigned char *chaine, p_encodage enc)
{
	// Point d'amélioration, utiliser une allocation dynamique de mémoire et utiliser memcpy pour ajouter des éléments
	int length_enc = strlen((char *)s_encodage(enc));
	int length = strlen((char *)chaine);

	// Créer un nouvel espace mémoire qui peut contenir toutes les chaines
	unsigned char *s_new_encodage = (unsigned char *)calloc(length_enc + length + 1, sizeof(unsigned char));

	// Copie les contenus
	memcpy(s_new_encodage, s_encodage(enc), sizeof(unsigned char) * length_enc);
	memcpy(s_new_encodage + length_enc, chaine, sizeof(unsigned char) * length);
	s_new_encodage[length_enc + length] = '\0';

	// Désalloue l'ancien pointeur
	free(s_encodage(enc));
	enc->s_enc = s_new_encodage;
}
// GET/SET

// UTILS
void print_encodage(p_encodage enc)
{
	int length = strlen((char *)enc->s_enc);
	printf("Taille de l'encodage : %d\n", length);

	for (int i = 0; i < length; i++)
	{
		printf("%c", charAt_encodage(i, enc));
	}

	printf("\n");
}

void binaire(int entier, unsigned char s[ASCII_SIZE])
{
	int pt = 0;
	int puissance;

	/*
	* On passe en revue chaque 2^i pour savoir si un le bit i
	* peut être égal à 0 ou 1. On commence par le bit de poid fort.
	*/
	for (int i = ASCII_SIZE - 2; i >= 0; i--)
	{
		puissance = pow(2, i);

		// Si le résultat de la puissance est >= 0 alors le bit correspondant est égal à 1
		// Car si la différence est positive ou nul alors le résultat de la puissance est comprise
		// dans le reste et donc le nombre à écrire est plus grand ou égal que celle-ci.
		if ((entier - puissance) >= 0)
		{
			s[pt++] = '1';
			entier -= puissance;
		}
		else
		{
			s[pt++] = '0';
		}
	}
	s[pt] = '\0';
}

void code_ascii(unsigned char c, unsigned char *c_tab)
{
	binaire((int)c, c_tab);
}
// UTILS

void create_code_arbre(Arbre a, p_encodage enc)
{
	if (!est_feuille(a))
	{
		append_encodage((unsigned char *)"0", enc);
		create_code_arbre(fils_gauche(a), enc);
		create_code_arbre(fils_droit(a), enc);
	}
	else
	{
		append_encodage((unsigned char *)"1", enc);
		unsigned char c[ASCII_SIZE] = {'0'};
		code_ascii(racine(a), c);
		append_encodage(c, enc);
	}
}

void create_code_texte(p_encodage enc, unsigned char *m)
{
	Arbre dico = enc->dico;
	int length = strlen((char *)m);
	unsigned char code[16] = {0};

	for (int i = 0; i < length; i++)
	{
		rechercher_encodage(dico, m[i], code, 0);
		append_encodage(code, enc);
	}
}

void frequences(unsigned char *m, p_encodage enc)
{
	int *tab_freq = t_frequences(enc);
	int length = strlen((char *)m);

	for (int i = 0; i < length; i++)
	{
		tab_freq[(unsigned int)m[i]]++;
	}
}

/*
// TESTS
int main()
{
	// Test réel
	char *filename = "test_encodage.txt";
	compress(filename);
	// Test réel

	// p_encodage p_enc = create_encodage();

	// // Calculer la fréquence d'apparition des caractères d'un fichier :
	// char *text = "cagataagagaa";
	// int *p_frequence = t_frequences(p_enc); // Récupère l'adresse
	// frequences(text, p_frequence);
	// for (int i = 0; i < 255; i++)
	// {
	// 	if (p_frequence[i] != 0)
	// 	{
	// 		printf("'%c' : %d\n", (char)i, p_frequence[i]);
	// 	}
	// }

	// // On crée une liste de noeud qui plus tard sera un arbre
	// int size = 0;
	// Arbre *t_noeuds = creer_liste_arbre(p_enc, &size);
	// printf("Arbres : \n");
	// for (int i = 0; i < size; i++)
	// {
	// 	printf("'%c' : %d\n", t_noeuds[i]->elt, t_noeuds[i]->poid);
	// }

	// // On cherche les candidats pour le minimum et on réalise Huffman
	// Arbre final = huffman_merge(t_noeuds, size);
	// char code_c[56] = {0};
	// assert(final->poid == 10);
	// assert(final->fils_droit->poid == 6);
	// assert(final->fils_gauche->poid == 4);
	// // printf("valeur de 11 %c", final->fils_droit->fils_droit->elt);

	// Arbre a = creer_arbre('a', 1, NULL, NULL);
	// Arbre b01 = creer_arbre('b', 1, NULL, NULL);
	// Arbre b02 = creer_arbre('c', 1, NULL, NULL);
	// Arbre b = creer_arbre('\0', 1, b01, b02);
	// Arbre o = creer_arbre('\0', 2, a, b);
	// rechercher_encodage(o, 'b', code_c, 0);
	// printf("code pour 'b' : %s\n", code_c);

	// // On désalloue l'arbre, pas besoin de lui pour le moment
	// detruire_liste_arbre(t_noeuds, size);

	// // On imagine un début de suite telle que :
	// append_encodage("00001", p_enc);

	// // Le contenu dans encodage est le même :
	// assert(strcmp(s_encodage(p_enc), "00001") == 0);

	// // On veut vérifier que l'on trouve bien le 1 à la position 4 :
	// assert(charAt_encodage(4, p_enc));

	// // On ajoute un A (01000001) dans le code, on veut vérifier l'ajout :
	// char c[ASCII_SIZE + 1] = {0};
	// c[ASCII_SIZE] = '\0';
	// code_ascii('A', c);
	// assert(strcmp(c, "01000001") == 0);
	// append_encodage(c, p_enc);

	// // On veut voir la chaine obtenue :
	// print_encodage(p_enc);
	// assert(strcmp(s_encodage(p_enc), "0000101000001") == 0);

	// destruct_encodage(p_enc);

	return 0;
}
*/