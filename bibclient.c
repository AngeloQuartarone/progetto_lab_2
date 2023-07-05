#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include "./lib/linkedList.h"
#include "./lib/util.h"

#define MAX_FIELDS 64
#define BUF_SIZE 1024
#define IP "127.0.0.1"
#define PORT 0
#define MSG_QUERY 'Q'
#define MSG_LOAN 'L'
#define MSG_RECORD 'R'
#define MSG_NO 'N'
#define MSG_ERROR 'E'
#define MSG_END 'X'

static int prestito = 0;
static int *en = 0;
static void *ptr;
static pthread_mutex_t *mtx_log;

typedef struct
{
    char *etichetta;
    char *valore;
} variable_for_request;

void toUpperCase(char *);
linked_list_t *arguments_parser(char *[], int);
void printList(linked_list_t *);
void removeDashes(char *);
void *fun_sig_handler(void *);

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        dprintf(2, "\e[1m\nUsage: ./biblient [--field]... [-p]\n\n[--field]: Wwhat you want to search for.\n     [-p]: Loan request.\n\n");
        exit(EXIT_SUCCESS);
    }

    en = pthread_mutex_init(&mtx_log, NULL);
    handle_en_error(en, "pthread_mutex_init");

    //------------------------------- arguments management -------------------------------------------------------

    linked_list_t *l;
    l = arguments_parser(argv, argc);
    int server = 0;
    // printList(l);

    //--------------------------------- message management ---------------------------------------------------------
    char type = MSG_ERROR;
    unsigned int length = 0;

    if (prestito)
    {
        type = MSG_LOAN;
    }
    else
    {
        type = MSG_QUERY;
    }

    size_t size = 0;

    for (int j = 0; j < l->size; j++)
    {
        variable_for_request *var = get_nth_element(l, j);
        size = size + size_of_ptr(var->etichetta) + size_of_ptr(var->valore) + (sizeof(char) * 2) + 10;
    }
    size = size + 1;
    length = (unsigned int)size;
    char *data = calloc(1, length);

    for (int i = 0; i < l->size; i++)
    {
        variable_for_request *var = get_nth_element(l, i);
        strcat(data, var->etichetta);
        strcat(data, ":");
        strcat(data, var->valore);
        strcat(data, ";");
    }

    //-------------------------------- configuration file management ---------------------------------------------

    FILE *configFile;

    configFile = fopen("./conf/addr.conf", "r");
    handle_null_error(configFile, "fopen config file");

    int count = 0;
    char lineptr[BUF_SIZE];
    char *s;
    while (!feof(configFile))
    {
        s = fgets(lineptr, BUF_SIZE, configFile);

        if (s == NULL)
        {
            if (count == 0)
            {
                message(&mtx_log, "\nconfiguration file './conf/bib.conf' is empty.\nshutting down client...\n\n");
                exit(EXIT_SUCCESS);
            }
            break;
        }
        count++;
    }
    en = fseek(configFile, 0, SEEK_SET);
    handle_en_error(en, "fseek");

    long sock[count];
    int index = 0;
    while (ptr = fgets(lineptr, BUF_SIZE, configFile) != NULL)
    {
        handle_null_error(ptr, "fgets");
        if (strcmp(lineptr, "\n") != 0)
        {
            char *copia = strtok(lineptr, ",");
            char *appoggio = strtok(NULL, ":");
            char *n_socketFD = strtok(NULL, "\n");
            sock[index] = atoi(n_socketFD);
            index++;
        }
    }
    en = fclose(configFile);
    handle_en_error(en, "fclose");

    //----------------------------- socket management -----------------------------------------------------------

    int array_for_serverSocket[count];

    for (int i = 0; i < count; i++)
    {
        if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            handle_en_error(errno, "socket");
        }
        array_for_serverSocket[i] = server;
    }
    struct sockaddr_in array_for_serverAddress[count];

    for (int i = 0; i < count; i++)
    {
        array_for_serverAddress[i].sin_family = AF_INET;
        array_for_serverAddress[i].sin_addr.s_addr = inet_addr(IP);
        array_for_serverAddress[i].sin_port = htons(sock[i]);
    }

    int connection = 0;
    int index_for_addr = 0;
    void *message_to_send = calloc(1, size);
    ptr = memcpy(message_to_send, &type, sizeof(type));
    handle_null_error(ptr, "memcpy");
    ptr = memcpy(message_to_send + sizeof(char), &length, sizeof(unsigned int));
    handle_null_error(ptr, "memcpy");
    ptr = memcpy(message_to_send + sizeof(char) + sizeof(unsigned int), data, strlen(data));
    handle_null_error(ptr, "memcpy");

    //-----------------------------------main loop ---------------------------------------------------------

    while (1)
    {
        char type_received;
        unsigned int length_received = 0;
        char *data_received = NULL;
        connection = connect(array_for_serverSocket[index_for_addr], (struct sockaddr *)&array_for_serverAddress[index_for_addr], sizeof(array_for_serverAddress[index_for_addr]));

        if (connection == 0)
        {
            char buff[BUF_SIZE] = "";
            write(array_for_serverSocket[index_for_addr], message_to_send, size);

            int n_bytes_readed = 0;

            while (n_bytes_readed = read(array_for_serverSocket[index_for_addr], &type_received, sizeof(char)))
            {
                if (type_received == MSG_END)
                {
                    break;
                }
                if (type_received == MSG_ERROR)
                {
                    dprintf(2, "type received: %c\n", type_received);
                    dprintf(2, "length received: %d\n", length_received);
                    dprintf(2, "data received: %s\n", data_received);
                    exit(EXIT_FAILURE);
                }

                dprintf(2, "type received: %c\n", type_received);
                n_bytes_readed = read(array_for_serverSocket[index_for_addr], &length_received, sizeof(unsigned int));
                dprintf(2, "length received: %d\n", length_received);
                data_received = calloc(1, (size_t)length_received + 1);
                n_bytes_readed = read(array_for_serverSocket[index_for_addr], data_received, length_received);
                dprintf(2, "data received: %s\n", data_received);
                safe_free(data_received);
                if (n_bytes_readed == 0)
                {
                    break;
                }
            }
        }
        index_for_addr++;
        if (index_for_addr == count)
        {
            break;
        }
    }

    //--------------------------------- free memory ---------------------------------------------------------
    safe_free(message_to_send);
    safe_free(data);
    free_list(l);
    safe_free(l);
    close(server);
    return 0;
}

