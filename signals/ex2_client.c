#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <math.h>

void client_hendler(int);
void err_hendler(int);
void send_toSerever(char* argv2, char* argv3, char* argv4, char *s_client_pid);
void print_number_from_file(const char *filename);
void remove_file(const char* filename);
char template[30] = "to_client_xxxxxx";

int main(int argc, char *argv[]){
    // Check that there are exactly four arguments
    if (argc != 5) {
        perror("[CLIENT]: Error: expected 4 arguments\n");
        return 1;
    }

    //Define signal hendlers
    signal(SIGUSR2, client_hendler);
    signal(1, err_hendler);    
    
    pid_t client_pid = getpid();
    //Convert PID into a string
    int length = snprintf( NULL, 0, "%d", client_pid );
    char* s_client_pid = malloc( length + 1 );
    snprintf( s_client_pid, length + 1, "%d", client_pid );
    
    
    // Call the send_toSerever function with the arguments from argv
    send_toSerever(argv[2], argv[3], argv[4], s_client_pid);
    
    // Free mallocated string
    free(s_client_pid);

    // Send server signal, after "to_srv.txt" file is ready
    kill(atoi(argv[1]), SIGUSR1);

    // Wait for signal from server after finishing calculation
    pause();
    
    return 0;
}

void client_hendler(int sig)
{
    signal(SIGUSR2, client_hendler);
    
    // Use snprintf to generate a unique filename based on the template and the PID
    snprintf(template, strlen(template), "to_client_%d", getpid());
    
    print_number_from_file(template);
    
    remove_file(template);
}

void err_hendler(int sig){
    signal(1, err_hendler);
    perror("[CLIENT]: Div by 0 error signal was sent...\n");
    exit(EXIT_FAILURE);
}

void send_toSerever(char* argv2, char* argv3, char* argv4, char* s_client_pid) {
    int fd, retry = 0;

    for (int i = 1; i <= 4; i++){
        fd = open("to_srv.txt", O_WRONLY | O_CREAT | O_EXCL, 0666);
        if (fd < 0){
            sleep(1 + rand() % 4);
        }else{
            break;
        }
        if (i == 4){
            perror("[CLIENT]: Error: Could not create file after 4 retries\n");
            exit(1);
        }
    }

    char buffer[1024];
    sprintf(buffer, "%s\n%s\n%s\n%s", s_client_pid, argv2, argv3, argv4);
    write(fd, buffer, strlen(buffer));
    close(fd);
}

void print_number_from_file(const char *filename) {
    // Open the file
    int fd = open(filename, O_RDONLY);

    // Check if the file was opened successfully
    if (fd < 0) {
        perror("Error opening file");
        return;
    }

    // Read the number from the file
    char buffer[16];
    ssize_t num_bytes_read = read(fd, buffer, sizeof(buffer));

    // Check if the read was successful
    if (num_bytes_read < 0) {
        perror("Error reading from file");
        close(fd);
        return;
    }

    // Convert the string read from the file to an integer
    int number = atoi(buffer);

    // Print the number to the screen
    printf("[CLIENT]: The solotion is: %d\n", number);

    // Close the file
    close(fd);
}

void remove_file(const char* filename) {
    // Create a child process
    pid_t pid = fork();
    if (pid == 0) {
        // This is the child process. Set up the arguments for the "rm" command.
    	char* argv[] = {"rm", (char*)filename, NULL};

    	// Execute the "rm" command
   	 execvp("rm", argv);
    } else {
	     // This is the parent process. Wait for the child process to finish.
	     int status;
	     waitpid(pid, &status, 0);
        }
    return;
}
