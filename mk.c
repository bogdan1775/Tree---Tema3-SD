#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET_SIZE 26
#define MAX 1000

#define DIE(assertion, call_description)  \
	do {                                  \
		if (assertion) {                  \
			fprintf(stderr, "(%s, %d): ", \
					__FILE__, __LINE__);  \
			perror(call_description);     \
			exit(errno);                  \
		}                                 \
	} while (0)

typedef struct trie_node_t trie_node_t;
struct trie_node_t {
	// 1 daca este sfarsit de nod, 0 daca nu
	int end_of_word;

	trie_node_t **children;
	int n_children;
	int nr_cuv;
};

typedef struct trie_t trie_t;
struct trie_t {
	trie_node_t *root;

	int size;

	int data_size;
	int alphabet_size;

	int n_nodes;
};

// functia creeaza un nod din trie
trie_node_t *trie_create_node(void)
{
	// aloca memorie pentru nod
	trie_node_t *new;
	new = malloc(sizeof(trie_node_t));
	DIE(!new, "Alocare esuata");

	new->children = calloc(ALPHABET_SIZE, sizeof(trie_node_t *));
	DIE(!new->children, "Alocare esuata");

	// initializeaza datele
	new->end_of_word = 0;
	new->n_children = 0;
	new->nr_cuv = 0;
	return new;
}

// functia creeaza trie-ul
trie_t *trie_create(int data_size, int alphabet_size)
{
	trie_t *trie;
	trie = malloc(sizeof(trie_t));
	DIE(!trie, "Alocare esuata");

	trie->data_size = data_size;

	trie->root = trie_create_node();

	// initializeaza datele
	trie->size = 0;
	trie->alphabet_size = alphabet_size;
	trie->n_nodes = 0;

	return trie;
}

// functia insereaza un cuvant in trie
void trie_insert_node(trie_t *trie, trie_node_t *node, char *key)
{
	// daca s-a ajuns la sfarsitul cuvantului
	if (strlen(key) == 0) {
		node->end_of_word = 1;
		node->nr_cuv++;
		return;
	}

	int prima_litera = key[0] - 'a';
	trie_node_t *next_node = node->children[prima_litera];

	if (!next_node) {
		next_node = trie_create_node();
		node->n_children++;
		trie->n_nodes++;
		node->children[prima_litera] = next_node;
	}

	// se trece la urmatoarea litera
	trie_insert_node(trie, next_node, key + 1);
}

void trie_insert(trie_t *trie, char *key)
{
	trie_insert_node(trie, trie->root, key);
}

// functia sterge un cuvant din trie
int trie_remove_node(trie_t *trie, trie_node_t *node, char *key)
{
	// daca s-a ajuns la sfarsitul cuvantului
	if (strlen(key) == 0) {
		if (node->end_of_word == 1) {
			node->end_of_word = 0;
			node->nr_cuv = 0;

			return (node->n_children == 0);
		}
		return 0;
	}

	int prima_litera = key[0] - 'a';
	trie_node_t *next_node = node->children[prima_litera];

	if (next_node && trie_remove_node(trie, next_node, key + 1) == 1) {
		if (next_node->children)
			free(next_node->children);
		free(next_node);

		node->children[prima_litera] = NULL;
		node->n_children--;
		trie->n_nodes--;

		if (node->n_children == 0 && node->end_of_word == 0)
			return 1;
	}

	return 0;
}

void trie_remove(trie_t *trie, char *key)
{
	trie_remove_node(trie, trie->root, key);
}

// functia elibereaza memoria trie-ului
void trie_free_node(trie_node_t **node)
{
	if ((*node) == NULL)
		return;

	// se elibereaza pentru fiecare copil
	for (int i = 0; i < ALPHABET_SIZE; ++i)
		if ((*node)->children[i])
			trie_free_node(&(*node)->children[i]);

	if ((*node)->children)
		free((*node)->children);

	if (*node)
		free((*node));

	*node = NULL;
}

void trie_free(trie_t **trie)
{
	trie_free_node(&((*trie)->root));
	free((*trie));
	*trie = NULL;
}

