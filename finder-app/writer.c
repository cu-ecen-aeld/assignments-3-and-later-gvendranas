#include <stdio.h>
#include <syslog.h>

int main(int argc, char *argv[]) {

    // First of all let's open syslog
    openlog(argv[0], LOG_PID | LOG_CONS, LOG_USER);

    // Check if user has entered the right number of arguments
    if (argc != 3) {
        syslog(LOG_ERR, "%s <filename> <text_to_be_added>\n", argv[0]);
        return 1;
    }

    // Get the file name from the first command-line argument
    const char *filename = argv[1];
    // Open the specified file
    FILE *file = fopen(filename, "w");
    // Check if file exists, return 1 if not
    if (file == NULL) {
        //perror("Error opening file");
        syslog(LOG_ERR, "Error opening file");
        return 1;
    }

    // Get the text from the second command-line argument
    const char *text = argv[2];

    // Write input text to the specified file
    syslog(LOG_DEBUG, "Writing %s to %s", text, filename);
    if (fprintf(file, "%s\n", text) < 0) {
        syslog(LOG_ERR, "Could not write into file");
        fclose(file);
        return 1;
    }

    // Close file
    fclose(file);
    return 0;
}