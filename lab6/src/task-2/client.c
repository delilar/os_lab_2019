#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>
#include <sys/types.h>

struct Server {
    char ip[255];
    int port;
};

// Structure to hold thread arguments
struct ThreadArgs {
    struct Server server;
    uint64_t begin;
    uint64_t end;
    uint64_t mod;
    uint64_t result;
};

uint64_t MultModulo(uint64_t a, uint64_t b, uint64_t mod) {
    a = a % mod;
    uint64_t result = 0;
    while (b > 0) {
        if (b % 2 == 1)
            result = (result + a) % mod;
        a = (a * 2) % mod;
        b /= 2;
    }
    return result;
}

bool ConvertStringToUI64(const char *str, uint64_t *val) {
    char *end = NULL;
    unsigned long long i = strtoull(str, &end, 10);
    if (errno == ERANGE) {
        fprintf(stderr, "Out of uint64_t range: %s\n", str);
        return false;
    }
    if (errno != 0)
        return false;
    *val = i;
    return true;
}

// Function to process server connection in parallel
void* ProcessServerConnection(void* arg) {
    struct ThreadArgs* args = (struct ThreadArgs*)arg;
    struct hostent* hostname = gethostbyname(args->server.ip);
    if (hostname == NULL) {
        fprintf(stderr, "gethostbyname failed with %s\n", args->server.ip);
        pthread_exit(NULL);
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(args->server.port);
    server.sin_addr.s_addr = *((unsigned long*)hostname->h_addr);

    int sck = socket(AF_INET, SOCK_STREAM, 0);
    if (sck < 0) {
        fprintf(stderr, "Socket creation failed!\n");
        pthread_exit(NULL);
    }

    if (connect(sck, (struct sockaddr*)&server, sizeof(server)) < 0) {
        fprintf(stderr, "Connection failed\n");
        close(sck);
        pthread_exit(NULL);
    }

    // Prepare and send task
    char task[sizeof(uint64_t) * 3];
    memcpy(task, &args->begin, sizeof(uint64_t));
    memcpy(task + sizeof(uint64_t), &args->end, sizeof(uint64_t));
    memcpy(task + 2 * sizeof(uint64_t), &args->mod, sizeof(uint64_t));

    if (send(sck, task, sizeof(task), 0) < 0) {
        fprintf(stderr, "Send failed\n");
        close(sck);
        pthread_exit(NULL);
    }

    // Receive result
    char response[sizeof(uint64_t)];
    if (recv(sck, response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Receive failed\n");
        close(sck);
        pthread_exit(NULL);
    }

    memcpy(&args->result, response, sizeof(uint64_t));
    close(sck);
    pthread_exit(NULL);
}

// Function to read servers from file
struct Server* ReadServersFile(const char* filename, unsigned int* servers_num) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Cannot open server file\n");
        return NULL;
    }

    // Count number of servers
    *servers_num = 0;
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        (*servers_num)++;
    }

    // Allocate memory for servers
    struct Server* servers = malloc(sizeof(struct Server) * (*servers_num));
    if (!servers) {
        fclose(file);
        return NULL;
    }

    // Read server information
    rewind(file);
    int i = 0;
    while (fgets(line, sizeof(line), file)) {
        char* ip = strtok(line, ":");
        char* port = strtok(NULL, "\n");
        if (ip && port) {
            strncpy(servers[i].ip, ip, sizeof(servers[i].ip) - 1);
            servers[i].port = atoi(port);
            i++;
        }
    }

    fclose(file);
    return servers;
}

int main(int argc, char** argv) {
    uint64_t k = 0;  // Changed from -1 to 0
    uint64_t mod = 0;  // Changed from -1 to 0
    char servers_file[255] = {'\0'};
    bool k_set = false;
    bool mod_set = false;

    while (true) {
        static struct option options[] = {
            {"k", required_argument, 0, 0},
            {"mod", required_argument, 0, 0},
            {"servers", required_argument, 0, 0},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);

        if (c == -1)
            break;

        switch (c) {
            case 0: {
                switch (option_index) {
                    case 0:
                        if (ConvertStringToUI64(optarg, &k)) k_set = true;
                        break;
                    case 1:
                        if (ConvertStringToUI64(optarg, &mod)) mod_set = true;
                        break;
                    case 2:
                        memcpy(servers_file, optarg, strlen(optarg));
                        break;
                    default:
                        printf("Index %d is out of options\n", option_index);
                }
            } break;
            case '?':
                printf("Arguments error\n");
                break;
            default:
                fprintf(stderr, "getopt returned character code 0%o?\n", c);
        }
    }

    if (!k_set || !mod_set || !strlen(servers_file)) {
        fprintf(stderr, "Using: %s --k 1000 --mod 5 --servers /path/to/file\n", argv[0]);
        return 1;
    }

    // Read servers from file
    unsigned int servers_num = 0;
    struct Server* servers = ReadServersFile(servers_file, &servers_num);
    if (!servers || servers_num == 0) {
        fprintf(stderr, "Failed to read servers from file\n");
        return 1;
    }

    // Create threads for parallel server processing
    pthread_t* threads = malloc(sizeof(pthread_t) * servers_num);
    struct ThreadArgs* thread_args = malloc(sizeof(struct ThreadArgs) * servers_num);

    // Divide work between servers
    uint64_t chunk_size = k / servers_num;
    uint64_t remainder = k % servers_num;
    uint64_t current_begin = 1;

    // Start threads for each server
    for (unsigned int i = 0; i < servers_num; i++) {
        thread_args[i].server = servers[i];
        thread_args[i].begin = current_begin;
        thread_args[i].end = current_begin + chunk_size - 1;
        if (i == servers_num - 1) {
            thread_args[i].end += remainder;
        }
        thread_args[i].mod = mod;
        current_begin = thread_args[i].end + 1;

        if (pthread_create(&threads[i], NULL, ProcessServerConnection, &thread_args[i])) {
            fprintf(stderr, "Failed to create thread\n");
            free(threads);
            free(thread_args);
            free(servers);
            return 1;
        }
    }

    // Wait for all threads and combine results
    uint64_t final_result = 1;
    for (unsigned int i = 0; i < servers_num; i++) {
        pthread_join(threads[i], NULL);
        final_result = MultModulo(final_result, thread_args[i].result, mod);
    }

    printf("Final result: %lu\n", final_result);

    // Cleanup
    free(threads);
    free(thread_args);
    free(servers);

    return 0;
}