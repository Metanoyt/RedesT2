#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include <regex.h> 
#include <stdint.h>

// Entradas: String conteniendo un ip en texto,
		//   entero que representa logisticamente la valides de la ip
// Salida: Arreglo de uint8_t conteniendo los valores numericos del ip
uint8_t* splitter(char* ip, int* check){
    char *token = strtok(ip, ".");
	static uint8_t array[4];
	int value;
    for (int i = 0; i < 4; i++) {
    	value = atoi(token);
    	if(value>255){*check=0;}
    	array[i] = (uint8_t) value;
        token = strtok(NULL, ".");
    }
    return array;
}

// Entrada: Arreglo de uint8_t conteniendo los valores numericos de un ip
// Salida: String del ip a partir de los valores numericos ingresados
char* reconstruct(uint8_t* array){
	static char buffer[15];
	snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u\n", array[0],array[1],array[2],array[3]);
	return buffer;
}

// Entrada: String que representa la mascara de bits, ya sea en su forma directa o con su CIDR
// Salida: Arreglo de uint8_t conteniendo los valores numericos de la mascara de bits
uint8_t* normalizeMask(char* mask, int* check){
	static uint8_t array[4];

	int str_len = strlen(mask);
	if (mask[str_len - 1] == '\n') {
			mask[str_len - 1] = '\0';
	}
	str_len = strlen(mask);
	if (mask[str_len - 1] == '\r') {
			mask[str_len - 1] = '\0';
	}
	if(mask[0]=='/'){
		int value = atoi(strtok(mask, "/"));
		if(value > 32 || value < 8){
			*check = 0;
		}
    	unsigned int new_mask = ~(0xFFFFFFFF >> value);
    	array[0] = (new_mask & 0xFF000000)>>24;
    	array[1] = (new_mask & 0x00FF0000)>>16;
    	array[2] = (new_mask & 0x0000FF00)>>8;
    	array[3] = (new_mask & 0x000000FF);
		return array;
	}
    char *token = strtok(mask, ".");
    int value;
    for (int i = 0; i < 4; i++) {
    	value = atoi(token);
    	array[i] = (uint8_t) value;
    	if(value>255){*check = 0;}
        token = strtok(NULL, ".");
    }
    return array;
}

// Entrada: Arreglo de uint8_t conteniendo los valores numericos del ip,
//			Arreglo de uint8_t conteniendo los valores numericos de la mascara de bits
// Salida: String del ip de broadcast calculado a partir de la ip y la mascara de bits ingresados
char* broadcast(uint8_t* ip, uint8_t* mask){
	for (int i = 4 ; i-- > 0 ; ){
    	ip[i] = ip[i] | ~mask[i];
	}
	return reconstruct(ip);
}

// Entrada: Arreglo de uint8_t conteniendo los valores numericos del ip,
//			Arreglo de uint8_t conteniendo los valores numericos de la mascara de bits
// Salida: String del ip de network calculado a partir de la ip y la mascara de bits ingresados
char* network(uint8_t* ip, uint8_t* mask){
	for (int i = 4 ; i-- > 0 ; ){
    	ip[i] = ip[i] & mask[i];
	}
	return reconstruct(ip);
}

// Entrada: Arreglo de uint8_t conteniendo los valores numericos del ip,
//			Arreglo de uint8_t conteniendo los valores numericos de la mascara de bits
// Salida: String del ip de broadcast calculado a partir de la ip y la mascara de bits ingresados
char* hostrange(uint8_t* ip, uint8_t* mask){
	static char buffer[36];
	char byte[9];
	for(int i=0; i < 4; i++){
		if((ip[i] & ~mask[i]) > 0){
			snprintf(byte, 9, "{1-%u}.", ~mask[i] & 254);
		}
		else{
			snprintf(byte, 9, "%u.", ip[i]);
		}
		strcat(buffer,byte);
	}
	return buffer;
}

