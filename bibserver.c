#define _XOPEN_SOURCE
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/select.h>
#include <fcntl.h>
#include <time.h>
#include "./lib/linkedList.h"
#include "./lib/queue.h"
#include "./lib/book.h"
#include "./lib/util.h"

#define BUF_SIZE 1024
#define MAX_FIELDS 64
#define IP "127.0.0.1"
#define PORT 0
#define MSG_QUERY 'Q'
#define MSG_LOAN 'L'
#define MSG_RECORD 'R'
#define MSG_NO 'N'
#define MSG_ERROR 'E'
#define MSG_END 'X'

static int running = 1;
static int *en = 0;
static void *ptr;
static pthread_mutex_t *mtx_log;
static pthread_mutex_t *mtx_for_loan;
FILE *log_file;

typedef struct
{
    Queue *queue_of_FDs;
    linked_list_t *all_books;
    pthread_mutex_t *mutex;
    fd_set *clients;
    int *maxFD;
} str_for_worker;

void aggiornaMax(int *, fd_set);
void *worker(void *);
void LOG(const char *, ...);
void *fun_sig_handler(void *);

int main(int argc, char const *argv[])
{
    if (argc <= 2)
    {
        dprintf(2, "\e[1m\nUsage: ./bibserver NAME_BIB FILE_RECORD [W]\n\n   NAME_BIB: name of the server\nFILE_RECORD: path of the file record to read\n          W: number of worker threads\n\n");
        exit(EXIT_SUCCESS);
    }

    //--------------------------------- signal management ---------------------------------
    en = pthread_mutex_init(&mtx_log, NULL);
    handle_en_error(en, "pthread_mutex_init");
    en = pthread_mutex_init(&mtx_for_loan, NULL);
    handle_en_error(en, "pthread_mutex_init");

    sigset_t set;
    en = sigemptyset(&set);
    handle_en_error(en, "sigemptyset");
    en = sigaddset(&set, SIGINT);
    handle_en_error(en, "sigaddset");
    en = sigaddset(&set, SIGTERM);

    pthread_t signal_handler;
    en = pthread_sigmask(SIG_BLOCK, &set, NULL);
    handle_en_error(en, "pthread_sigmask");
    en = pthread_create(&signal_handler, NULL, fun_sig_handler, &set);
    handle_en_error(en, "pthread_create");
    en = pthread_detach(signal_handler);
    handle_en_error(en, "pthread_detach");

    int n_thr_Server = atoi(argv[3]);
    const char *filename = argv[2];
    char *lineptr = NULL;
    lineptr = calloc(1, BUF_SIZE);
    handle_null_error(lineptr, "calloc");
    linked_list_t *books = calloc(1, sizeof(linked_list_t));
    handle_null_error(books, "calloc");
    initialize_list(books);

    char *log_file_name = calloc(1, BUF_SIZE);
    handle_null_error(log_file_name, "calloc");
    strcpy(log_file_name, "./log/");
    strcat(log_file_name, argv[1]);
    strcat(log_file_name, ".log");
    log_file = fopen(log_file_name, "w+");
    handle_null_error(log_file_name, "fopen");

    //--------------------------------- parsing bib file ---------------------------------

    FILE *opened_file = NULL;
    opened_file = fopen(filename, "r");
    if (opened_file == NULL)
    {
        message(&mtx_log, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    else
    {

        while ((ptr = fgets(lineptr, BUF_SIZE, opened_file)) != NULL)
        {
            if (strcmp(lineptr, "\n") != 0)
            {
                single_book *book = calloc(1, sizeof(single_book));
                initialize_book(book);
                tokenize(lineptr, book);
                int present = is_present(books, book);
                if (!present)
                {
                    add_node(books, book);
                }
                else
                {
                    safe_free(book);
                }
            }
        }
    }
    en = fclose(opened_file);
    handle_en_error(en, "fclose");
    loan_check(books);
    // print_books_list(books);

    //--------------------------------- socket management ---------------------------------

    int serverSocket = 0;
    struct sockaddr_in serverAddr;
    fd_set allFDs, readFDs;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        handle_en_error(serverSocket, "socket failed");
    }
    en = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    handle_en_error(en, "setsockopt");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    serverAddr.sin_port = htons(PORT);

    en = bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    handle_en_error(en, "bind");

    socklen_t size_of_sockaddr = sizeof(serverAddr);
    en = getsockname(serverSocket, (struct sockaddr *)&serverAddr, &size_of_sockaddr);
    handle_en_error(en, "getsockname");

    FILE *conf;
    conf = fopen("./conf/addr.conf", "a");
    en = flock(fileno(conf), LOCK_EX);
    handle_en_error(en, "flock");
    en = fprintf(conf, "SERVER_NAME:%s, SOCKET_FD:%d\n", argv[1], ntohs(serverAddr.sin_port));
    if (en < 0)
    {
        handle_en_error(en, "fprintf");
    }
    en = flock(fileno(conf), LOCK_UN);
    handle_en_error(en, "flock");
    en = fclose(conf);
    handle_en_error(en, "fclose");
    Queue *queue_of_sock = calloc(1, sizeof(Queue));
    handle_null_error(queue_of_sock, "calloc");
    init(queue_of_sock);
    pthread_mutex_t m;
    en = pthread_mutex_init(&m, NULL);
    handle_en_error(en, "pthread_mutex_init");

    if (listen(serverSocket, SOMAXCONN) < 0)
    {
        message(&mtx_log, "listen failed");
        exit(EXIT_FAILURE);
    }
    FD_ZERO(&allFDs);
    FD_SET(serverSocket, &allFDs);
    int maxFD = serverSocket;
    int lastFd = 0;
    int clientFD = -1;

    //--------------------------------- thread management ---------------------------------
    str_for_worker args;
    args.queue_of_FDs = queue_of_sock;
    args.all_books = books;
    args.mutex = &m;
    args.maxFD = &maxFD;
    args.clients = &allFDs;

    pthread_t thread_id[n_thr_Server];
    for (int j = 0; j < n_thr_Server; j++)
    {
        en = pthread_create(&thread_id[j], NULL, worker, &args);
        handle_en_error(en, "pthread_create");
    }

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000000;
    int check_select = -1;

    //--------------------------------- main loop ---------------------------------------------

    int *client = 0;
    while (running)
    {
        en = pthread_mutex_lock(&m);
        handle_en_error(en, "pthread_mutex_lock");
        readFDs = allFDs;
        int currMax = maxFD;
        en = pthread_mutex_unlock(&m);
        handle_en_error(en, "pthread_mutex_unlock");

        if ((check_select = select(currMax + 1, &readFDs, NULL, NULL, &timeout)) == -1)
        {
            message(&mtx_log, "select failed");
            exit(EXIT_FAILURE);
        }

        for (int i = 0; i < currMax + 1; i++)
        {
            if (FD_ISSET(i, &readFDs))
            {

                if (i == serverSocket)
                {

                    if ((clientFD = accept(serverSocket, NULL, NULL)) < 0)
                    {
                        message(&mtx_log, "accept failed");
                        exit(EXIT_FAILURE);
                    }
                    en = pthread_mutex_lock(&m);
                    handle_en_error(en, "pthread_mutex_lock");
                    FD_SET(clientFD, &allFDs);
                    if (clientFD > maxFD)
                    {
                        maxFD = clientFD;
                    }
                    en = pthread_mutex_unlock(&m);
                    handle_en_error(en, "pthread_mutex_unlock");
                }
                else
                {
                    if (i > lastFd)
                    {
                        client = calloc(1, sizeof(int));
                        handle_null_error(client, "calloc");
                        *client = i;
                        lastFd = i;
                        push(client, queue_of_sock);
                    }
                }
            }
        }
        check_select = 0;
    }

    //--------------------------------- end of program ---------------------------------------------

    for (int i = 0; i < n_thr_Server; i++)
    {
        en = pthread_join(thread_id[i], NULL);
        handle_en_error(en, "pthread_join");
    }

    char *directory;
    strtok(filename, "/");
    directory = strtok(NULL, "/");
    char *f_name = strtok(NULL, "");
    char *new_path = calloc(1, BUF_SIZE);
    strcpy(new_path, "./");
    strcat(new_path, "bibData_to_test");
    strcat(new_path, "/");
    strcat(new_path, f_name);

    opened_file = fopen(new_path, "w+");
    if (opened_file == NULL)
    {
        message(&mtx_log, "Error opening file %s\n", filename);
        exit(EXIT_FAILURE);
    }
    else
    {
        for (int i = 0; i < books->size; i++)
        {
            single_book *book = get_nth_element(books, i);
            char *data_to_send = rec_to_string(book);
            fprintf(opened_file, "%s\n", data_to_send);
            safe_free(data_to_send);
        }
    }
    fclose(opened_file);
    fclose(log_file);
    safe_free(log_file_name);
    safe_free(new_path);
    safe_free(lineptr);
    safe_free(client);
    free_list(books);
    safe_free(books);
    destroy(queue_of_sock);
    safe_free(queue_of_sock);
    pthread_mutex_destroy(&mtx_log);
    return 0;
}

// --------------------------------- FUNCTIONS -------------------------------------------------

/**
 * Behaviour of threads worker.
 *
 * @param args struct that all threads need to work. See str_for_worker for more.
 */
void *worker(void *args)
{
    Queue *qFDs = ((str_for_worker *)args)->queue_of_FDs;
    linked_list_t *books = ((str_for_worker *)args)->all_books;
    pthread_mutex_t *m = ((str_for_worker *)args)->mutex;
    int *maxFD = ((str_for_worker *)args)->maxFD;
    fd_set *client = ((str_for_worker *)args)->clients;
    time_t rawtime;
    struct tm *timeinfo;
    int queryNumber = 0;
    int loanRequestNumber = 0;
    int isLoan = 0;
    int isPresent = 0;

    int n_bytes_readed = 0;
    single_book *book_to_search;

    while (1)
    {
        isLoan = 0;
        queryNumber = 0;
        loanRequestNumber = 0;
        int *fd = (int *)non_blocking_pop(qFDs);
        if (fd == NULL)
        {
            if (!running)
            {
                safe_free(fd);
                break;
            }
            if (fd == NULL)
            {
                continue;
            }
        }
        unsigned int size = 0;
        char type = MSG_ERROR;

        n_bytes_readed = read(*fd, &type, sizeof(char));
        n_bytes_readed = read(*fd, &size, sizeof(unsigned int));
        char *data = calloc(1, size);
        handle_null_error(data, "calloc");
        n_bytes_readed = read(*fd, data, (size_t)size);
        book_to_search = data_tokenizer(data);

        if (n_bytes_readed > 0)
        {

            if (strcmp(book_to_search->anno, "ERROR") == 0)
            {
                char *data_to_send = "ERROR";
                size_t length = strlen(data_to_send);
                size = sizeof(char) + sizeof(unsigned int) + strlen(data_to_send);
                void *message_to_send = calloc(1, size);
                memcpy(message_to_send, &type, sizeof(char));
                memcpy(message_to_send + sizeof(char), &length, sizeof(unsigned int));
                memcpy(message_to_send + sizeof(char) + sizeof(unsigned int), data_to_send, length);
                write(*fd, message_to_send, size);
                continue;
            }
            else
            {
                int hit = 0;
                for (int i = 0; i < books->size; i++)
                {

                    single_book *book = get_nth_element(books, i);
                    isPresent = book_contains_all(book, book_to_search);
                    if (isPresent)
                    {
                        if (type == MSG_LOAN)
                        {
                            time(&rawtime);
                            timeinfo = localtime(&rawtime);

                            en = pthread_mutex_lock(&mtx_for_loan);
                            handle_en_error(en, "pthread_mutex_lock");
                            int available = date_check(book->prestito);
                            if (available || strcmp(book->prestito, "AVAILABLE") == 0)
                            {
                                timeinfo->tm_mday += 30;
                                mktime(timeinfo);
                                strftime(book->prestito, BUF_SIZE, "%d-%m-%Y %OH:%M:%S", timeinfo);
                                en = pthread_mutex_unlock(&mtx_for_loan);
                                handle_en_error(en, "pthread_mutex_unlock");

                                char type = MSG_RECORD;
                                char *data_to_send = rec_to_string(book);
                                size_t length = strlen(data_to_send);
                                size = sizeof(char) + sizeof(unsigned int) + strlen(data_to_send);
                                void *message_to_send = calloc(1, size);

                                ptr = memcpy(message_to_send, &type, sizeof(char));
                                handle_null_error(ptr, "memcpy");
                                ptr = memcpy(message_to_send + sizeof(char), (unsigned int *)&length, sizeof(unsigned int));
                                handle_null_error(ptr, "memcpy");
                                ptr = memcpy(message_to_send + sizeof(char) + sizeof(unsigned int), data_to_send, length);
                                handle_null_error(ptr, "memcpy");
                                write(*fd, message_to_send, size);

                                en = pthread_mutex_lock(&mtx_log);
                                handle_en_error(en, "pthread_mutex_lock");
                                fprintf(log_file, "%s\n", data_to_send);
                                en = pthread_mutex_unlock(&mtx_log);
                                handle_en_error(en, "pthread_mutex_unlock");

                                loanRequestNumber = loanRequestNumber + 1;
                                safe_free(message_to_send);
                                safe_free(data_to_send);
                            }
                            else
                            {
                                en = pthread_mutex_unlock(&mtx_for_loan);
                                handle_en_error(en, "pthread_mutex_unlock");
                                type = MSG_NO;
                                char *data_to_send = "";
                                size_t length = strlen(data_to_send);
                                size = sizeof(char) + sizeof(unsigned int) + strlen(data_to_send);
                                void *message_to_send = calloc(1, size);
                                memcpy(message_to_send, &type, sizeof(char));
                                memcpy(message_to_send + sizeof(char), &length, sizeof(unsigned int));
                                memcpy(message_to_send + sizeof(char) + sizeof(unsigned int), data_to_send, length);
                                write(*fd, message_to_send, size);
                            }
                            isLoan = 1;
                        }
                        else if (type == MSG_QUERY)
                        {
                            char type = MSG_RECORD;
                            char *data_to_send = rec_to_string(book);
                            size_t length = strlen(data_to_send);
                            size = sizeof(char) + sizeof(unsigned int) + strlen(data_to_send);
                            void *message_to_send = calloc(1, size);

                            ptr = memcpy(message_to_send, &type, sizeof(char));
                            handle_null_error(ptr, "memcpy");
                            ptr = memcpy(message_to_send + sizeof(char), (unsigned int *)&length, sizeof(unsigned int));
                            handle_null_error(ptr, "memcpy");
                            ptr = memcpy(message_to_send + sizeof(char) + sizeof(unsigned int), data_to_send, length);
                            handle_null_error(ptr, "memcpy");
                            write(*fd, message_to_send, size);

                            en = pthread_mutex_lock(&mtx_log);
                            handle_en_error(en, "pthread_mutex_lock");
                            fprintf(log_file, "%s\n", data_to_send);
                            en = pthread_mutex_unlock(&mtx_log);
                            handle_en_error(en, "pthread_mutex_unlock");

                            queryNumber = queryNumber + 1;
                        }
                    }
                }
                if (!isPresent)
                {
                    type = MSG_NO;
                    char *data_to_send = "";
                    size_t length = strlen(data_to_send);
                    size = sizeof(char) + sizeof(unsigned int) + strlen(data_to_send);
                    void *message_to_send = calloc(1, size);
                    memcpy(message_to_send, &type, sizeof(char));
                    memcpy(message_to_send + sizeof(char), &length, sizeof(unsigned int));
                    memcpy(message_to_send + sizeof(char) + sizeof(unsigned int), data_to_send, length);
                    write(*fd, message_to_send, size);
                }
            }
            if (isLoan)
            {
                en = pthread_mutex_lock(&mtx_log);
                handle_en_error(en, "pthread_mutex_lock");
                fprintf(log_file, "LOAN %d\n\n", loanRequestNumber);
                en = pthread_mutex_unlock(&mtx_log);
                handle_en_error(en, "pthread_mutex_unlock");
            }
            else
            {
                en = pthread_mutex_lock(&mtx_log);
                handle_en_error(en, "pthread_mutex_lock");
                fprintf(log_file, "QUERY %d\n\n", queryNumber);
                en = pthread_mutex_unlock(&mtx_log);
                handle_en_error(en, "pthread_mutex_unlock");
            }
            type = MSG_END;
            write(*fd, &type, sizeof(char));
        }

        if (n_bytes_readed == 0)
        {

            en = pthread_mutex_lock(&m);
            handle_en_error(en, "pthread_mutex_lock");
            FD_CLR(*fd, client);
            aggiornaMax(maxFD, *client);
            en = pthread_mutex_unlock(&m);
            handle_en_error(en, "pthread_mutex_unlock");
        }

        safe_free(data);
        safe_free(book_to_search);
    }

    return NULL;
}

/**
 * Behaviour of signal handler thread.
 *
 * @param args: sigset_t that the thread need to work.
 */
void *fun_sig_handler(void *arg)
{
    sigset_t *set = arg;
    int s, sig;

    while (1)
    {
        s = sigwait(set, &sig);
        if (s != 0)
        {
            handle_en_error(s, "sigwait");
        }
        else
        {
            // message(&mtx_log, "\n\nShutting down server...\n");
            running = 0;
        }
    }
    return NULL;
}

/**
 * This function update the max value of the set.
 *
 * @param max
 * @param set
 */
void aggiornaMax(int *max, fd_set set)
{
    while (!FD_ISSET(*max, &set))
        (*max)--;
    return;
}