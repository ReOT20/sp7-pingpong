#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

extern int sigpause(int mask);

static volatile int ball_received = 0;
static int other_pid = 0;

void handler(int sig) {
    (void)sig; 
    ball_received = 1;
}

void play(const char *send_msg, const char *recv_msg, int rounds, int old_mask) {
    for (int i = 0; i < rounds; i++) {
        while (!ball_received) {
            sigpause(old_mask);
        }
        
        ball_received = 0;
        printf("proc[%d]: %s received\n", getpid(), recv_msg);
        
        usleep(100000); 
        
        printf("proc[%d]: %s send\n", getpid(), send_msg);
        kill(other_pid, SIGUSR1);
    }
}

int main(int argc, char *argv[]) {
    int rounds = 5;
    if (argc > 1) {
        rounds = atoi(argv[1]);
    }

    signal(SIGUSR1, handler);

    int block_mask = sigmask(SIGUSR1);
    
    int old_mask = sigblock(block_mask); 

    int pid = fork();
    
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    
    if (pid == 0) {
        other_pid = getppid();
        play("pong", "ping", rounds, old_mask);
        exit(0);
    } else {
        other_pid = pid;
        usleep(50000);
        
        printf("proc[%d]: ping send\n", getpid());
        kill(other_pid, SIGUSR1);
        
        play("ping", "pong", rounds, old_mask);
        
        wait(0);
    }
    
    return 0;
}
