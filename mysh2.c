// #define _XOPEN_SOURCE 700

#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
 
//==========================================
// UTILITIES
//==========================================

#define ARRAY_ELEMS(x) (sizeof(x) / sizeof(x[0]))
#define STREQL(x, y) (strcmp(x, y) == 0)
#define USAGE(str) (printf("Usage: %s\n", str))

// Simple storage for command line args. 
typedef struct
{
	int argc;
	char* buffer;
	char** argv;
} split_args;

const char* SH_CARAT = "$ ";

// Yeah, Yeah. I know. Globals. This is a small program. 
const char* prompt_carat;


void prompt_user()
{
	printf("%s", prompt_carat);
}

bool file_executable(const char* filename)
{
	return access(filename, F_OK) != -1;
}


// Try to find the program in our path. 
char* alloc_find_in_path(const char* command, const split_args* args)
{
	for(int i = 0; i < args->argc; i++) {
		const char* dir = args->argv[i];
		const int dir_len = strlen(dir);
		const int cmd_len = strlen(command);
		const int total_len = strlen(dir) + strlen(command) + 2; // +1 for '/' + for NULL

		char* composite_path = strndup(dir, total_len);
		composite_path[dir_len] = '/';
		strcpy(&composite_path[dir_len + 1], command);

		if(file_executable(composite_path)) {
			return composite_path;
		}
	}

	return NULL;
}

// Parse a line into posix cmd args. 
split_args split_into_args(const char* line, const char* delim)
{
	// Precondition
	assert(line != NULL);
	assert(line != NULL);
	assert(strlen(delim) != 0);

	// Copy the line of text. 
	char* buffer = strdup(line);
	int line_len = strlen(buffer);

	// Split it into tokens. 
	char **argv = NULL;
	// destroy newline characters
	for(int i = 0; i < line_len; i++) {
		if(buffer[i] == '\n') {
			buffer[i] = delim[0];
		}
	}

	char* token = strtok (buffer, delim);
	int argc = 0;

	// Tokenize string
	while(token) {
		argc++;
		argv = realloc(argv, sizeof(char*) * argc);

		assert(argv != NULL);
		argv[argc - 1] = token;
		token = strtok(NULL, delim);
	}

	// Add trailing NULL for free call.
	argv = realloc(argv, sizeof(char*) * (argc + 1));
	argv[argc] = 0;

	// Construct and return value
	split_args args;
	args.argc = argc;
	args.buffer = buffer;
	args.argv = argv;

	return args;
}

// Free our command arguments. 
void free_cmd_args(split_args* args)
{
	free(args->argv);
	free(args->buffer);
}

// bool file_exists(const char* filename)
// {
// 	return access(filename, F_OK) != -1;
// }


bool directory_exists(const char* dirname)
{
	struct stat sb;
	return (stat(dirname, &sb) == 0 && S_ISDIR(sb.st_mode));
}

//==========================================
// MAIN BOOTSTRAP
//==========================================

int execute_child(const char* path, char** argv)
{
	assert(path != NULL);
	assert(argv != NULL);

	pid_t pid = fork();
	if(pid == 0) {
		// Error
		int result = execv(path, argv);
		if(result == -1) {
			perror("unable to start child");		
		}
		exit(result);
	}
	else if(pid < 0) {
		perror("unable to start child");		
	}
	else {
		// Parent process
		int status;
		do {
	        pid_t wait_pid = waitpid(pid, &status, WUNTRACED);
	    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
	    return status;
	}

	return 1;
}

void run_shell(int argc, char** argv) {
	// No command. 
	if(argc < 1) return;

	const char* command = argv[0];

	// Leave if we exit lol
	if(STREQL(command, "exit")) {
		exit (EXIT_SUCCESS);
	}

	// Check file exists at path. 
	if(file_executable(command)) {
		execute_child(command, argv);
	}
	// Consult the path. 
	else {

		const char* real_system_path = getenv("PATH");
		const char* system_path = malloc(strlen(real_system_path));

		split_args path_args = split_into_args(system_path, ";");

		char* path = alloc_find_in_path(command, &path_args);
		if(path != NULL) {
			execute_child(path, argv);
			free(path);
		}

		free_cmd_args(&path_args);

		printf("%s: command not found\n", command);

		return;
	}
}

int main(int argc, char** argv) {
	prompt_carat = SH_CARAT; // Start as SH carat

    char *line = NULL;
	ssize_t read;
    size_t len = 0;
	// Prompt
    prompt_user();

	// Preimptive flush input
	fflush(stdin);

    while ((read = getline(&line, &len, stdin)) != -1) {

        // Parse line as a set of arguments.
        split_args args = split_into_args(line, " ");

        // Run with given args
        run_shell(args.argc, args.argv);

		// Free resources. 
		free_cmd_args(&args);
	    free(line);

		// Flush input buffer
		fflush(stdin);
		fflush(stdout);

		// Prompt
		prompt_user();
    }

    free(line);
	exit(EXIT_SUCCESS);
}