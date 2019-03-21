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

bool file_executable(char* filename)
{
	return access(filename, X_OK) != -1;
}


// Try to find the program in our path. 
char* alloc_find_in_path(char* command, const split_args* args)
{
	for(int i = 0; i < args->argc; i++) {
		char* dir = args->argv[i];
		const int dir_len = strlen(dir);
		const int cmd_len = strlen(command);
		const int total_len = strlen(dir) + strlen(command) + 2; // +1 for '/' + for NULL

		char* composite_path = malloc(total_len);

		strcpy(composite_path, dir);
		composite_path[dir_len] = '/';
		strcpy(&composite_path[dir_len + 1], command);

		if(file_executable(composite_path)) {
			return composite_path;
		}
		else {
			free(composite_path);
		}
	}

	return NULL;
}

int count_num_tokens(char* buffer, char* delim)
{
	// Counting buffer. 
	char* counting_buffer = strdup(buffer);
	char* token = strtok(counting_buffer, delim);
	int argc = 0;

	// Count the number of tokens in string. 
	while(token) {
		argc++;
		token = strtok(NULL, delim);
	}
	free(counting_buffer);
	return argc;
}

// Parse a line into posix cmd args. 
split_args split_into_args(char* line, char* delim)
{
	// Precondition
	assert(line != NULL);
	assert(delim != NULL);

	// Copy the line of text. 
	char* buffer = strdup(line);
	int line_len = strlen(buffer);

	// destroy newline characters
	for(int i = 0; i < line_len; i++) {
		if(buffer[i] == '\n') {
			buffer[i] = delim[0];
		}
	}

	int argc = count_num_tokens(buffer, delim);

	// Add trailing NULL for free call.
	char **argv = malloc(sizeof(char*) * (argc + 1));

	// Record the tokens into argument array. 
	char* token = strtok(buffer, delim);
	int current_pos = 0;
	while(token)
	{
		argv[current_pos] = token;
		current_pos++;
		token = strtok(NULL, delim);
	}
	argv[argc] = NULL;

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

bool directory_exists(char* dirname)
{
	struct stat sb;
	return (stat(dirname, &sb) == 0 && S_ISDIR(sb.st_mode));
}

//==========================================
// MAIN BOOTSTRAP
//==========================================

int execute_child(char* path, char** argv)
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

	char* command = argv[0];
	assert(strlen(command) > 0);
	bool is_relative = (command[0] == '.');
	bool is_absolute = (command[0] == '/');

	// Leave if we exit lol
	if(STREQL(command, "exit")) {
		exit (EXIT_SUCCESS);
	}

	// Relative path. 
	if(is_relative || is_absolute) {
		// Check file exists at path. 
		if(file_executable(command)) {
			execute_child(command, argv);
		}
		else {
			printf("%s: command not found\n", command);
		}
	}
	// Not relative and not absolute? Consult the path. 
	else {
		char* real_system_path = getenv("PATH");
		char* system_path = strdup(real_system_path);

		split_args path_args = split_into_args(system_path, ":");

		char* path = alloc_find_in_path(command, &path_args);
		if(path != NULL) {
			execute_child(path, argv);
			free(path);
		}
		else {
			printf("%s: command not found\n", command);
		}

		free(system_path);
		free_cmd_args(&path_args);
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

		// Flush input buffer
		fflush(stdin);
		fflush(stdout);

		// Prompt
		prompt_user();
    }

    free(line);
	exit(EXIT_SUCCESS);
}