// functia insereaza un cuvant in trie
void inserare_cuvant(trie_t *trie)
{
	char *cuvant;
	cuvant = malloc(MAX * sizeof(char));
	DIE(!cuvant, "Alocare esuata");

	scanf("%s", cuvant);
	// se insereaza cuvantul citit
	trie_insert(trie, cuvant);

	free(cuvant);
}

// functia afiseaza cuvintele care difera prin maxim k litere
void trie_dfs(trie_node_t *node, int nivel, int nivel_max, int k, char *cuvant,
			  int *ok, char *cpy_cuvant)
{
	if (nivel > nivel_max)
		return;

	// verifica daca cuvantul exista in trie
	if (node->end_of_word && nivel == nivel_max) {
		int nr = 0;
		int size_cuv = strlen(cuvant);
		cpy_cuvant[nivel] = '\0';

		// numara cate litere difera
		for (int i = 0; i < size_cuv; i++)
			if (cpy_cuvant[i] != cuvant[i])
				nr++;

		// il afiseaza daca difera maxim k litere
		if (nr <= k) {
			printf("%s\n", cpy_cuvant);
			*ok = 1;
		}

		return;
	}

	// verifica in timp ce construieste daca difera prin mai mult de k litere
	cpy_cuvant[nivel] = '\0';
	if (cpy_cuvant) {
		int nr = 0;
		int size = strlen(cpy_cuvant);
		for (int i = 0; i < size; i++)
			if (cpy_cuvant[i] != cuvant[i])
				nr++;
		if (nr > k)
			return;
	}

	for (int i = 0; i < 26; i++) {
		if (node->children[i]) {
			// se construieste cuvantul
			if (node->children[i])
				cpy_cuvant[nivel] = 'a' + i;

			trie_dfs(node->children[i], nivel + 1, nivel_max, k, cuvant, ok,
					 cpy_cuvant);
		}
	}
}

// functia afiseaza toate cuvintele care difera prin maxim k litere
void autocorrect_cuvant(trie_t *trie)
{
	char *cuvant;
	cuvant = malloc(MAX * sizeof(char));
	DIE(!cuvant, "Alocare esuata");

	int k;
	scanf("%s", cuvant);
	scanf("%d", &k);
	int nivel = 0;

	// in cpy_cuvant se construieste cuvantul
	char *cpy_cuvant;
	cpy_cuvant = malloc(MAX * sizeof(char));
	DIE(!cpy_cuvant, "Alocare esuata");

	// ok reprezinta daca s-a gasit macar un cuvant
	int ok = 0;
	trie_dfs(trie->root, nivel, strlen(cuvant), k, cuvant, &ok, cpy_cuvant);

	if (ok == 0)
		printf("No words found\n");

	free(cuvant);
	free(cpy_cuvant);
}

// functia afiseaza primul cuvant cu prefixul dat daca exista
void dfs_nr_crt_1(trie_node_t *node, int *ok, char *cpy_cuvant,
				  int indice_cuvant)
{
	// verifica daca cuvantul exista si daca da il afiseaza
	if (node->end_of_word == 1) {
		cpy_cuvant[indice_cuvant] = '\0';
		printf("%s\n", cpy_cuvant);

		// se retine daca s-a gasit un cuvant
		*ok = 1;
		return;
	}

	for (int i = 0; i < 26 && *ok == 0; i++) {
		if (node->children[i]) {
			// se construieste cuvantul
			if (node->children[i])
				cpy_cuvant[indice_cuvant] = 'a' + i;

			dfs_nr_crt_1(node->children[i], ok, cpy_cuvant, indice_cuvant + 1);
		}
	}
}

// functia afiseaza cel mai scurt cuvant cu prefixul dat
void dfs_nr_crt_2(trie_node_t *node, char *cpy_cuvant, int indice_cuvant,
				  char *cuvant)
{
	// verifica daca exista cuvantul
	if (node->end_of_word == 1) {
		cpy_cuvant[indice_cuvant] = '\0';

		// se verifica lungimea cuvintelor
		if (strlen(cuvant) == 0)
			strcpy(cuvant, cpy_cuvant);
		else if (strlen(cuvant) > strlen(cpy_cuvant))
			strcpy(cuvant, cpy_cuvant);
		return;
	}

	for (int i = 0; i < 26; i++) {
		if (node->children[i]) {
			// se construieste cuvantul
			if (node->children[i])
				cpy_cuvant[indice_cuvant] = 'a' + i;
			dfs_nr_crt_2(node->children[i], cpy_cuvant, indice_cuvant + 1,
						 cuvant);
		}
	}
}

