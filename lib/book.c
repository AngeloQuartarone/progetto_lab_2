#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "./book.h"
#include "util.h"


#define BUF_SIZE 1024
#define MAX_FIELDS 128

/**
 * Initialize a single book
 *
 * @param book: book to initialize
 */
void *initialize_book(single_book *book)
{
    strcpy(book->autore, "");
    strcpy(book->titolo, "");
    strcpy(book->editore, "");
    strcpy(book->anno, "");
    strcpy(book->volume, "");
    strcpy(book->scaffale, "");
    strcpy(book->collocazione, "");
    strcpy(book->luogo_pubblicazione, "");
    strcpy(book->descrizione_fisica, "");
    strcpy(book->nota, "");
    strcpy(book->prestito, "");
    return 0;
}

/**
 * Initialize a single book with all fields set to "END"
 * This function is not used in the program, but it is useful future improvements
 *
 * @param EOB: book to initialize
 */
void *end_of_book(single_book *EOB)
{
    strcpy(EOB->autore, "END");
    strcpy(EOB->titolo, "END");
    strcpy(EOB->editore, "END");
    strcpy(EOB->anno, "END");
    strcpy(EOB->volume, "END");
    strcpy(EOB->scaffale, "END");
    strcpy(EOB->collocazione, "END");
    strcpy(EOB->luogo_pubblicazione, "END");
    strcpy(EOB->descrizione_fisica, "END");
    strcpy(EOB->nota, "END");
    strcpy(EOB->prestito, "END");
    return 0;
}

int is_present(linked_list_t *l, single_book *b)
{
    int index = 0;
    single_book *act;
    while (1)
    {
        if(index == l->size){
            break;
        }
        act = (single_book *)get_nth_element(l, index);
        if (strcmp(act->autore, b->autore) == 0 && strcmp(act->titolo, b->titolo) == 0 &&
                 strcmp(act->editore, b->editore) == 0 && strcmp(act->anno, b->anno) == 0 &&
                 strcmp(act->volume, b->volume) == 0 && strcmp(act->scaffale, b->scaffale) == 0 &&
                 strcmp(act->collocazione, b->collocazione) == 0 &&
                 strcmp(act->luogo_pubblicazione, b->luogo_pubblicazione) == 0 &&
                 strcmp(act->descrizione_fisica, b->descrizione_fisica) == 0 &&
                 strcmp(act->nota, b->nota) == 0 && strcmp(act->prestito, b->prestito) == 0)
        {
            return 1;
        }
        else
        {
            index++;
        }
    }
    return 0;
}

/**
 * Tokenize a single line from a file and store it in a single_book struct
 *
 * @param linea string to tokenize
 * @param book book to store the data
 */
void tokenize(char *linea, single_book *book)
{
    char *etichetta;
    char *valore;
    etichetta = strtok(linea, ":");
    valore = strtok(NULL, ";");
    remove_spaces(etichetta);
    remove_spaces(valore);
    while (etichetta != NULL && valore != NULL)
    {

        if (strstr(etichetta, "autore") != NULL)
        {
            strcat(book->autore, valore);
            remove_spaces(book->autore);
        }
        else if (strstr(etichetta, "titolo") != NULL)
        {
            strcat(book->titolo, valore);
            remove_spaces(book->titolo);
        }
        else if (strstr(etichetta, "editore") != NULL)
        {
            strcat(book->editore, valore);
            remove_spaces(book->editore);
        }
        else if (strstr(etichetta, "anno") != NULL)
        {
            strcat(book->anno, valore);
            remove_spaces(book->anno);
        }
        else if (strstr(etichetta, "volume") != NULL)
        {
            strcat(book->volume, valore);
            remove_spaces(book->volume);
        }
        else if (strstr(etichetta, "scaffale") != NULL)
        {
            strcat(book->scaffale, valore);
            remove_spaces(book->scaffale);
        }
        else if (strstr(etichetta, "collocazione") != NULL)
        {
            strcat(book->collocazione, valore);
            remove_spaces(book->collocazione);
        }
        else if (strstr(etichetta, "luogo_pubblicazione") != NULL)
        {
            strcat(book->luogo_pubblicazione, valore);
            remove_spaces(book->luogo_pubblicazione);
        }
        else if (strstr(etichetta, "descrizione_fisica") != NULL)
        {
            strcat(book->descrizione_fisica, valore);
            remove_spaces(book->descrizione_fisica);
        }
        else if (strstr(etichetta, "nota") != NULL)
        {
            strcat(book->nota, valore);
            remove_spaces(book->nota);
        }
        else if (strstr(etichetta, "prestito") != NULL)
        {
            strcat(book->prestito, valore);
            remove_spaces(book->prestito);
        }
        else
        {
            perror("Etichetta non riconosciuta");
            exit(EXIT_FAILURE);
        }

        etichetta = strtok(NULL, ":");
        valore = strtok(NULL, ";");
    }
}

