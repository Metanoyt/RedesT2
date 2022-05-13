#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <wait.h>
#include <fcntl.h> 
#include <sys/types.h>

//Definicion de las estructuras
typedef struct {
    int *array;
    size_t used;
    size_t size;
} Array;

struct node{
    struct node *next;
    int *socketCliente; 
}typedef Node;

//Declaracion de las globales
//----- HTTP  mensajes de response
#define MAX_PATH 4096 //max cantidad de caraceteres en el path
#define BACKLOG_SIZE 500 //cantidad de conexiones que va a poder almazenar 
#define PORT 6995

pthread_t *thread_pool;
pid_t *pid;
pthread_mutex_t lock_exit = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lockCola = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condPool = PTHREAD_COND_INITIALIZER;
typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

Node *inicio;
Node *fin;


char *strreplace(char *s, const char *s1, const char *s2) {
    char *p = strstr(s, s1);
    if (p != NULL) {
        size_t len1 = strlen(s1);
        size_t len2 = strlen(s2);
        if (len1 != len2)
            memmove(p + len2, p + len1, strlen(p + len1) + 1);
        memcpy(p, s2, len2);
    }
    return s;
}

int* pop(){
    if (inicio == NULL) {
        return NULL;
    } else {
        int *result = inicio->socketCliente;
        Node *temp = inicio;
        inicio = temp->next; // cambio el inicio por el siguiente en la fila
        if (inicio == NULL) {
            fin = NULL;
        }
        free(temp);
        return result;
    }
}

void add(int *nuevo_socket){
    Node *nuevoNodo = malloc(sizeof(Node));
    nuevoNodo->socketCliente = nuevo_socket;
    nuevoNodo->next = NULL;
    if (fin == NULL) {
        inicio = nuevoNodo;
    } else {
        fin->next = nuevoNodo;
    }
    fin = nuevoNodo;

}


Request *parseMessage(char *buffer){
    
}

void* stdinReader(void *args){
    pthread_mutex_lock(&lock_exit);
    while(1){
        /**
         * Setea variables que voy a usar como el buffer y el File Identifier(FD)
         */
        pthread_mutex_unlock(&lock_exit);
        fd_set rfds;
        struct timeval tv;
        int retval, len;
        char buf[4096];

        /**
         * Seteo el File Identifier numero 0 que es la entrada por consola, ademas de poner un tiempo de 5s para leer
         */
        FD_ZERO(&rfds);
        FD_SET(0, &rfds);
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        /**
         * Selecciono el buffer de consola previamente obtenido para leer
         */
        retval = select(1, &rfds, NULL, NULL, &tv);
        /* Don't rely on the value of tv now! */
        /**
         * Verificacion de errores
         */
        if (retval < 0) {
            perror("select()");
            exit(-1);
        }
        if (FD_ISSET(0, &rfds)) {
            len = read(0, buf, 4096);
            if (len > 0) {
                buf[len] = 0;

                if(strcmp(buf,"exit\n") == 0){
                    exit(0);
                }
            } else {
                perror("read()");
                exit(-1);
            }
        }

        pthread_mutex_lock(&lock_exit);
    }
}

void * handleMessage(int* p_client_socket){
    int client_socket = *p_client_socket;
    free(p_client_socket);
    char buffer[BUFSIZ];
    memset(buffer,0,strlen(buffer));
    size_t bytes_read;
    int msgsize = 0;
    char actualpath[MAX_PATH+1];
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(client_socket,&fds);
    char numberString[4096];

    //esperamos 1 segundo si aun no hay nada en el socket de lectura
    //while(select(FD_SETSIZE,&fds,NULL,NULL,NULL)==0){
        //sleep(1);
    //}
    msgsize=0;
    //lee al cliente y obtiene el nombre del archivo
    sleep(1);
    while((bytes_read = read(client_socket, buffer+msgsize, sizeof(buffer)-msgsize-1)) > 0 ) {
        msgsize += bytes_read;
        if (msgsize > BUFSIZ-1 || buffer[msgsize-1] == '\n') break;
    }
    if(bytes_read==-1){
        printf("error con la info recibida");
        exit(1);
    }
    buffer[msgsize-1] = 0; //se termina el msg en null y se remueve el \n
    printf("Buffer:[%s]\n",buffer);

    Request *r = parseMessage(buffer);


    if(r==NULL){
        close(client_socket);
        return NULL;
    }
    //toString(r);
    fflush(stdout);

    Response *response;
    //Veo en que metodo cae
    if (!strcmp(r->method,"GET")){
        response = executeGet(r,buffer,&client_socket);
        if(response == NULL){
            close(client_socket);
            return NULL;
        }
    }else if (!strcmp(r->method,"POST")){
        close(client_socket);
        return NULL;
    }else{
        /**
         * TODO Not Implemented response
         */
        close(client_socket);
        return NULL;
    }
    while(response->size > 0){
        writeResponse(response->type,response,&client_socket);
        response->size = 0;
        //printf("write :  %ld\n", n);
    }
    //writeResponse(response->type,response,&client_socket);

    freeRequest(r);
    freeResponse(response);
    close(client_socket);
    printf("cerrando conexion\n");
}

int main(int argc, char **argv){
    int server;
    unsigned int addr_size;
    SA_IN direccionServer, client_addr;

    pthread_t hilo_consola;
    pthread_create(&hilo_consola, NULL, stdinReader,NULL);

    //Limpiemoas la estructura
    memset(&direccionServer, 0, sizeof(direccionServer));

    //Se establece el objeto socket
    server = socket(AF_INET , SOCK_STREAM , 0);

    //AF_INET es una familia de direcciones
    direccionServer.sin_family = AF_INET;
    //INADDR_ANY = en realidad es la IP especial 0.0.0.0
    direccionServer.sin_addr.s_addr = INADDR_ANY;
    //Este es el puerto del servidor
    direccionServer.sin_port = htons(6995);

    //Esto es para no esperar despues de matar el servidor
    int activado = 1;
    setsockopt(server,SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

    if (bind(server,(SA*)&direccionServer, sizeof(direccionServer)) == -1){
        perror("Falló el bind del socket");
        return 1;
    }
    
    if (listen(server,BACKLOG_SIZE) == -1) {
        perror("Falló el listen del socket");
        return 1;
    }
    
    pthread_mutex_lock(&lock_exit);
    while (1) {
        int client_socket;
        printf("Esperando conexiones\n");
        //esperando clientes
        addr_size = sizeof(SA_IN);
        client_socket = accept(server, (void*)&client_addr, &addr_size);
        printf("Nuevo cliente\n");

        int *pclient = malloc(sizeof(int));
        *pclient = client_socket;

        pthread_t thread;
        pthread_create(&thread, NULL, (void *) handleMessage, pclient);
    }

    return 0;
}
