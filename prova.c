#define _XOPEN_SOURCE
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

int date_check(char *date)
{
    double diff = 0;
    time_t currentTime = time(NULL);
    time_t timestamp;

    struct tm *actualTime = localtime(&currentTime);
    struct tm *endRentTime = malloc(sizeof(struct tm)); /*= localtime(&currentTime)*/
    ;
    if (strptime(date, "%d-%m-%Y %OH:%M:%S", endRentTime) != NULL)
    {
        timestamp = mktime(endRentTime);
    }
    else
    {
        //message("Date format not valid\n");
    }
    diff = difftime(currentTime, timestamp);
    if (diff >= 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int main() {
    char *date = "";
    printf("%d\n", date_check(date));

    return 0;
}
