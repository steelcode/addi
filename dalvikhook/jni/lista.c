#include <stdlib.h>
#include "lista.h"
#include "dalvik_hook.h"
#include "log.h"

int inserisci(lista *p_L, void* d, char *cls, char* meth, char * sign, char *hash){
	lista tmp;
	//struct dalvik_hook_t* dhtmp = (struct dalvik_hook_t*) malloc(sizeof(struct dalvik_hook_t*));
	//memcpy(dhtmp, d, sizeof(struct dalvik_hook_t));
	if(*p_L == NULL){
		*p_L = (lista) malloc(sizeof(struct nodo_lista));
		if ( !*p_L ) return 0;
		//(*p_L)->dh = d;
		(*p_L)->dh =  (struct dalvik_hook_t *) malloc(sizeof(struct dalvik_hook_t *));
		(*p_L)->dh = d;
		//memcpy((*p_L)->dh, d, sizeof(struct dalvik_hook_t));
		
		strcpy( (*p_L)->clname, cls  );
		strcpy( (*p_L)->meth_name, meth  );
		strcpy( (*p_L)->meth_sig, sign  );
		strcpy( (*p_L)->hashvalue, hash  );
		(*p_L)->next=NULL;
		return 1;
	}
	for(tmp=*p_L;tmp->next!=NULL;tmp=tmp->next);
	tmp->next=(lista)malloc(sizeof(struct nodo_lista));
	if ( !tmp->next ) return 0;
	//(tmp->next)->dh = d;
	(tmp->next)->dh = (struct dalvik_hook_t *) malloc(sizeof(struct dalvik_hook_t*));
	(tmp->next)->dh = d;
	//memcpy((tmp->next)->dh, d, sizeof(struct dalvik_hook_t));

	strcpy( (tmp->next)->clname, cls  );
	strcpy( (tmp->next)->meth_name, meth  );
	strcpy( (tmp->next)->meth_sig, sign  );
	strcpy( (tmp->next)->hashvalue, hash  );
	(tmp->next)->next = NULL;
	return 1;
}
void * cerca(lista l, char *hash){
	while((l) != NULL){
		if( strcmp( (l)->hashvalue, hash) == 0 )
			return (l)->dh;			
		(l) = (l)->next;
	}
	return (void *)0;
}

void cancella(lista *p_L, void * d){

}
void stampa_lista(lista l){
	struct dalvik_hook_t* dd;
	while((l) != NULL){
		dd = (struct dalvik_hook_t *)(l)->dh;
		log("+++++++++++++++++++++++++++++++++++++++++++\n")
		//log("clname = %s\n", (l)->clname)
		//log("meth_name = %s\n", (l)->meth_name)
		//log("meth_sign = %s\n", (l)->meth_sig)
		log("hash = %s\n", (l)->hashvalue)
		log("real num args = %d\n", dd->real_args)
		log("DALVIK HOOK = 0x%x\n", (unsigned int) dd->method)
		log("+++++++++++++++++++++++++++++++++++++++++++\n")
		(l) = (l)->next;
	}
}