#include "../include/fileSystem.h"

int main(int argc, char **argv) {
	if (argc > 2) {
		return EXIT_FAILURE;
	}
	logger = log_create("filesystem.log", "filesystem_main", 1, LOG_LEVEL_INFO);
	config = config_create(argv[1]);
	if(config == NULL){
		log_error(logger, "No se encontró el archivo :(");
		exit(1);
	}
	leer_config();

	levantar_archivos();
	//inicializar_variables();

	// Conecto CPU con memoria
	fd_memoria = crear_conexion(IP_MEMORIA, PUERTO_MEMORIA);
	enviar_mensaje("Hola, soy Filesystem.", fd_memoria);

	// Inicio de servidor
	fd_filesystem = iniciar_servidor(logger, IP, PUERTO);

	// Conexion Kernel
	pthread_t conexion_memoria;
	pthread_create(&conexion_memoria, NULL, (void*) server_escuchar, NULL);
	pthread_join(conexion_memoria, NULL);

	terminar_programa(logger, config);
	return 0;
}

void leer_config(){
	IP = config_get_string_value(config, "IP_ESCUCHA");
	PUERTO = config_get_string_value(config, "PUERTO_ESCUCHA");
	IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
	PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
	PATH_SUPERBLOQUE = config_get_string_value(config, "PATH_SUPERBLOQUE");
	PATH_BITMAP = config_get_string_value(config, "PATH_BITMAP");
	PATH_BLOQUES = config_get_string_value(config, "PATH_BLOQUES");
}

// ------------------ INIT --------------------------

void inicializar_variables(){
	//ARRAY_BLOQUES[TAMANIO];
	//ARRAY_BITMAP[BLOCK_COUNT];
}

void leer_superbloque(){

	superbloque = config_create(PATH_SUPERBLOQUE);

	if(superbloque == NULL){
		log_error(logger, "No se encontró el archivo superbloque >:(");
		exit(1);
	}

	BLOCK_SIZE = config_get_int_value(superbloque, "BLOCK_SIZE");
	BLOCK_COUNT = config_get_int_value(superbloque, "BLOCK_COUNT");
}

void crear_bitmap(){

	int fd;
	int tamanio_bitmap = ceil(BLOCK_COUNT/8);

	if(access(PATH_BITMAP, F_OK) != -1){
		fd = open(PATH_BITMAP, O_RDWR);
		void* buffer = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		bitmap = bitarray_create_with_mode(buffer, tamanio_bitmap, LSB_FIRST);
	}
	else{
		// El archivo no existe entonces lo creo y lo cierro, puede que esto se borre y sea innecesario
		fd = open(PATH_BITMAP, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

		if(fd == -1){
			log_error(logger, "Hubo un problema creando el archivo de bitmap >:(");
			exit(1);
		}

		log_info(logger, "fd: %d", fd);

		char bitarray[tamanio_bitmap];
		//lleno de 0s el bitarray
		memset(bitarray, 0, tamanio_bitmap);

		// Mapea el archivo en memoria
		void* buffer = mmap(NULL, tamanio_bitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

		if(buffer == MAP_FAILED){
			log_error(logger, "Error ejecutando mmap.");
			close(fd);
			return;
		}

		bitmap = bitarray_create_with_mode(buffer, tamanio_bitmap, LSB_FIRST);

		msync(bitmap->bitarray, tamanio_bitmap, MS_SYNC);

		write(fd, bitmap->bitarray, sizeof(bitarray));
	}
	// Esta parte puede que ni vaya por el momento
    // Copia los bits del bitarray al mapeo en memoria
//    memcpy(mmap_funciono, bitmap->bitarray, tamanio_bitmap);
//	//free(bitarray_p);
//
//    // Liberar recursos
//    munmap(mmap_funciono, tamanio_bitmap);
//    bitarray_destroy(bitmap);
    close(fd);

}

void crear_archivo_de_bloques(){

	//Primero deberia leer el archivo y en caso de que no este creado crearlo
	int fd = open(PATH_BLOQUES, O_CREAT | O_RDWR);

	if(fd == -1){
		log_error(logger, "Hubo un problema creando o abriendo el archivo de bloques >:(");
		exit(1);
	}

	//X AHORA QUEDA HASTA ACA CHECKPOINT 3

	/*

	//Ver que onda despues con esto
	TAMANIO = BLOCK_SIZE * BLOCK_COUNT;
	ARRAY_BLOQUES= malloc(TAMANIO);

	for(int i = 0; i < TAMANIO; i++){
		//ARRAY_BLOQUES[i] = malloc();
		strcpy(ARRAY_BLOQUES[i], "");
	}
	*/
}


void levantar_archivos(){
	leer_superbloque();
	crear_bitmap();
	crear_archivo_de_bloques();
}

void terminar_programa(){
	log_destroy(logger);
	config_destroy(config);
}

// ------------------ COMUNICACION ------------------

static void procesar_conexion() {
	op_code cop;
	char* nombre_archivo;

	while (socket_cliente != -1) {
        if (recv(socket_cliente, &cop, sizeof(op_code), 0) != sizeof(op_code)) {
            log_info(logger, "El cliente se desconecto de %s server", server_name);
            return;
        }
		switch (cop) {
		case MENSAJE:
			recibir_mensaje(logger, socket_cliente);
			break;
		case MANEJAR_F_OPEN:
			nombre_archivo = recv_nombre_archivo(socket_cliente);
			log_info(logger,"Se esta ejecutando un MANEJAR_F_OPEN");
			manejar_f_open(nombre_archivo);
			break;
		case MANEJAR_F_CREATE:
			nombre_archivo = recv_nombre_archivo(socket_cliente);
			log_info(logger,"Se esta ejecutando un MANEJAR_F_CREATE");
			manejar_f_create(nombre_archivo);
			break;
		case MANEJAR_F_TRUNCATE:
			break;
		case MANEJAR_F_READ:
			break;
		case MANEJAR_F_WRITE:
			break;
		default:
			log_error(logger, "Algo anduvo mal en el server de %s", server_name);
			return;
		}
	}

	log_warning(logger, "El cliente se desconecto de %s server", server_name);
	return;
}

void server_escuchar() {
	server_name = "Filesystem";
	socket_cliente = esperar_cliente(logger, server_name, fd_filesystem);

	if (socket_cliente == -1) {
		log_info(logger, "Hubo un error en la conexion del %s", server_name);
	}
	procesar_conexion();
}

bool existe_fcb(char* nombre_archivo){
	//TODO Hacer esta funcion
	return true;
}

void manejar_f_open(char* nombre_archivo){

	if(existe_fcb(nombre_archivo)){
		send_confirmacion_archivo_abierto(socket_cliente);
	} else{
		send_aviso_archivo_inexistente(socket_cliente);
	}
}

void manejar_f_create(char* nombre_archivo){

	fcb *nuevo_fcb = malloc(sizeof(fcb));
	//strcpy(nuevo_fcb->nombre_archivo, nombre_archivo);	//TODO Tira un warning, dice que no se inicializo
	nuevo_fcb->tamanio_archivo = 0;
	nuevo_fcb->puntero_directo = 0;
	nuevo_fcb->puntero_indirecto = 0;
	send_confirmacion_archivo_creado(socket_cliente);
}
