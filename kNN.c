#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NWORD 20
#define MAX 100

#define DIE(assertion, call_description)  \
	do {                                  \
		if (assertion) {                  \
			fprintf(stderr, "(%s, %d): ", \
					__FILE__, __LINE__);  \
			perror(call_description);     \
			exit(errno);                  \
		}                                 \
	} while (0)

typedef struct k_bst_node_t k_bst_node_t;
struct k_bst_node_t {
	// copilul din stanga
	k_bst_node_t *left;

	// copilul din dreapta
	k_bst_node_t *right;

	// data continuta de nod
	void *data;
};

typedef struct k_bst_tree_t k_bst_tree_t;
struct k_bst_tree_t {
	// root-ul tree-ului
	k_bst_node_t *root;

	// ce fel de k-d tree este
	int k;

	size_t data_size;
};

// aloca memorie pentru un nod si il initializeaza
static k_bst_node_t *__k_bst_node_create(void *data, size_t data_size)
{
	k_bst_node_t *k_bst_node;

	k_bst_node = malloc(sizeof(*k_bst_node));
	DIE(!k_bst_node, "k_bst_node malloc");

	k_bst_node->left = NULL;
	k_bst_node->right = NULL;

	k_bst_node->data = malloc(data_size);
	DIE(!k_bst_node->data, "k_bst_node->data malloc");

	memcpy(k_bst_node->data, data, data_size);

	return k_bst_node;
}

// aloca memorie pentru un k-d tree
k_bst_tree_t *k_bst_tree_create(size_t data_size, int k)
{
	k_bst_tree_t *k_bst_tree;

	k_bst_tree = malloc(sizeof(*k_bst_tree));
	DIE(!k_bst_tree, "k_bst_tree malloc");

	k_bst_tree->root = NULL;
	k_bst_tree->data_size = data_size;
	k_bst_tree->k = k;

	return k_bst_tree;
}

// functia insereaza un nod in k tree
k_bst_node_t *k_bst_tree_insert(k_bst_tree_t *k_bst, k_bst_node_t *root,
								void *data, int h)
{
	if (!root)
		return __k_bst_node_create(data, k_bst->data_size);

	// rest retine in functie de ce coordonata se compara
	int rest = h % k_bst->k;
	int *aux = (int *)data;
	int *data_nod = (int *)root->data;

	if (aux[rest] < data_nod[rest])
		root->left = k_bst_tree_insert(k_bst, root->left, data, h + 1);
	else
		root->right = k_bst_tree_insert(k_bst, root->right, data, h + 1);

	return root;
}

// functia elibereaza memoria k tree-ului in mod recursiv
static void __k_bst_tree_free(k_bst_node_t *k_bst_node)
{
	if (!k_bst_node)
		return;

	__k_bst_tree_free(k_bst_node->left);
	__k_bst_tree_free(k_bst_node->right);

	free(k_bst_node->data);
	free(k_bst_node);
}

void k_bst_tree_free(k_bst_tree_t *k_bst_tree)
{
	__k_bst_tree_free(k_bst_tree->root);
	free(k_bst_tree);
}

// se insereaza in k-d tree punctele citite si returnea k-d tree-ul creat
k_bst_tree_t *load_file(void)
{
	k_bst_tree_t *k_bst;

	char *filename;
	filename = malloc(MAX * sizeof(char));
	DIE(!filename, "Alocare esuata");

	// se deschide fisierul
	scanf("%s", filename);
	FILE *in = fopen(filename, "rt");
	DIE(!in, "Nu s-a putut deschide fisierul");

	// n numarul de puncte, k-numarul de coordonate
	int n, k;
	fscanf(in, "%d", &n);
	fscanf(in, "%d", &k);

	k_bst = k_bst_tree_create(k * sizeof(int), k);

	int nr = 0, coord;

	// data retine coordonatele unui punct
	int *data;
	data = malloc(k * sizeof(int));
	DIE(!data, "Alocare esuata");

	// se insereaza punctele in k-d tree
	while (nr < n) {
		for (int i = 0; i < k; i++) {
			fscanf(in, "%d", &coord);
			data[i] = coord;
		}
		k_bst->root = k_bst_tree_insert(k_bst, k_bst->root, data, 0);
		nr++;
	}

	// se inchide fisierul
	fclose(in);

	free(data);
	free(filename);

	return k_bst;
}