/**
 * Print a single book in a human readable format
 * @attention this function is not thread safe, use it only for debug purposes
 *
 * @param l linked list to print
 */
void print_books_list(linked_list_t *l)
{
    int index = 0;
    single_book *act;
    while (1)
    {
        if (index == l->size)
        {
            break;
        }
        act = get_nth_element(l, index);

        printf("autore[%d]: %s.\n", index, act->autore);
        printf("titolo[%d]: %s.\n", index, act->titolo);
        printf("editore[%d]: %s.\n", index, act->editore);
        printf("anno[%d]: %s.\n", index, act->anno);
        printf("volume[%d]: %s.\n", index, act->volume);
        printf("scaffale[%d]: %s.\n", index, act->scaffale);
        printf("collocazione[%d]: %s.\n", index, act->collocazione);
        printf("luogo_pubblicazione[%d]: %s.\n", index, act->luogo_pubblicazione);
        printf("descrizione_fisica[%d]: %s.\n", index, act->descrizione_fisica);
        printf("nota[%d]: %s.\n", index, act->nota);
        printf("prestito[%d]: %s.\n", index, act->prestito);
        printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        index++;
    }
    return;
}

/**
 * Remove the spaces of the beginning and the end of a string
 *
 * @param str string to remove the spaces
 */
void remove_spaces(char str[])

{
    int start = 0, end = 0;
    int length = strlen(str);

    while (isspace(str[start]))
        start++;

    end = length - 1;
    while (end >= start && isspace(str[end]))
        end--;

    int i, j = 0;
    for (i = start; i <= end; i++)
    {
        str[j] = str[i];
        j++;
    }

    str[j] = '\0';
}

/**
 * Tokeize the string passed from the client and store it in a single_book struct
 * This function check the validity of the data passed from the client (the date must be between 1500 and the actual year)
 *
 * @param str_to_tok
 * @return single_book*
 */
single_book *data_tokenizer(char *str_to_tok)
{
    single_book *book_to_return = calloc(1, sizeof(single_book));
    handle_null_error(book_to_return, "calloc in data_tokenizer");
    initialize_book(book_to_return);
    char *etichetta;
    char *valore;

    etichetta = strtok(str_to_tok, ":");
    valore = strtok(NULL, ";");

    while (etichetta != NULL && valore != NULL)
    {
        if (strcmp(etichetta, "autore") == 0)
        {
            strcpy(book_to_return->autore, valore);
        }
        else if (strcmp(etichetta, "titolo") == 0)
        {
            strcpy(book_to_return->titolo, valore);
        }
        else if (strcmp(etichetta, "editore") == 0)
        {
            strcpy(book_to_return->editore, valore);
        }
        else if (strcmp(etichetta, "anno") == 0)
        {
            if (atoi(valore) < 1500 || atoi(valore) > 2023)
            {
                strcpy(book_to_return->anno, "ERROR");
            }
        }
        else if (strcmp(etichetta, "volume") == 0)
        {
            strcpy(book_to_return->volume, valore);
        }
        else if (strcmp(etichetta, "scaffale") == 0)
        {
            strcpy(book_to_return->scaffale, valore);
        }
        else if (strcmp(etichetta, "collocazione") == 0)
        {
            strcpy(book_to_return->collocazione, valore);
        }
        else if (strcmp(etichetta, "luogo_pubblicazione") == 0)
        {
            strcpy(book_to_return->luogo_pubblicazione, valore);
        }
        else if (strcmp(etichetta, "descrizione_fisica") == 0)
        {
            strcpy(book_to_return->descrizione_fisica, valore);
        }
        else if (strcmp(etichetta, "nota") == 0)
        {
            strcpy(book_to_return->nota, valore);
        }
        else
        {
            perror("Etichetta non riconosciuta");
        }

        etichetta = strtok(NULL, ":");
        valore = strtok(NULL, ";");
    }

    return book_to_return;
}

/**
 * Convert a single_book struct to a string
 *
 * @param book book to convert
 * @return char* string converted
 */
char *rec_to_string(single_book *book)
{
    char *str = calloc(1, (sizeof(char) * BUF_SIZE));
    handle_null_error(str, "calloc in rec_to_string");
    strcat(str, "autore:");
    strcat(str, book->autore);
    strcat(str, ";titolo:");
    strcat(str, book->titolo);
    strcat(str, ";editore:");
    strcat(str, book->editore);
    strcat(str, ";anno:");
    strcat(str, book->anno);
    strcat(str, ";volume:");
    strcat(str, book->volume);
    strcat(str, ";scaffale:");
    strcat(str, book->scaffale);
    strcat(str, ";collocazione:");
    strcat(str, book->collocazione);
    strcat(str, ";luogo_pubblicazione:");
    strcat(str, book->luogo_pubblicazione);
    strcat(str, ";descrizione_fisica:");
    strcat(str, book->descrizione_fisica);
    strcat(str, ";nota:");
    strcat(str, book->nota);
    strcat(str, ";prestito:");
    strcat(str, book->prestito);
    strcat(str, ";");
    return str;
}

