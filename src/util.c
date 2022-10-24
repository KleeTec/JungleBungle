#include <sys/time.h>
#include <malloc.h>
#include <string.h>

#include "include/util.h"

long currentTimeMillis() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return ( tv.tv_sec ) * 1000 + ( tv.tv_usec ) / 1000;
}

char* appendChar(char* str1, char* str2) {
    char* result = malloc(strlen(str1) + strlen(str2) + 1);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}