// functia cauta punctele care se afla la cea mai mica distanta
void find_point(k_bst_node_t *k_bst_node, int *data, int k, int *vect,
				int *size_vect, long *min_dist, int h)
{
	if (!k_bst_node)
		return;

	long distanta = 0;
	int *aux = (int *)k_bst_node->data;
	for (int i = 0; i < k; i++)
		distanta += (data[i] - aux[i]) * (data[i] - aux[i]);

	// daca se gaseste o distanta mai mica se actualizeaza min_dist
	if (distanta < *min_dist) {
		*min_dist = distanta;
		*size_vect = 1;
		for (int i = 0; i < k; i++)
			vect[i] = aux[i];
	}

	// daca are aceeasi distanta se retine si coordonatele acestui punct
	else if (distanta == *min_dist) {
		(*size_vect)++;
		for (int i = (*size_vect - 1) * k; i < *size_vect * k; i++)
			vect[i] = aux[i - (*size_vect - 1) * k];
	}

	// rest retine in functie de ce coordonata se face comparatia
	int rest = h % k;

	if (data[rest] < aux[rest]) {
		k_bst_node_t *node = k_bst_node->left;
		find_point(node, data, k, vect, size_vect, min_dist, h + 1);

	} else {
		k_bst_node_t *node = k_bst_node->right;
		find_point(node, data, k, vect, size_vect, min_dist, h + 1);
	}

	if ((data[rest] - aux[rest]) * (data[rest] - aux[rest]) <= *min_dist) {
		if (data[rest] < aux[rest]) {
			k_bst_node_t *node = k_bst_node->right;
			find_point(node, data, k, vect, size_vect, min_dist, h + 1);

		} else {
			k_bst_node_t *node = k_bst_node->left;
			find_point(node, data, k, vect, size_vect, min_dist, h + 1);
		}
	}
}

// functia afiseaza punctele care se afla la cea mai mica distanta
void nearest_neighbours(k_bst_tree_t *k_bst)
{
	// retine coordonatele punctului pentru care cautam
	int *data;
	data = malloc(k_bst->k * sizeof(int));
	DIE(!data, "Alocare esuata");

	for (int i = 0; i < k_bst->k; i++)
		scanf("%d", &data[i]);

	// distanta minima
	long min_dist = __LONG_MAX__;

	// vect retine coordonatele punctelor care se afla la distanta minima
	int *vect, size_vect = 1;
	vect = malloc(NWORD * k_bst->k * sizeof(int));
	DIE(!vect, "Alocare esuata");

	find_point(k_bst->root, data, k_bst->k, vect, &size_vect, &min_dist, 0);

	// se ordoneaza vectorul
	for (int i = 0; i < size_vect - 1; i++) {
		if (vect[i * k_bst->k] > vect[(i + 1) * k_bst->k]) {
			for (int j = 0; j < k_bst->k; j++) {
				int aux = vect[i * k_bst->k + j];
				vect[i * k_bst->k + j] = vect[(i + 1) * k_bst->k + j];
				vect[(i + 1) * k_bst->k + j] = aux;
			}
		} else if (vect[i * k_bst->k] > vect[(i + 1) * k_bst->k])
			if (vect[i * k_bst->k + 1] > vect[(i + 1) * k_bst->k + 1]) {
				for (int j = 0; j < k_bst->k; j++) {
					int aux = vect[i * k_bst->k + j];
					vect[i * k_bst->k + j] = vect[(i + 1) * k_bst->k + j];
					vect[(i + 1) * k_bst->k + j] = aux;
				}
			}
	}

	// se afiseaza
	for (int i = 0; i < size_vect; i++) {
		for (int j = 0; j < k_bst->k; j++)
			printf("%d ", vect[j + i * k_bst->k]);
		printf("\n");
	}

	free(vect);
	free(data);
}

