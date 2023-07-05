#ifndef BOOK_H
#define BOOK_H

#include "./linkedList.h"
#define BUF_SIZE 1024

typedef struct
{
    char autore[BUF_SIZE];
    char titolo[BUF_SIZE];
    char editore[BUF_SIZE];
    char anno[BUF_SIZE];
    char volume[BUF_SIZE];
    char scaffale[BUF_SIZE];
    char collocazione[BUF_SIZE];
    char luogo_pubblicazione[BUF_SIZE];
    char descrizione_fisica[BUF_SIZE];
    char nota[BUF_SIZE];
    char prestito[BUF_SIZE];
} single_book;

typedef enum {
    AUTHOR,
    TITLE,
    EDITOR,
    YEAR,
    VOLUME,
    SHELF,
    LOCATION,
    PLACE,
    PHYSICAL,
    NOTE,
    LOAN
} Book_Fields;

typedef struct {
    Book_Fields field;
    char *value;
} Book_Search;

void *end_of_book(single_book *);
void *initialize_book(single_book *);
void tokenize(char *, single_book *);
void print_books_list(linked_list_t *);
void remove_spaces(char *);
void loan_check(linked_list_t *);
single_book *data_tokenizer(char *);
int book_contains(single_book*, Book_Fields, char*);
int book_contains_all(single_book*, single_book*);
char *rec_to_string(single_book *book);
size_t size_of_book(single_book *b);
int date_check(char *);

#endif