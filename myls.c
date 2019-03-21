#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#define STREQL(x, y) (strcmp(x, y) == 0)

void print_usage()
{
    printf("%s\n", "Welcome to myls! It prints things from directories! (well enough)");
    printf("%s\n", "\t-a: print hidden files.");
}

int main(int argc, char **argv) {

    bool should_show_hidden = false;

    // nope. 
    if(argc > 2) {
        print_usage();
        return -1;
    }
    else if(argc == 2) {
        // If we havd -a, 
        if(STREQL(argv[1], "-a")){
            should_show_hidden = true;
        }
        // nope again. 
        else{
            print_usage();
            return -1;
        }
    }
    // not sure why this would happen, but still nope. 
    else if(argc < 1) {
        print_usage();
        return -1;
    }

    // Open the directory. 
    DIR *directory = opendir("./");

    // Check for errors. 
    if (directory != NULL) {
        // Loop until we don't have any more entries. 
        struct dirent *entry;
        while (entry = readdir(directory)) {
            const char *filename = entry->d_name;
            assert(strlen(filename) != 0);
            // Check for hidden files. 
            // Cover the case of: 
            //  '.'
            //  '..'
            //  hidden files
            if(!should_show_hidden && (filename[0] == '.')) continue;
            // Print it out. 
            printf("%s ", entry->d_name);
        }
        printf("\n");
        // Close it down.
        closedir(directory);
    }
    else {          
        perror ("Couldn't open the directory");
    }

    return 0;
}