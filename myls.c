#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#define ARRAY_ELEMS(x) (sizeof(x) / sizeof(x[0]))
#define STREQL(x, y) (strcmp(x, y) == 0)
#define USAGE(str) (printf("Usage: %s\n", str))

int main(int argc, char **argv)
{
	printf("Hello world! (%d, %s) \n", argc, argv[0]);
}