// functia determina punctele care apartin intervalului
void check_coord(k_bst_node_t *k_bst_node, int *data, int k, int *vect,
				 int *size_vect, int h)
{
	if (!k_bst_node)
		return;

	int *aux = (int *)k_bst_node->data;

	// i retine in functie de ce coordonata se face comparatia
	int i = h % k;
	if (aux[i] <= data[2 * i]) {
		check_coord(k_bst_node->right, data, k, vect, size_vect, h + 1);

	} else if (aux[i] > data[2 * i + 1]) {
		check_coord(k_bst_node->left, data, k, vect, size_vect, h + 1);

	} else {
		check_coord(k_bst_node->left, data, k, vect, size_vect, h + 1);
		check_coord(k_bst_node->right, data, k, vect, size_vect, h + 1);
	}

	aux = (int *)k_bst_node->data;
	int ok = 1;

	// se verifica daca punctul are coordonatele in interval
	for (i = 0; i < k; i++)
		if (aux[i] < data[i * 2] || aux[i] > data[i * 2 + 1])
			ok = 0;

	// se retin coordonatele
	if (ok == 1) {
		(*size_vect)++;
		for (i = (*size_vect - 1) * k; i < (*size_vect) * k; i++)
			vect[i] = aux[i - (*size_vect - 1) * k];
	}
}

// functia afiseaza punctele care se afla in intervalul citit
void range_search(k_bst_tree_t *k_bst)
{
	// retine coordonatele punctului cautat
	int *data;
	data = malloc(2 * k_bst->k * sizeof(int));
	DIE(!data, "Alocare esuata");

	int aux;
	for (int i = 0; i < k_bst->k * 2; i++)
		scanf("%d", &data[i]);

	// vect retine coordonatle punctelor care se afla in intervalul citit
	int *vect, size_vect = 0;
	vect = malloc(NWORD * k_bst->k * sizeof(int));
	DIE(!vect, "Alocare esuata");

	check_coord(k_bst->root, data, k_bst->k, vect, &size_vect, 0);

	// se ordoneaza coordonatele punctelor care se afla in intervalul citit
	for (int i = 0; i < size_vect - 1; i++) {
		if (vect[i * k_bst->k] > vect[(i + 1) * k_bst->k]) {
			for (int j = 0; j < k_bst->k; j++) {
				aux = vect[i * k_bst->k + j];
				vect[i * k_bst->k + j] = vect[(i + 1) * k_bst->k + j];
				vect[(i + 1) * k_bst->k + j] = aux;
			}
		} else if (vect[i * k_bst->k] > vect[(i + 1) * k_bst->k])
			if (vect[i * k_bst->k + 1] > vect[(i + 1) * k_bst->k + 1]) {
				for (int j = 0; j < k_bst->k; j++) {
					aux = vect[i * k_bst->k + j];
					vect[i * k_bst->k + j] = vect[(i + 1) * k_bst->k + j];
					vect[(i + 1) * k_bst->k + j] = aux;
				}
			}
	}

	// se afiseaza punctele
	for (int i = 0; i < size_vect; i++) {
		for (int j = 0; j < k_bst->k; j++)
			printf("%d ", vect[j + i * 2]);
		printf("\n");
	}

	free(vect);
	free(data);
}

// se elibereaza memoria totala
void free_memory(k_bst_tree_t *k_bst)
{
	k_bst_tree_free(k_bst);
}

int main(void)
{
	k_bst_tree_t *k_bst;
	char *command;
	command = malloc(MAX * sizeof(char));

	while (1) {
		scanf("%s", command);
		if (strcmp(command, "LOAD") == 0) {
			k_bst = load_file();

		} else if (strcmp(command, "NN") == 0) {
			nearest_neighbours(k_bst);

		} else if (strcmp(command, "RS") == 0) {
			range_search(k_bst);

		} else {
			free_memory(k_bst);
			break;
		}
	}

	free(command);

	return 0;
}