// functia retine in sirul cuvant cuvantul cu frecventa cea mai mare
void dfs_nr_crt_3(trie_node_t *node, char *cpy_cuvant, int *maxi,
				  int indice_cuvant, char *cuvant)
{
	// se verifica daca exista cuvantul
	if (node->end_of_word) {
		// se verifica frecventa cuvintelor
		cpy_cuvant[indice_cuvant] = '\0';
		if (strlen(cuvant) == 0) {
			strcpy(cuvant, cpy_cuvant);
			*maxi = node->nr_cuv;
		} else if (node->nr_cuv > *maxi) {
			strcpy(cuvant, cpy_cuvant);
			*maxi = node->nr_cuv;
		}
	}

	for (int i = 0; i < 26; i++)
		if (node->children[i]) {
			// se contruieste cuvantul in cpy_cuvant
			if (node->children[i])
				cpy_cuvant[indice_cuvant] = 'a' + i;

			dfs_nr_crt_3(node->children[i], cpy_cuvant, maxi,
						 indice_cuvant + 1, cuvant);
		}
}

// functia afiseaza primul cuvant cu prefixul dat daca exista
void aplicatre_crt_1(trie_t *trie, char *prefix)
{
	trie_node_t *node;
	node = trie->root;
	int indice_cuvant = 0;

	// in cpy_cuvant se construieste cuvantul
	char *cpy_cuvant;
	cpy_cuvant = malloc(MAX * sizeof(char));
	DIE(!cpy_cuvant, "Alocare esuata");

	// se construieste cuvantul pana la prefix
	while (strlen(prefix)) {
		if (node->children[prefix[0] - 'a']) {
			node = node->children[prefix[0] - 'a'];
			cpy_cuvant[indice_cuvant++] = prefix[0];
		} else {
			printf("No words found\n");
			free(cpy_cuvant);
			return;
		}
		prefix = prefix + 1;
	}

	int ok = 0;
	// se apeleaza functia pentru a afisa cuvantul daca exista
	dfs_nr_crt_1(node, &ok, cpy_cuvant, indice_cuvant);

	free(cpy_cuvant);
}

// functia afiseaza cel mai scurt cuvant cu prefixul dat
void aplicatre_crt_2(trie_t *trie, char *prefix)
{
	trie_node_t *node;
	node = trie->root;
	char *cuvant, *cpy_cuvant;

	// cuvant retine cuvantul cel mai scurt
	cuvant = malloc(MAX * sizeof(char));
	DIE(!cuvant, "Alocare esuata");

	// in cpy_cuvant se construieste cuvantul
	cpy_cuvant = malloc(MAX * sizeof(char));
	DIE(!cpy_cuvant, "Alocare esuata");

	int indice_cuvant = 0;
	cuvant[0] = '\0';

	// se sconstruieste cuvantul pana la ultima litera din prefix
	while (strlen(prefix)) {
		if (node->children[prefix[0] - 'a']) {
			node = node->children[prefix[0] - 'a'];
			cpy_cuvant[indice_cuvant++] = prefix[0];
		} else {
			printf("No words found\n");
			free(cuvant);
			free(cpy_cuvant);
			return;
		}
		prefix = prefix + 1;
	}

	// se apeleaza functia pentru a determina cuvantul
	dfs_nr_crt_2(node, cpy_cuvant, indice_cuvant, cuvant);

	printf("%s\n", cuvant);
	free(cuvant);
	free(cpy_cuvant);
}

