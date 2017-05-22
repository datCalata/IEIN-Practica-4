/*
 * Copyright (c) 2017, Universidad Politecnica de Madrid - B105 Electronic Systems Lab
 * All rights reserved.

 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    This product includes software developed by the B105 Electronic Systems Lab.
 * 4. Neither the name of the B105 Electronic Systems Lab nor the
 *    names of its contributors may be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY UNIVERSITY AND CONTRIBUTORS ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * dron_control.c
 *
 *  Created on: 24 de abr. de 2017
 *      Author: Roberto Rodriguez-Zurrunero  <r.rodriguezz@b105.upm.es>
 *
 */
/**
 * @file dron_control.c
 */

#include "dron_control.h"

#define WRITE_FILENAME		"write_msg_file.txt"
#define READ_FILENAME		"read_msg_file.txt"
#define MSG_FORMAT			"%s\r\n"
#define MSG_MAX_SIZE		16

//Conversor String int

///////FSM TABLE FUNCTIONS//////
//CONDITION FUNCTIONS
//TODO Declaración de funciones INPUT de la máquina de estados. Condiciones para entrar en un estado
static int input_condition_func_stop(fsm_t* fsm);
static int input_condition_func_rising_1(fsm_t* fsm);
static int input_condition_func_rising_2(fsm_t* fsm);
static int input_condition_func_stable_1(fsm_t* fsm);
static int input_condition_func_stable_2(fsm_t* fsm);
static int input_condition_func_descending_1(fsm_t* fsm);
static int input_condition_func_descending_2(fsm_t* fsm);


//OUTPUT FUNCTIONS
//TODO Declaración de funciones OUTPUT de la máquina de estados. Se ejecutan cuando se alcanza el estado.
static void output_func_stop(fsm_t* fsm);
static void output_func_rising_1(fsm_t* fsm);
static void output_func_rising_2(fsm_t* fsm);
static void output_func_stable_1(fsm_t* fsm);
static void output_func_stable_2(fsm_t* fsm);
static void output_func_descending_1(fsm_t* fsm);
static void output_func_descending_2(fsm_t* fsm);

// Máquina de estados: lista de transiciones
// { EstadoOrigen, CondicionDeDisparo, EstadoFinal, AccionesSiTransicion }
fsm_trans_t dron_transition_table[] = {
		{STOPPED, input_condition_func_stop,RISING,output_func_stop},
		{RISING,input_condition_func_rising_1,STABLE,output_func_rising_1},
		{RISING,input_condition_func_rising_2,DESCENDING,output_func_rising_2},
		{STABLE,input_condition_func_stable_1,DESCENDING,output_func_stable_1},
		{STABLE,input_condition_func_stable_2,RISING,output_func_stable_2},
		{DESCENDING,input_condition_func_descending_1,STOPPED,output_func_descending_1},
		{DESCENDING,input_condition_func_descending_2,STABLE,output_func_descending_2},
		{-1, NULL, -1, NULL },
};


/////Dron FSM object functions//////
dron_fsm_t* new_dron_fsm(fsm_trans_t* dron_transition_table){
	dron_fsm_t* new_dron_fsm = (dron_fsm_t*) malloc(sizeof(dron_fsm_t));
	//TODO Inicializar...
	new_dron_fsm->fsm.tt = dron_transition_table;
	new_dron_fsm->fsm.current_state = STOPPED;
	new_dron_fsm->started_flag = 0;
	new_dron_fsm->stopping_flag = 0;
	new_dron_fsm->current_height = 0;
	new_dron_fsm->msg_list = msg_list_init();
	return new_dron_fsm;
}

int delete_dron_fsm(dron_fsm_t* dron_fsm){
	//TODO Borrar la lista de mensajes y el objeto dron_fsm
	msg_list_delete(dron_fsm->msg_list);
	free(dron_fsm);
	return 1;
}


int read_msg(dron_fsm_t* dron_fsm){
	//TODO Leer el fichero de entrada y guardar todos los mensajes que se encuentren en la lista de mensajes.
	//	   Eliminar el fichero una vez leído por completo. Si no se puede abrir el fichero o no existe devolver -1

	FILE *p_fichero;
	char* linea;


	p_fichero = fopen(READ_FILENAME,"r");
	if(p_fichero == NULL){
		perror("Error de Apertura de fichero");
		return -1;
	}


	while(!feof(p_fichero)){
		linea = (char*)malloc(MSG_MAX_SIZE*sizeof(char));
		fscanf(p_fichero,MSG_FORMAT,linea);
		if(msg_list_push_element(dron_fsm->msg_list,linea) == 0){
			perror("Error introduciendo elemento en la lista");
			return -1;
		}
	}

	remove(READ_FILENAME);
	fclose(p_fichero);
	return 1;
}


