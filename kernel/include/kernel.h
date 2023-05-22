#ifndef KERNEL_H_
#define KERNEL_H_

#include <protocolo.h>
#include <sockets.h>
#include <shared.h>
#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>
#include <semaphore.h>


typedef enum
{
	FIFO,
	HRRN,
} t_algoritmo;
//Memoria Robado tp Nahu
typedef struct
{
	int indice_tabla_de_pagina;
	int numero_pagina;
} t_pagina;

typedef struct
{
	char* recurso;
	int id;
	int instancias;
	t_queue* cola_block_asignada;
}t_recurso;

typedef struct
{
	t_registros* registros;
	t_contexto_ejecucion* contexto_de_ejecucion;
	t_segmento* seg_fault;						//Agregado para la MMU, probablemente vaya en el contexto
	uint16_t estimado_proxima_rafaga;
	time_t tiempo_ingreso_ready;
	time_t tiempo_ingreso_exec;
} t_pcb;

typedef struct
{
	t_pcb* pcb;
	int tiempo;
}t_manejo_io;

t_log* logger;
t_log* logger_obligatorio;
t_config* config;
int server_socket;
int fd_cpu;
int fd_memoria;
int fd_filesystem;
char* server_name;

// Variables del config (Las pongo aca asi no estamos revoleando el cfg para todos lados)
char* IP;
char* PUERTO;
char* IP_FILESYSTEM;
char* PUERTO_FILESYSTEM;
char* IP_CPU;
char* PUERTO_CPU;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
t_algoritmo ALGORITMO_PLANIFICACION;
uint16_t ESTIMACION_INICIAL;
double HRRN_ALFA;
int GRADO_MAX_MULTIPROGRAMACION;
char** RECURSOS;
int* INSTANCIAS_RECURSOS;
t_list* lista_recursos;

// Variables PCBs
int generador_pid;
t_list* lista_ready;
t_queue* cola_exit;
t_queue* cola_listos_para_ready;
t_queue* cola_exec;
t_queue* cola_block;

// Semaforos y pthread
pthread_mutex_t mutex_generador_pid;
pthread_mutex_t mutex_cola_ready;
pthread_mutex_t mutex_cola_listos_para_ready;
pthread_mutex_t mutex_cola_exit;
pthread_mutex_t mutex_cola_exec;
pthread_mutex_t mutex_cola_block;
sem_t sem_multiprog;
sem_t sem_listos_ready;
sem_t sem_ready;
sem_t sem_exec;
sem_t sem_exit;

// INIT
void leer_config();
int* string_to_int_array(char** array_de_strings);
void asignar_algoritmo(char* algoritmo);
bool generar_conexiones();
void inicializar_variables();
t_list* inicializar_recursos();

// COMUNICACION
static void procesar_conexion(void* args);
void iterator(char* value);
int server_escuchar(int server_socket);

// PCBS
t_pcb *pcb_create(t_list* instrucciones, int pid);
void pcb_destroy(t_pcb* pcb);
void cambiar_estado(t_pcb *pcb, estado_proceso nuevo_estado);
void procesar_cambio_estado(t_pcb* pcb, estado_proceso estado_nuevo);
void armar_pcb(t_list *instrucciones);
void actualizar_contexto_pcb(t_pcb* pcb, t_contexto_ejecucion* contexto);

// PLANIFICACION
void planificar();
void planificar_largo_plazo();
void exit_pcb();
void ready_pcb();
t_pcb *safe_pcb_pop(t_queue *queue, pthread_mutex_t *mutex);
void safe_pcb_push(t_queue *queue, t_pcb *pcb, pthread_mutex_t *mutex);
void set_pcb_ready(t_pcb* pcb);
void log_cola_ready();
t_list *pcb_to_pid_list(t_list *list);
char* algoritmo_to_string(t_algoritmo algoritmo);
void planificar_corto_plazo();
void exec_pcb();
t_pcb* elegir_pcb_segun_algoritmo();
void dispatch(t_pcb* pcb);
t_pcb* obtener_pcb_HRRN();
bool maximo_HRRN(t_pcb* pcb1, t_pcb* pcb2);
double response_ratio(t_pcb* pcb);
void calcular_estimacion(t_pcb* pcb);
void manejar_io(t_pcb* pcb,  int tiempo);
void exec_io(void* void_arg);
void manejar_wait(t_pcb* pcb, char* recurso);
t_recurso* buscar_recurso(char* recurso);
void manejar_signal(t_pcb* pcb, char* recurso);

#endif /* KERNEL_H_ */