// Entrada: Socket del cliente encontrado
// Comportamiento: La funcion va a leer el socket recibido,
//				   validara el contenido de ese socket utilizando para saber si la solicitud tiene un formato y valores correctos
// 				   escribe el calculo esperado de la solicitud en el socket del cliente.
const char*  handleMsg(char* msg){
	regex_t regex;
	static char response[1024];
	strcpy(response,"Funcionalidad no implementada\n");
	memset(response, 0, sizeof(response));
	char* mask = (char *) malloc(sizeof(char)*20);
	char* ip = (char *) malloc(sizeof(char)*20);
	int validIp = 1;
	int validMask = 1;
	
	
	regcomp(&regex,"^GET (BROADCAST|NETWORK NUMBER|HOSTS RANGE) IP ([0-9]{1,3}[.]){3}[0-9]{1,3} MASK (([0-9]{1,3}[.]){3}[0-9]{1,3}|/[0-9]{1,2})",REG_EXTENDED);
	if (regexec(&regex, msg, 0, NULL, 0) == 0){
    	char* type;
    	strtok(msg, " ");
    	type = strtok(NULL, " "); strtok(NULL, " ");
    	if(0>strcmp("BROADCAST",type)){ strtok(NULL, " ");}
		ip = strtok(NULL, " "); strtok(NULL," "); 
		mask = strtok(NULL," ");
		//printf("Found ip and mask  %s x %s\n",ip,mask);
    	uint8_t* n_ip = splitter(ip, &validIp);
    	if(validIp == 0){strcpy(response,"ip invalida\n");return response;}
		uint8_t* n_mask = normalizeMask(mask, &validMask);
		if(validMask == 0){strcpy(response, "mascara invalida\n");return response;}
    	switch(strcmp("HOSTS",type)){
    		case 0:
				strcpy(response, hostrange(n_ip,n_mask));
    			break;
    		case 1:
				strcpy(response,broadcast(n_ip,n_mask));
    			break;
    		default:
    			strcpy(response, network(n_ip,n_mask));
    	}
    	return response;
	}
	regcomp(&regex,"^GET RANDOM SUBNETS NETWORK NUMBER ([0-9]{1,3}[.]){3}[0-9]{1,3} MASK /[0-9]{1,2} NUMBER [0-9]{1,2} SIZE /[0-9]{1,2}",REG_EXTENDED);
	if (regexec(&regex, msg, 0, NULL, 0) == 0){
    	char* token = strtok(msg, " ");
    	char* number;
    	char* size;
		for (int i = 0; i < 5; i++) {token = strtok(NULL, " ");}
		ip = token; strtok(NULL," "); 
		mask = strtok(NULL," "); strtok(NULL," "); 
		number = strtok(NULL," "); strtok(NULL," "); 
		size = strtok(NULL," ");
    	
    	return response;
	}
	strcpy(response,"Formato invalido\n");
    return response;
}



int main(int argc, char const *argv[]) {

  int serverFd, clientFd;
  struct sockaddr_in server, client;
  int len;
  int port = 9666;
  char buffer[1024];
  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  len = sizeof(server);
  
  int activado = 1;
  setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));
  if (bind(serverFd, (struct sockaddr *)&server, len) < 0) {
    perror("Cannot bind sokcet");
    exit(2);
  }
  if (listen(serverFd, 10) < 0) {
    perror("Listen error");
    exit(3);
  }
  while (1) {
    len = sizeof(client);
    printf("waiting for clients\n");
    if ((clientFd = accept(serverFd, (struct sockaddr *)&client, &len)) < 0) {
      perror("accept error");
      exit(4);
    }
    char *client_ip = inet_ntoa(client.sin_addr);
    printf("Accepted new connection from a client %s:%d\n", client_ip, ntohs(client.sin_port));
    memset(buffer, 0, sizeof(buffer));
    int size = read(clientFd, buffer, sizeof(buffer));
    if ( size < 0 ) {
      perror("read error");
      exit(5);
    }
    printf("received %s from client\n", buffer);
    if (write(clientFd, handleMsg(buffer), size) < 0) {
      perror("write error");
      exit(6);
    }
	memset(buffer, 0, sizeof(buffer));
    close(clientFd);
  }
  close(serverFd);
  return 0;
}