/**
 * Converts a string to uppercase.
 *
 * @param str string to convert.
 */
void toUpperCase(char *str)
{
    if (str[0] >= 'a' && str[0] <= 'z')
    {
        str[0] = str[0] - 32;
    }
    else
        return;
}

/**
 * Parses the arguments passed to the program and returns a linked list containing the arguments.
 *
 * @param arguments arguments passed to the program.
 * @param n_arg number of arguments passed to the program.
 * @return linked_list_t*: linked list containing the arguments.
 */
linked_list_t *arguments_parser(char *arguments[], int n_arg)
{
    linked_list_t *list = (linked_list_t *)malloc(sizeof(linked_list_t));
    initialize_list(list);
    char *var_to_tok;
    char *etichetta;
    char *valore;
    int aut = 0, tit = 0, edi = 0, ann = 0, vol = 0, scaf = 0, col = 0, luo = 0, desc = 0, not = 0;

    for (int i = 1; i < n_arg; i++)
    {

        if (strstr(arguments[i], "--autore") != NULL)
        {
            if (aut)
            {
                printf("Impossible arguments");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var1 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var1->etichetta = etichetta;
            this_var1->valore = valore;

            add_node(list, this_var1);
            aut = 1;
            // free(this_var1);
        }
        else if (strstr(arguments[i], "--titolo") != NULL)
        {
            if (tit)
            {
                printf("Impossible arguments");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var2 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var2->etichetta = etichetta;
            this_var2->valore = valore;

            add_node(list, this_var2);
            tit = 1;
            // free(this_var2);
        }
        else if (strstr(arguments[i], "--editore") != NULL)
        {
            if (edi)
            {
                printf("Impossible arguments");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var3 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var3->etichetta = etichetta;
            this_var3->valore = valore;

            add_node(list, this_var3);
            edi = 1;
            // free(this_var3);
        }
        else if (strstr(arguments[i], "--anno") != NULL)
        {
            if (ann)
            {
                printf("Impossible arguments");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var4 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var4->etichetta = etichetta;
            this_var4->valore = valore;

            add_node(list, this_var4);
            ann = 1;
            // free(this_var4);
        }
        else if (strstr(arguments[i], "--volume") != NULL)
        {
            if (vol)
            {
                printf("Impossible arguments");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var5 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var5->etichetta = etichetta;
            this_var5->valore = valore;

            add_node(list, this_var5);
            vol = 1;
        }
        else if (strstr(arguments[i], "--scaffale") != NULL)
        {
            if (scaf)
            {
                printf("Impossible arguments");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var6 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var6->etichetta = etichetta;
            this_var6->valore = valore;

            add_node(list, this_var6);
            scaf = 1;
        }
        else if (strstr(arguments[i], "--collocazione") != NULL)
        {
            if (col)
            {
                printf("Impossible arguments");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var7 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var7->etichetta = etichetta;
            this_var7->valore = valore;

            add_node(list, this_var7);
            col = 1;
        }
        else if (strstr(arguments[i], "--luogo_pubblicazione") != NULL)
        {
            if (luo)
            {
                printf("Impossible arguments");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var8 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var8->etichetta = etichetta;
            this_var8->valore = valore;

            add_node(list, this_var8);
            luo = 1;
        }
        else if (strstr(arguments[i], "--descrizione_fisica") != NULL)
        {
            if (desc)
            {
                printf("Impossible arguments");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var9 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var9->etichetta = etichetta;
            this_var9->valore = valore;

            add_node(list, this_var9);
            desc = 1;
        }
        else if (strstr(arguments[i], "--nota") != NULL)
        {
            if (not )
            {
                printf("Impossible arguments\n");
                exit(EXIT_FAILURE);
            }
            variable_for_request *this_var10 = (variable_for_request *)malloc(sizeof(variable_for_request));
            var_to_tok = arguments[i];
            etichetta = strtok(var_to_tok, "=");
            removeDashes(etichetta);
            valore = strtok(NULL, "=");
            toUpperCase(valore);

            this_var10->etichetta = etichetta;
            this_var10->valore = valore;

            add_node(list, this_var10);
            not = 1;
        }
        else if (strstr(arguments[i], "-p") != NULL)
        {
            prestito = 1;
        }
        else
        {
            printf("Impossible arguments\n");
            exit(EXIT_FAILURE);
        }
    }

    return list;
}

/**
 * Removes the first two dashes from a string.
 *
 * @param str string to modify.
 */
void removeDashes(char *str)
{
    int len = strlen(str);
    int i, j;

    for (i = 0, j = 0; i < len; i++)
    {
        if (i < 2 && str[i] == '-')
        {
            continue; // Salta i primi due trattini
        }
        str[j++] = str[i];
    }

    str[j] = '\0';
}

/**
 * Prints all the elements of the list.
 *
 * @attention This function is not thread safe so it should be used only for debugging purposes.
 *
 * @param list list to print.
 */
void printList(linked_list_t *list)
{
    int index = 0;
    char *prova;
    variable_for_request *act = malloc(sizeof(variable_for_request));

    while (1)
    {

        act = get_nth_element(list, index);
        prova = strdup(act->etichetta);

        if (strcmp(prova, "END") == 0)
        {
            break;
        }
        else
        {
            printf("[%d]-> etichetta: %s valore: %s\n", index, act->etichetta, act->valore);
            index++;
        }
    }
    free(act);
    act = NULL;
    return;
}