/*
 * utils_servidor.h
 *
 *  Created on: Apr 10, 2023
 *      Author: utnso
 */

#ifndef UTILS_SERVIDOR_H_
#define UTILS_SERVIDOR_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<string.h>
#include<assert.h>

#define IP "127.0.0.1"
#define PUERTO "35351"

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(void);
int esperar_cliente(int);
t_list* recibir_paquete(int);
void recibir_mensaje(int);
int recibir_operacion(int);


#endif /* UTILS_SERVIDOR_H_ */
