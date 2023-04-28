#ifndef COMUNICACION_H_
#define COMUNICACION_H_

#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <inttypes.h>
#include <commons/log.h>
#include <protocolo.h>
#include <sockets.h>

typedef struct {
	t_log *log;
	int fd;
	char *server_name;
} t_procesar_conexion_args;


int server_escuchar(t_log* logger, int server_socket);


#endif /* COMUNICACION_H_ */