/**
 * Check if a book contains a key in a specific field
 *
 * @param book book to check
 * @param field field to check
 * @param key key to search
 * @return int 1 if the book contains the key, 0 otherwise
 */
int book_contains(single_book *book, Book_Fields field, char *key)
{
    if ((strcmp(key, "") == 0) || key == NULL)
    {
        return 1;
    }
    char *s = NULL;
    switch (field)
    {
    case AUTHOR:
        s = book->autore;
        break;
    case TITLE:
        s = book->titolo;
        break;
    case EDITOR:
        s = book->editore;
        break;
    case YEAR:
        s = book->anno;
        break;
    case VOLUME:
        s = book->volume;
        break;
    case SHELF:
        s = book->scaffale;
        break;
    case LOCATION:
        s = book->collocazione;
        break;
    case PLACE:
        s = book->luogo_pubblicazione;
        break;
    case PHYSICAL:
        s = book->descrizione_fisica;
        break;
    case NOTE:
        s = book->nota;
        break;
    case LOAN:
        s = book->prestito;
        break;
    }
    int result = (strstr(s, key) != NULL);
    return result;
}

/**
 * Check if a book contains all the keys in a specific field
 *
 * @param book book to check
 * @param search_book book to search
 * @return int 1 if the book contains all the keys, 0 otherwise
 */
int book_contains_all(single_book *book, single_book *search_book)
{
    if (search_book == NULL)
    {
        return 1;
    }
    int result = 1;
    result &= book_contains(book, AUTHOR, search_book->autore);
    result &= book_contains(book, TITLE, search_book->titolo);
    result &= book_contains(book, EDITOR, search_book->editore);
    result &= book_contains(book, YEAR, search_book->anno);
    result &= book_contains(book, VOLUME, search_book->volume);
    result &= book_contains(book, SHELF, search_book->scaffale);
    result &= book_contains(book, LOCATION, search_book->collocazione);
    result &= book_contains(book, PLACE, search_book->luogo_pubblicazione);
    result &= book_contains(book, PHYSICAL, search_book->descrizione_fisica);
    result &= book_contains(book, NOTE, search_book->nota);
    result &= book_contains(book, LOAN, search_book->prestito);
    return result;
}

/**
 * Calculate the size of a single_book struct
 *
 * @param b book to calculate the size
 * @return size_t size of the book
 */
size_t size_of_book(single_book *b)
{
    size_t size = 0;

    size += size_of_ptr(b->autore);
    size += size_of_ptr(b->titolo);
    size += size_of_ptr(b->editore);
    size += size_of_ptr(b->anno);
    size += size_of_ptr(b->volume);
    size += size_of_ptr(b->scaffale);
    size += size_of_ptr(b->collocazione);
    size += size_of_ptr(b->luogo_pubblicazione);
    size += size_of_ptr(b->descrizione_fisica);
    size += size_of_ptr(b->nota);
    size += size_of_ptr(b->prestito);
    size += 24;
    return size;
}

/**
 * Check if a date is passed or not
 *
 * @param date date to check
 * @return int 1 if the date is passed, 0 otherwise
 */
int date_check(char *date)
{
    double diff = 0;
    time_t currentTime = time(NULL);
    time_t timestamp;

    struct tm *endRentTime = calloc(1, sizeof(struct tm));
    ;
    if (strptime(date, "%d-%m-%Y %OH:%M:%S", endRentTime) != NULL)
    {
        timestamp = mktime(endRentTime);
    }
    diff = difftime(currentTime, timestamp);
    if (diff >= 0)
    {
        safe_free(endRentTime);
        return 1;
    }
    else
    {
        safe_free(endRentTime);
        return 0;
    }
}

/**
 * this function check if all book in the linked list have the prestito field, if not it set it to "AVAILABLE"
 *
 * @param l linked list to check
 */
void loan_check(linked_list_t *l)
{
    int index = 0;
    single_book *act;
    while (1)
    {
        act = (single_book *)get_nth_element(l, index);
        if (strcmp(act->prestito, "") == 0)
        {
            strcpy(act->prestito, "AVAILABLE");
        }
        if (index == l->size - 1)
        {
            break;
        }
        else
        {
            index++;
        }
    }
    return;
}