#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "hash.h"

#define SERVER_PORT 8888
#define MAX_WORKERS 10
#define BUFFER_SIZE 1024
#define MAX_TEXT 256

typedef struct {
    int sock;
    uint64_t range_start;
    uint64_t range_end;
    int id;
    int active;
} WorkerInfo;

WorkerInfo g_workers[MAX_WORKERS];
int g_total_workers = 0;
int g_solution_found = 0;
char g_solution_hash[BUFFER_SIZE];
uint64_t g_solution_nonce = 0;
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

void read_text_file(char* out);
void configure_parameters(char* pad_text, int* zero_count, int* worker_count);
void partition_work(int pad_len, int worker_count);
void* handle_worker(void* param);
void notify_all_workers(const char* msg);

int main() {
    int server_sock, worker_sock;
    struct sockaddr_in addr_server, addr_client;
    socklen_t client_len;
    char original_text[MAX_TEXT];
    char pad_text[MAX_TEXT];
    int zero_count, worker_count;
    int optval = 1;

    printf("=== SIMULADOR DE BLOCKCHAIN - SERVIDOR ===\n\n");

    read_text_file(original_text);
    printf("Texto original del archivo: %s\n\n", original_text);

    configure_parameters(pad_text, &zero_count, &worker_count);
    g_total_workers = worker_count;

    printf("\n=== CONFIGURACION ===\n");
    printf("Texto de relleno: %s (longitud: %d)\n", pad_text, (int)strlen(pad_text));
    printf("Condicion: Hash terminado en %d ceros\n", zero_count);
    printf("Numero de workers: %d\n\n", worker_count);

    partition_work(strlen(pad_text), worker_count);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error al crear socket");
        return 1;
    }

    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval))) {
        perror("Error en setsockopt");
        return 1;
    }

    printf("Socket creado.\n");

    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = INADDR_ANY;
    addr_server.sin_port = htons(SERVER_PORT);

    if (bind(server_sock, (struct sockaddr*)&addr_server, sizeof(addr_server)) < 0) {
        perror("Error en bind");
        return 1;
    }

    printf("Bind realizado.\n");

    if (listen(server_sock, MAX_WORKERS) < 0) {
        perror("Error en listen");
        return 1;
    }

    printf("Esperando conexiones en el puerto %d...\n\n", SERVER_PORT);

    client_len = sizeof(struct sockaddr_in);

    for (int i = 0; i < worker_count && !g_solution_found; i++) {
        worker_sock = accept(server_sock, (struct sockaddr*)&addr_client, &client_len);

        if (worker_sock < 0) {
            perror("Error al aceptar conexion");
            continue;
        }

        printf("Worker %d conectado desde %s:%d\n",
               i + 1,
               inet_ntoa(addr_client.sin_addr),
               ntohs(addr_client.sin_port));

        g_workers[i].sock = worker_sock;
        g_workers[i].id = i;
        g_workers[i].active = 1;

        char msg[BUFFER_SIZE];
        sprintf(msg, "CONFIG|%s|%s|%d|%llu|%llu",
                original_text,
                pad_text,
                zero_count,
                g_workers[i].range_start,
                g_workers[i].range_end);
        send(worker_sock, msg, strlen(msg), 0);

        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handle_worker, &g_workers[i]);
        pthread_detach(thread_id);
    }

    printf("\nTodos los workers conectados. Iniciando busqueda...\n\n");

    while (1) {
        sleep(1);

        int active_count = 0;
        for (int i = 0; i < worker_count; i++) {
            if (g_workers[i].active) {
                active_count++;
            }
        }

        if (active_count == 0 || g_solution_found) {
            break;
        }
    }

    printf("\n=== RESULTADO FINAL ===\n");
    if (g_solution_found) {
        printf("SOLUCION ENCONTRADA!\n");
        printf("Hash: %s\n", g_solution_hash);
        printf("Nonce: %llu\n", g_solution_nonce);
    } else {
        printf("No se encontro solucion en el espacio de busqueda.\n");
    }

    for (int i = 0; i < worker_count; i++) {
        close(g_workers[i].sock);
    }
    close(server_sock);
    pthread_mutex_destroy(&g_mutex);

    return 0;
}

void read_text_file(char* out) {
    FILE* f = fopen("Archivo.txt", "r");
    if (f == NULL) {
        printf("Error: No se pudo abrir Archivo.txt\n");
        strcpy(out, "Texto por defecto");
        return;
    }

    if (fgets(out, MAX_TEXT, f) == NULL) {
        strcpy(out, "Texto por defecto");
    } else {
        size_t len = strlen(out);
        if (len > 0 && out[len - 1] == '\n') {
            out[len - 1] = '\0';
        }
    }

    fclose(f);
}

void configure_parameters(char* pad_text, int* zero_count, int* worker_count) {
    printf("Ingrese el texto de relleno (caracteres alfanumericos): ");
    scanf("%s", pad_text);

    printf("Ingrese la cantidad de ceros consecutivos al final del hash: ");
    scanf("%d", zero_count);

    printf("Ingrese el numero de workers (1-%d): ", MAX_WORKERS);
    scanf("%d", worker_count);

    if (*worker_count < 1) *worker_count = 1;
    if (*worker_count > MAX_WORKERS) *worker_count = MAX_WORKERS;
}

void partition_work(int pad_len, int worker_count) {
    uint64_t total = count_total_variants(pad_len);
    uint64_t per_worker = total / worker_count;

    printf("Total de combinaciones: %llu\n", total);
    printf("Combinaciones por worker: %llu\n\n", per_worker);

    for (int i = 0; i < worker_count; i++) {
        g_workers[i].range_start = i * per_worker;
        g_workers[i].range_end = (i == worker_count - 1)
                                 ? total - 1
                                 : (i + 1) * per_worker - 1;

        printf("Worker %d: rango [%llu - %llu]\n",
               i + 1,
               g_workers[i].range_start,
               g_workers[i].range_end);
    }
}

void* handle_worker(void* param) {
    WorkerInfo* w = (WorkerInfo*)param;
    char buf[BUFFER_SIZE];
    int nread;

    while ((nread = recv(w->sock, buf, BUFFER_SIZE - 1, 0)) > 0) {
        buf[nread] = '\0';

        if (strncmp(buf, "SOLUCION", 8) == 0) {
            char* tok = strtok(buf, "|");
            tok = strtok(NULL, "|");
            strcpy(g_solution_hash, tok);
            tok = strtok(NULL, "|");
            g_solution_nonce = strtoull(tok, NULL, 10);

            pthread_mutex_lock(&g_mutex);
            if (!g_solution_found) {
                g_solution_found = 1;
                printf("\n*** Worker %d encontro la solucion! ***\n", w->id + 1);
                printf("Hash: %s\n", g_solution_hash);
                printf("Nonce: %llu\n", g_solution_nonce);
                notify_all_workers("DETENER");
            }
            pthread_mutex_unlock(&g_mutex);
            break;
        } else if (strncmp(buf, "NO_ENCONTRADO", 13) == 0) {
            printf("Worker %d termino sin encontrar solucion en su rango.\n", w->id + 1);
            break;
        } else if (strncmp(buf, "PROGRESO", 8) == 0) {
            printf("Worker %d: %s\n", w->id + 1, buf);
        }
    }

    w->active = 0;
    return NULL;
}

void notify_all_workers(const char* msg) {
    for (int i = 0; i < g_total_workers; i++) {
        if (g_workers[i].active && g_workers[i].sock > 0) {
            send(g_workers[i].sock, msg, strlen(msg), 0);
        }
    }
}
