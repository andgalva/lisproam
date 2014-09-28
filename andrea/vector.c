/* SOURCE: https://gist.github.com/EmilHernvall/953968
 * Simple vector implementation to store users' data
 * */

#include <stdio.h>
#include <stdlib.h>

#include "vector.h"
#include "andrea.h"

void vector_init(vector *v) {
	v->data = NULL;
	v->size = 0;
	v->count = 0;
}

int vector_count(vector *v) {
	return v->count;
}

void vector_add(vector *v, void *e) {
	if (v->size == 0) {
		v->size = 10;
		v->data = malloc(sizeof(void*) * v->size);
		memset(v->data, '\0', sizeof(void) * v->size);
	}
	// condition to increase v->data:
	// last slot exhausted
	if (v->size == v->count) {
		v->size *= 2;
		v->data = realloc(v->data, sizeof(void*) * v->size);
	}
	v->data[v->count] = e;
	v->count++;
}

void vector_set(vector *v, int index, void *e) {
	if (index >= v->count) {
		return;
	}
	v->data[index] = e;
}

void *vector_get(vector *v, int index) {
	if (index >= v->count) {
		return NULL;
	}
	return v->data[index];
}

void *vector_search_username(vector *v, char* un) {
	int i;
	for (i = 0; i < vector_count(v); i++) {
		struct user_info *ui = (struct user_info *) vector_get(v, i);
		if (ui != NULL && strcmp(ui->username, un) ==0) {
			return ui;
		}
	}
	return NULL;
}

void *vector_search_eid(vector *v, char* eid) {
	int i;
	for (i = 0; i < vector_count(v); i++) {
		struct user_info *ui = (struct user_info *) vector_get(v, i);
		if (ui != NULL && strcmp(ui->eid, eid) ==0) {
			return ui;
		}
	}
	return NULL;
}

void *vector_search_mac(vector *v, char* mac) {
	int i;
	for (i = 0; i < vector_count(v); i++) {
		struct user_info *ui = (struct user_info *) vector_get(v, i);
		if (ui != NULL && strcmp(ui->mac, mac) ==0) {
			return ui;
		}
	}
	return NULL;
}

void *vector_search_nonce(vector *v, uint64_t nonce) {
	int i;
	for (i = 0; i < vector_count(v); i++) {
		struct user_info *ui = (struct user_info *) vector_get(v, i);
		if (ui != NULL && ui->ms_nonce != NULL && ui->ms_nonce == nonce) {
			return ui;
		}
	}
	return NULL;
}

void *vector_get_map_server(vector *v, char* eid) {
	int i;
	for (i = 0; i < vector_count(v); i++) {
		struct user_info *ui = (struct user_info *) vector_get(v, i);
		if (ui != NULL && strcmp(ui->eid, eid) ==0 && ui->foreign == 1) // only foreign users
		{
			lispd_map_server_list_t *ms = (lispd_map_server_list_t *) malloc(sizeof(lispd_map_server_list_t));
			ms->address = (lisp_addr_t *) malloc(sizeof(lisp_addr_t));
			get_lisp_addr_from_char(ui->ms_address, ms->address);
			ms->key = (char *) malloc(sizeof(ui->ms_key));
			strcpy(ms->key, ui->ms_key);
			ms->key_type = 1;
			ms->proxy_reply = 1;
			ms->next = NULL;
			return ms;
		}
	}
	return NULL;
}

int vector_get_index(vector *v, char *eid) {
	int i;
	for (i = 0; i < vector_count(v); i++) {
		struct user_info *ui = (struct user_info *) vector_get(v, i);
		if (ui != NULL && strcmp(ui->eid, eid) ==0) {
			return i;
		}
	}
	return -1;
}

void vector_delete(vector *v, char* eid) {
	int index = vector_get_index(v, eid);
	if (index == -1) {
		return;
	}
	int i, j;
	void **newarr = (void**)malloc(sizeof(void*) * v->count * 2);
	for (i = 0, j = 0; i < v->count; i++) {
		if (v->data[i] != NULL) {
			newarr[j] = v->data[i];
			j++;
		}
	}
	free(v->data);
	v->data = newarr;
	v->count--;
}

void vector_free(vector *v) {
	free(v->data);
}

/* andrea START */

void vector_print(vector *v) {
	printf("---Vector---\n");
	printf("Count: %d\n", v->count);
	printf("Size: %d\n", v->size);
	printf("--- end ---\n");
}

/* andrea END */

int test(void) {
	vector v;
	vector_init(&v);
	vector_add(&v, "emil");
	vector_add(&v, "hannes");
	vector_add(&v, "lydia");
	vector_add(&v, "olle");
	vector_add(&v, "erik");
	int i;
	printf("first round:\n");
	for (i = 0; i < vector_count(&v); i++) {
	printf("%s\n", vector_get(&v, i));
	}
	vector_delete(&v, 1);
	vector_delete(&v, 3);
	printf("second round:\n");
	for (i = 0; i < vector_count(&v); i++) {
		printf("%s\n", vector_get(&v, i));
	}
	vector_free(&v);
	return 0;
}
