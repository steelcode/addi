
#ifndef LISTA_H
#define LISTA_H


typedef int Bool;
#define TRUE 1
#define FALSE 0

typedef struct nodo_lista *lista;
struct nodo_lista {
	void * dh;
	char clname[256];
	char meth_name[256];
	char meth_sig[256];
	char hashvalue[512];
	lista next;
};

int inserisci(lista*, void *, char*, char*, char*,char*);
void cancella(lista*, void *);
void * cerca(lista p_L, char *hash);

#endif