// functia afiseaza cel mai frecvent cuvant
void aplicatre_crt_3(trie_t *trie, char *prefix)
{
	trie_node_t *node;
	node = trie->root;
	char *cuvant, *cpy_cuvant;

	// cuvant retine cuvantul cel mai frecvent
	cuvant = malloc(MAX * sizeof(char));
	DIE(!cuvant, "Alocare esuata");

	// in cpy_cuvant construiesc cuvantul
	cpy_cuvant = malloc(MAX * sizeof(char));
	DIE(!cpy_cuvant, "Alocare esuata");

	int indice_cuvant = 0;
	cuvant[0] = '\0';

	// se sconstruieste cuvantul pana la ultima litera din prefix
	while (strlen(prefix)) {
		if (node->children[prefix[0] - 'a']) {
			node = node->children[prefix[0] - 'a'];
			cpy_cuvant[indice_cuvant++] = prefix[0];
		} else {
			printf("No words found\n");
			free(cuvant);
			free(cpy_cuvant);
			return;
		}
		prefix = prefix + 1;
	}

	// maxi retine nr maxim de aparitii
	int maxi = 0;
	dfs_nr_crt_3(node, cpy_cuvant, &maxi, indice_cuvant, cuvant);

	printf("%s\n", cuvant);
	free(cuvant);
	free(cpy_cuvant);
}

// functia aplica toate cele 3 criterii
void aplicatre_crt_0(trie_t *trie, char *prefix)
{
	aplicatre_crt_1(trie, prefix);
	aplicatre_crt_2(trie, prefix);
	aplicatre_crt_3(trie, prefix);
}

// functia aplica unul din criterii in functie de criteriul citit
void autocomplete_cuvant(trie_t *trie)
{
	char *prefix;
	prefix = malloc(MAX * sizeof(char));
	DIE(!prefix, "Alocare esuata");

	int nr_crit;
	scanf("%s", prefix);
	scanf("%d", &nr_crit);
	if (nr_crit == 1)
		aplicatre_crt_1(trie, prefix);

	if (nr_crit == 2)
		aplicatre_crt_2(trie, prefix);

	if (nr_crit == 3)
		aplicatre_crt_3(trie, prefix);

	if (nr_crit == 0)
		aplicatre_crt_0(trie, prefix);

	free(prefix);
}

// functia adauga in trie cuvintele dintr-un fisier
void incarcare_fisier(trie_t *trie)
{
	char *filename;
	filename = malloc(MAX * sizeof(char));
	DIE(!filename, "Alocare esuata");

	scanf("%s", filename);

	// se deschide fisierul
	FILE *in = fopen(filename, "rt");
	DIE(!filename, "Nu s-a putut deschide fisierul");

	char *cuvant;
	cuvant = malloc(MAX * sizeof(char));
	DIE(!cuvant, "Alocare esuata");

	// se citeste fiecare cuvant in parte
	while (feof(in) == 0) {
		fscanf(in, "%s", cuvant);
		// se insereaza fiecare cuvant
		trie_insert(trie, cuvant);
	}

	fclose(in);

	free(cuvant);
	free(filename);
}

// functia sterge un cuvant din trie
void stergere_cuvant(trie_t *trie)
{
	char *cuvant;
	cuvant = malloc(MAX * sizeof(char));
	DIE(!cuvant, "Alocare esuata");

	scanf("%s", cuvant);

	// se elimina cuvantul din trie
	trie_remove(trie, cuvant);

	free(cuvant);
}

// functia elibereaza toate memoria utilizata de trie
void eliberare_memorie(trie_t *trie)
{
	trie_free(&trie);
}

int main(void)
{
	trie_t *trie;
	trie = trie_create(sizeof(char), ALPHABET_SIZE);

	char *comanda;
	comanda = malloc(MAX * sizeof(char));
	DIE(!comanda, "Alocare esuata");

	while (1) {
		scanf("%s", comanda);
		if (strcmp(comanda, "INSERT") == 0) {
			inserare_cuvant(trie);

		} else if (strcmp(comanda, "LOAD") == 0) {
			incarcare_fisier(trie);

		} else if (strcmp(comanda, "REMOVE") == 0) {
			stergere_cuvant(trie);

		} else if (strcmp(comanda, "AUTOCORRECT") == 0) {
			autocorrect_cuvant(trie);

		} else if (strcmp(comanda, "AUTOCOMPLETE") == 0) {
			autocomplete_cuvant(trie);

		} else if (strcmp(comanda, "EXIT") == 0) {
			eliberare_memorie(trie);
			break;
		}
	}

	free(comanda);

	return 0;
}
