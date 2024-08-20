#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <poll.h>

#define BLOCK_SIZE 4096
#define IPC_RESULT_ERROR (-1)
#define FILENAME "chat.c"

#define SEM_PRODUCER1_NAME "/myproducer1"
#define SEM_CONSUMER1_NAME "/myconsumer1"
#define SEM_PRODUCER2_NAME "/myproducer2"
#define SEM_CONSUMER2_NAME "/myconsumer2"

static int get_shared_block(char *filename, int size) {
    key_t key = ftok(filename, 0);
    if (key == IPC_RESULT_ERROR) {
        return IPC_RESULT_ERROR;
    }
    return shmget(key, size, 0644 | IPC_CREAT);
}

char *attach_memory_block(char *filename, int size) {
    int shared_block_id = get_shared_block(filename, size);
    char *result = shmat(shared_block_id, NULL, 0);
    if (result == (char *)IPC_RESULT_ERROR) {
        return NULL;
    }
    return result;
}

bool detach_memory_block(char *block) {
    return (shmdt(block) != IPC_RESULT_ERROR);
}

void set_nonblocking_input() {
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag &= ~(ICANON | ECHO);
    ttystate.c_cc[VMIN] = 1;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <1 or 2>\n", argv[0]);
        return 1;
    }

    int user = atoi(argv[1]);
    if (user != 1 && user != 2) {
        printf("Please specify 1 or 2 as the argument.\n");
        return 1;
    }

    char *block = attach_memory_block(FILENAME, BLOCK_SIZE * 2);
    if (block == NULL) {
        printf("Error! Unable to get shm block!\n");
        return -1;
    }

    sem_t *sem_prod1 = sem_open(SEM_PRODUCER1_NAME, O_CREAT, 0660, 0);
    sem_t *sem_cons1 = sem_open(SEM_CONSUMER1_NAME, O_CREAT, 0660, 1);
    sem_t *sem_prod2 = sem_open(SEM_PRODUCER2_NAME, O_CREAT, 0660, 0);
    sem_t *sem_cons2 = sem_open(SEM_CONSUMER2_NAME, O_CREAT, 0660, 1);

    char *write_block = (user == 1) ? block : block + BLOCK_SIZE;
    char *read_block = (user == 1) ? block + BLOCK_SIZE : block;
    sem_t *sem_prod = (user == 1) ? sem_prod1 : sem_prod2;
    sem_t *sem_cons = (user == 1) ? sem_cons1 : sem_cons2;
    sem_t *sem_prod_other = (user == 1) ? sem_prod2 : sem_prod1;
    sem_t *sem_cons_other = (user == 1) ? sem_cons2 : sem_cons1;

    printf("Chat started. Type 'quit' to exit.\n");

    set_nonblocking_input();

    char input[BLOCK_SIZE] = {0};
    int input_pos = 0;
    bool should_quit = false;

    while (!should_quit) {
        struct pollfd fds[1];
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;

        int ret = poll(fds, 1, 100);  // Wait for 100ms

        if (ret > 0 && (fds[0].revents & POLLIN)) {
            char c;
            if (read(STDIN_FILENO, &c, 1) > 0) {
                if (c == '\n') {
                    input[input_pos] = '\0';
                    printf("\nYou: %s\n", input);

                    sem_wait(sem_cons);
                    strncpy(write_block, input, BLOCK_SIZE);
                    sem_post(sem_prod);

                    if (strcmp(input, "quit") == 0) {
                        should_quit = true;
                    }

                    input_pos = 0;
                    memset(input, 0, BLOCK_SIZE);
                } else if (input_pos < BLOCK_SIZE - 1) {
                    input[input_pos++] = c;
                    printf("%c", c);
                    fflush(stdout);
                }
            }
        }

        int sem_value;
        sem_getvalue(sem_prod_other, &sem_value);
        if (sem_value > 0) {
            sem_wait(sem_prod_other);
            printf("\nOther: %s\n", read_block);
            if (strcmp(read_block, "quit") == 0) {
                should_quit = true;
            }
            read_block[0] = 0;
            sem_post(sem_cons_other);
        }
    }

    // Reset terminal settings
    struct termios ttystate;
    tcgetattr(STDIN_FILENO, &ttystate);
    ttystate.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

    sem_close(sem_prod1);
    sem_close(sem_cons1);
    sem_close(sem_prod2);
    sem_close(sem_cons2);
    detach_memory_block(block);

    printf("\nChat ended.\n");
    return 0;
}
