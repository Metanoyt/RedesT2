#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <string.h> 
#include <regex.h> 
#include <stdint.h>

uint8_t* splitter(char* ip){
    char *token = strtok(ip, ".");
	static uint8_t array[4];
    for (int i = 0; i < 4; i++) {
    	array[i] = (uint8_t) atoi(token);
    	// agregar validacion del valor 
		// ip 255.255.255.255
        token = strtok(NULL, ".");
    }
    return array;
}
char* reconstruct(uint8_t* array){
	static char buffer[15];
	snprintf(buffer, sizeof(buffer), "%u.%u.%u.%u", array[0],array[1],array[2],array[3]);
	return buffer;
}

uint8_t* normalizeMask(char* mask){
	static uint8_t array[4];
	if(mask[0]=='/'){
		int value = atoi(strtok(mask, "/"));
		if(value > 32 || value < 8){
			printf("error mascara de bits no valida");
		}
    	unsigned int new_mask = ~(0xFFFFFFFF >> value);
    	array[0] = (new_mask & 0xFF000000)>>24;
    	array[1] = (new_mask & 0x00FF0000)>>16;
    	array[2] = (new_mask & 0x0000FF00)>>8;
    	array[3] = (new_mask & 0x000000FF);
		return array;
	}
    char *token = strtok(mask, ".");
    for (int i = 0; i < 4; i++) {
    	array[i] = (uint8_t) atoi(token);
    	if(array[i]>255){
    		printf("Error mascara de bits no valida");
    	}
        token = strtok(NULL, ".");
    }
    return array;
}

char* broadcast(uint8_t* ip, uint8_t* mask){
	for (int i = 4 ; i-- > 0 ; ){
    	ip[i] = ip[i] | ~mask[i];
	}
	return reconstruct(ip);
}

char* network(uint8_t* ip, uint8_t* mask){
	for (int i = 4 ; i-- > 0 ; ){
    	ip[i] = ip[i] & mask[i];
	}
	return reconstruct(ip);
}

//char* hostrange(){}

//char* randomhostname(){}
char* handleMsg(char* msg){
	regex_t regex;
	static char* response ="Funcionalidad no implementada o error inesperado";
	char* mask;
	char* ip;
	
	regcomp(&regex,"^GET (BROADCAST|NETWORK NUMBER|HOSTS RANGE) IP ([0-9]{1,3}[.]){3}[0-9]{1,3} MASK (([0-9]{1,3}[.]){3}[0-9]{1,3}|/[0-9]{1,2})",REG_EXTENDED);
	if (regexec(&regex, msg, 0, NULL, 0) == 0){
    	char* type;
    	strtok(msg, " ");
    	type = strtok(NULL, " "); strtok(NULL, " ");
    	if(0>strcmp("BROADCAST",type)){ strtok(NULL, " ");}
		ip = strtok(NULL, " "); strtok(NULL," "); 
		mask = strtok(NULL," ");
		//printf("Found ip and mask  %s x %s\n",ip,mask);
    	uint8_t* n_ip = splitter(ip);
		uint8_t* n_mask = normalizeMask(mask);
    	switch(strcmp("HOSTS",type)){
    		case 0:
    			//printf("es host\n");
    			break;
    		case 1:
    			response = broadcast(n_ip,n_mask);
    			break;
    		default:
    			response = network(n_ip,n_mask);
    	}
    	//printf(response);
    	return response;
	}
	regcomp(&regex,"^GET RANDOM SUBNETS NETWORK NUMBER ([0-9]{1,3}[.]){3}[0-9]{1,3} MASK /[0-9]{1,2} NUMBER [0-9]{1,2} SIZE /[0-9]{1,2}",REG_EXTENDED);
	if (regexec(&regex, msg, 0, NULL, 0) == 0){
    	//printf("Pattern 2 found.\n");
    	char* token = strtok(msg, " ");
    	char* number;
    	char* size;
		for (int i = 0; i < 5; i++) {token = strtok(NULL, " ");}
		ip = token; strtok(NULL," "); 
		mask = strtok(NULL," "); strtok(NULL," "); 
		number = strtok(NULL," "); strtok(NULL," "); 
		size = strtok(NULL," ");
		//printf("Found ip mask numb size %s x %s x %s %s",ip,mask,number,size);
    	
    	return response;
	}
    //printf("Formato invalido\n");
	char* no_format ="Funcionalidad no implementada o error inesperado";
	return no_format;
}



int main(int argc, char const *argv[]) {

  int serverFd, clientFd;
  struct sockaddr_in server, client;
  int len;
  int port = 1234;
  char buffer[1024];
  if (argc == 2) {
    port = atoi(argv[1]);
  }
  serverFd = socket(AF_INET, SOCK_STREAM, 0);
  if (serverFd < 0) {
    perror("Cannot create socket");
    exit(1);
  }
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(port);
  len = sizeof(server);
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
	buffer = handleMessage(buffer);

    if (write(clientFd, buffer, size) < 0) {
      perror("write error");
      exit(6);
    }
    close(clientFd);
  }
  close(serverFd);
  return 0;
}
