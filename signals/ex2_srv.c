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

void srv_hendler(int);
void rcv_from_client(int *arg1, int *arg2, int *arg3, int *arg4);
int read_line(int file_descriptor, char *buffer, int max_size);
void send_toClient(int value, int pid);
void remove_file(const char* filename);
void create_temp_file(int pid);
char template[30] = "to_client_xxxxxx";
const char* to_srv = "to_srv.txt";

int main()
{
    signal(SIGUSR1, srv_hendler);
    while (true){
        pause();
    }

    return 0;
}

void srv_hendler(int sig)
{
    signal(SIGUSR1, srv_hendler);
    printf("[SERVER]: Heres Server, your signal has recieved...\n");
    // Create a child process
    int err = 0;
    pid_t pid = fork();
    if (pid == 0) {
        int P1 = 0, P2 = 0, P3 = 0, P4 = 0, solotion;
        rcv_from_client(&P1, &P2, &P3, &P4);
        //printf("[SERVER]: Server has recieved from client: PID = %d, agr1 = %d, Invoice operation = #%d, arg2 = %d\n", P1, P2, P3, P4);
        
        remove_file(to_srv);

        switch (P3){
            case 1: // +
                solotion = P2 + P4;
                break;
            case 2: // -
                solotion = P2 - P4;
                break;
            case 3: // *
                solotion = P2 * P4;
                break;
            case 4: // :
                if (P3 == 4 && P4 == 0){
                    perror("[SERVER]: Cannot devied by 0\n");
                    err = 1;
                    break;
	        }
	        solotion = P2 / P4;
                break;
            default:
                perror("[SERVER]: error in receving 2end argument");
                break;
        }
        if (err == 0){
            send_toClient(solotion, P1);
            kill(P1, SIGUSR2);
        }else{
            perror("[SERVER]: error sugnal sent");
            kill(P1, err);
        }
    }
    else {
	    // This is the parent process: return to waiting for additional signals from other clients
        return;
    }
}

// Call the rcv_from_client function and store the results in the variables
void rcv_from_client(int *arg1, int *arg2, int *arg3, int *arg4){
    // Open the file "to_srv.txt" in read mode
    int file_descriptor = open("to_srv.txt", O_RDONLY);
    
    // Check if the file was successfully opened
    if (file_descriptor == -1) {
        perror("[SERVER]: Error opening file\n");
        exit(EXIT_FAILURE);
    }
    
    // Read each line from the file and store it in a different variable
    char buffer[16];
    int n = read_line(file_descriptor, buffer, sizeof(buffer));
    if (n == -1) {
        perror("[SERVER]: Error reading from file\n");
        exit(EXIT_FAILURE);
    }
    *arg1 = atoi(buffer);
    
    n = read_line(file_descriptor, buffer, sizeof(buffer));
    if (n == -1) {
        perror("[SERVER]: Error reading from file\n");
        exit(EXIT_FAILURE);
    }
    *arg2 = atoi(buffer);
    
    n = read_line(file_descriptor, buffer, sizeof(buffer));
    if (n == -1) {
        perror("[SERVER]: Error reading from file\n");
        exit(EXIT_FAILURE);
    }
    *arg3 = atoi(buffer);

    
    n = read_line(file_descriptor, buffer, sizeof(buffer));
    if (n == -1) {
        perror("[SERVER]: Error reading from file\n");
        exit(EXIT_FAILURE);
    }
    *arg4 = atoi(buffer);
    
    // Close the file
    close(file_descriptor);
}

// Helper function to read a line from a file
int read_line(int file_descriptor, char *buffer, int max_size) {
    int i = 0;
    char c;
    
    while (read(file_descriptor, &c, 1) == 1 && c != '\n' && i < max_size - 1) {
        buffer[i++] = c;
    }
    
    if (c == '\n') {
        buffer[i++] = c;
    }
    
    buffer[i] = '\0';
    
    return i;
}

void send_toClient(int value, int pid) {
    
    // Open the file for writing
    create_temp_file(pid);
    int fd = open(template, O_WRONLY | O_CREAT | O_EXCL, 0666);

    // Check if the file was successfully opened
    if (fd < 0) {
        perror("[SERVER]: Error opening file");
        return;
    }

    // Convert the integer to a string
    char value_str[16];
    sprintf(value_str, "%d", value);

    // Write the string to the file
    ssize_t num_written = write(fd, value_str, strlen(value_str));

    // Check if the write was successful
    if (num_written < 0) {
        perror("[SERVER]: Error writing to file");
    }

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
}

void create_temp_file(int pid) {
    // Use snprintf to generate a unique filename based on the template and the PID
    snprintf(template, strlen(template), "to_client_%d", pid);

    // Use mkstemp to create a unique temporary file with the generated name
    return;
}
