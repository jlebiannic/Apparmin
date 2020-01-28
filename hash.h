#ifdef _WIN32
#include <malloc.h>
#include <process.h>
#else
#include <stdlib.h>
#endif
// Type des cellules pour les listes chaînées
struct Cellule {
  char* str;		// Pointeur sur la chaîne
  int nbr;			// Nombre d'occurrences
  Cellule *next;    // Pointeur sur la cellule suivante
  void *m;			// Membre attache
};

// Type des listes chaînées
typedef Cellule* Liste;

// Type pour les tables de hachage
struct HashTable {
  int mod;              // Nombre d'éléments de la table
  Liste *table;			// Pointeur sur la table allouée
};

// Modulo par défaut pour les tables de hachage
const int MOD = 701;

// Fonction de hachage pour une chaîne de caractères
// Calcul d'un entier compris entre 0 et mod-1 inclus
int hash(char* str, int mod)
{
  int h = 0;			// Valeur calculée
  int i;			// Indice de parcours de la chaîne

  for (i = 0; str[i] != '\0'; i++) {
    h = h*256 + (unsigned int) str[i];
    if (h >= mod)
      h %= mod;
  }
  return h;
}

// Allocation d'une cellule
Cellule* cellAlloc(char* str, void *m)
{
  // Allocation de la cellule proprement dite
  Cellule* c = (Cellule*) malloc(sizeof(Cellule));
  if (c == 0) {
    fprintf(stderr, "Plus de mémoire\n");
    exit(1);
  }
  // Allocation de la mémoire pour stocker la chaîne
  // Il faut allouer 1 octet de plus pour le '\0' de fin de chaîne.
  if ((c->str = (char *) malloc(strlen(str) + 1)) == 0) {
    fprintf(stderr, "Plus de mémoire\n");
    exit(1);
  }
  // Copie de la  chaîne dans la mémoire allouée
  strcpy(c->str, str);
  c->m = m; //attache le Membre
  // Initialisation du nombre d'occurrences
  c->nbr = 1;

  return c;
}

// Insertion d'une cellule dans une liste
// L'insertion est faite en tête de liste
void listeInsert(Liste& l, Cellule* c)
{
  c->next = l;
  if(l)
	  c->nbr = (l->nbr)+1;
  l = c;

}

// Affichage d'une liste
void listePrint(Liste l, FILE *f_hash)
{
	while (l != 0) {
		fprintf(f_hash,"	%s\n", l->str);
	l = l->next;
  }
}

// Allocation et initialisation d'une table de hachage
void hashTableInit(HashTable& ht, int mod)
{
  // Nombre d'éléments de la table
  ht.mod = mod;

  // Allocation de la table
  if ((ht.table = (Liste*) malloc(mod * sizeof(Liste))) == 0) {
    fprintf(stderr, "Plus de mémoire\n");
    exit(1);
  }

  // Initialisation de la table
  // Au début toutes les listes sont vides
  for (int i = 0; i < mod; i++)
    ht.table[i] = NULL;		// Liste vide
}


// Affichage complet d'une table de hachage
void hashTablePrint(HashTable& ht, FILE *f_hash)
{
  for (int i = 0; i < ht.mod; i++)
    if (ht.table[i] != 0) {
      fprintf(f_hash,"Entree %d [%d]\n", i, ht.table[i]->nbr);
      //listePrint(ht.table[i], f_hash);

    }
	
}

// Recherche une chaîne dans une table.
void* hashTableFind(HashTable& ht, char* str)
{
  // Liste où doit se trouver la chaîne
  Liste& l = ht.table[hash(str, ht.mod)];

  // Parcours de la liste
  for (Liste p = l; p != 0; p = p->next)
    if (strcmp(p->str, str) == 0)
      return p->m;			// Bingo

  // La chaine n'a pas été trouvée.
  return NULL;
}

// Insertion d'un membre en fonction d'une chaîne dans une table
Cellule* hashTableInsert(HashTable& ht, char* str, void *m)
{
  // Liste où doit se trouver la chaîne
  Liste& l = ht.table[hash(str, ht.mod)];

  // Cellule pour l'insérer
  Cellule* c = cellAlloc(str, m);
  listeInsert(l, c);
  return c;
}

void hashTableDestroy(HashTable& ht)
{
	// Parcours de toutes les cellules
	for (int i = 0; i < ht.mod; i++)
	{
		Liste p;		
		Liste next;
		for (p = ht.table[i]; p != 0; p = next)
		{
			next = p->next;
			free(p->m);
			free(p->str);
			free(p);
		}
	}

	// Libération de la table
	free(ht.table);

}

