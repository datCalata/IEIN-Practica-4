/*
 * msg_list.c
 *
 *  Created on: 26 de abr. de 2017
 *      Author: Yo
 */

#include "msg_list.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

msg_list_t* msg_list_init(void){
	msg_list_t* new_msg_list = (msg_list_t*) malloc(sizeof(msg_list_t));
	new_msg_list->msg = NULL;
	new_msg_list->next = NULL;
	return new_msg_list;
}

int msg_list_delete(msg_list_t* msg_list){
	//TODO TEXT IT
	msg_list_t* aux;
	while(msg_list->next != NULL){
		aux = msg_list->next;
		free(aux->msg);
		msg_list->next = aux->next;
		free(aux);
	}
	free(msg_list);
	return 1;
}


int msg_list_push_element(msg_list_t* msg_list, char* msg){

	//TODO TEST IT

	msg_list_t* entry = (msg_list_t*)malloc(sizeof(msg_list_t));
	if(entry == NULL){
		fprintf(stderr,"Error de memoria, no se puede introducir \n");
		return 0;
	}
	entry->msg = msg;
	entry->next = NULL;
	msg_list->next = entry;
	return 1;
}


char* msg_list_pop_element(msg_list_t* msg_list){	//Devolver NULL si la lista está vacía

	//TODO TEST IT

	msg_list_t* actual = msg_list;
	msg_list_t* next = msg_list->next;
	if(next == NULL){
		return NULL;
	}
	while(next != NULL){
		actual = next;
		next = actual->next;
	}
	return actual->msg;
}