int parse_and_update(dron_fsm_t* dron_fsm){
	//TODO Leer el mensaje más antiguo almacenado en la lista FIFO y parsearlo.
	//Según el tipo de mensaje que sea actualizar los flags internos del objeto dron_fsm.
	//Devolver -1 si no hay mensajes en la lista.
	char* orden = NULL;
	int altura;

	orden = msg_list_pop_element(dron_fsm->msg_list);
	if(orden == NULL){
		perror("Error al recibir el mensaje, no hay mensajes en la lista?");
		return -1;
	}

	if(strcmp(orden,"STOP") == 0){
		dron_fsm->stopping_flag = 1;
		dron_fsm->started_flag = 0;
		free(orden);
		return 1;
	}
	if(strcmp(orden,"START") == 0){
		dron_fsm->stopping_flag = 0;
		dron_fsm->started_flag = 1;
		free(orden);
		return 1;
	}

	altura = atoi(orden+1);
	dron_fsm->current_height += altura;
	//Si la altura fuese negativa la dejamos en 0
	if(dron_fsm->current_height < 0){
		dron_fsm->current_height = 0;
	}
	free(orden);
	return 1;
}



///////FSM TABLE FUNCTIONS//////
//CONDITION FUNCTIONS
//TODO Implementación de funciones INPUT de la máquina de estados. Condiciones para entrar en un estado
static int input_condition_func_stop(fsm_t* fsm){
	dron_fsm_t* dron = (dron_fsm_t*)fsm;
	if(dron->started_flag == 1 && dron->stopping_flag == 0){
		return 1;
	}
	return 0;
}

static int input_condition_func_rising_1(fsm_t* fsm){
	dron_fsm_t* dron = (dron_fsm_t*)fsm;
	if(dron->current_height >= STABLE_HEIGHT){
		return 1;
	}
	return 0;
}

static int input_condition_func_rising_2(fsm_t* fsm){
	dron_fsm_t* dron = (dron_fsm_t*)fsm;
	if(dron->started_flag == 0 && dron->stopping_flag == 1){
		dron->started_flag = 1;
		return 1;
	}
	return 0;
}

static int input_condition_func_stable_1(fsm_t* fsm){
	dron_fsm_t* dron = (dron_fsm_t*)fsm;
	if(dron->current_height > HEIGHT_LIMIT_HIGH || dron->stopping_flag == 1){
		return 1;
	}
	return 0;
}

static int input_condition_func_stable_2(fsm_t* fsm){
	dron_fsm_t* dron = (dron_fsm_t*)fsm;
	if(dron->current_height < HEIGHT_LIMIT_LOW){
		return 1;
	}
	return 0;
}

static int input_condition_func_descending_1(fsm_t* fsm){
	dron_fsm_t* dron = (dron_fsm_t*)fsm;
	if(dron->current_height == ZERO_HEIGHT && dron->stopping_flag == 1){
		dron->fsm.current_state = 0;
		return 1;
	}
	return 0;
}
static int input_condition_func_descending_2(fsm_t* fsm){
	dron_fsm_t* dron = (dron_fsm_t*)fsm;
	if(dron->current_height <= STABLE_HEIGHT && dron->stopping_flag != 1 ){
		return 1;
	}
	return 0;
}


//OUTPUT FUNCTIONS
//TODO Implementación de funciones OUTPUT de la máquina de estados. Se ejecutan cuando se alcanza el estado. Se escribe en el fichero de salida el estado que se alcanza.
static void output_func_stop(fsm_t* fsm){
	FILE *p_fichero;
	if((p_fichero = fopen(WRITE_FILENAME,"a")) == NULL){
		perror("Error al abrir el fichero de salida");
	}
	fprintf(p_fichero,"STOPPED->RISING \n");
	fclose(p_fichero);
}

static void output_func_rising_1(fsm_t* fsm){
	FILE *p_fichero;
	if((p_fichero = fopen(WRITE_FILENAME,"a")) == NULL){
			perror("Error al abrir el fichero de salida");
	}
	fprintf(p_fichero,MSG_FORMAT,"RISING->STABLE");
	fclose(p_fichero);
}

static void output_func_rising_2(fsm_t* fsm){
	FILE *p_fichero;
	if((p_fichero = fopen(WRITE_FILENAME,"a")) == NULL){
			perror("Error al abrir el fichero de salida");
		}
	fprintf(p_fichero,MSG_FORMAT,"RISING->DESCENDING");
	fclose(p_fichero);
}

static void output_func_stable_1(fsm_t* fsm){
	FILE *p_fichero;
	if((p_fichero = fopen(WRITE_FILENAME,"a")) == NULL){
			perror("Error al abrir el fichero de salida");
		}
	fprintf(p_fichero,MSG_FORMAT,"STABLE->DESCENDING");
	fclose(p_fichero);
}

static void output_func_stable_2(fsm_t* fsm){
	FILE *p_fichero;
	if((p_fichero = fopen(WRITE_FILENAME,"a")) == NULL){
			perror("Error al abrir el fichero de salida");
		}
	fprintf(p_fichero,MSG_FORMAT,"STABLE->RISING");
	fclose(p_fichero);
}

static void output_func_descending_1(fsm_t* fsm){
	FILE *p_fichero;
	if((p_fichero = fopen(WRITE_FILENAME,"a")) == NULL){
			perror("Error al abrir el fichero de salida");
		}
	fprintf(p_fichero,MSG_FORMAT,"DESCENDING->STOPPED");
	fclose(p_fichero);
}

static void output_func_descending_2(fsm_t* fsm){
	FILE *p_fichero;
	if((p_fichero = fopen(WRITE_FILENAME,"a")) == NULL){
			perror("Error al abrir el fichero de salida");
		}
	fprintf(p_fichero,MSG_FORMAT,"DESCENDING->STABLE");
	fclose(p_fichero);
}
