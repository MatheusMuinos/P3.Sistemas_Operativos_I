#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


static void manejador (int sig) {
    const char *msg;
    if (sig == SIGUSR1) {
        msg = "Señal recibida: SIGUSR1\n";
    } else if (sig == SIGUSR2) {
        msg = "Señal recibida: SIGUSR2\n";
    } else {
        msg = "Señal recibida: Desconocida\n";
    }
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main (void) {
    struct sigaction sa;
    sa.sa_handler = manejador;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction SIGUSR1");
        return 1;
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("sigaction SIGUSR2");
        return 1;
    }

    // Bloquear SIGUSR1 antes de fork para garantizar que P la tenga bloqueada
    sigset_t block;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &block, NULL) == -1) {
        perror("sigprocmask BLOCK");
        return 1;
    }

    pid_t h1 = fork();
    if (h1 == -1) {
        perror("fork H1");
        return 1;
    }

    if (h1 == 0) {
        // Hijo H1: desbloquear SIGUSR1 (no la necesita bloqueada)
        if (sigprocmask(SIG_UNBLOCK, &block, NULL) == -1) {
            perror("H1: sigprocmask UNBLOCK");
            _exit(102);
        }

        pid_t pid_abuelo = getppid();           // PID del abuelo (P)
        if (kill(pid_abuelo, SIGUSR1) == -1) {  // H1 envía SIGUSR1 a P
            perror("H1: kill SIGUSR1 a P");
        }

        pid_t n1 = fork();
        if (n1 == -1) {
            perror("fork N1");
            _exit(100);
        }
        if (n1 == 0) {
            // Nieto N1: envía SIGUSR2 al abuelo, espera 5s y termina
            if (kill(pid_abuelo, SIGUSR2) == -1) {
                perror("N1: kill SIGUSR2 a P");
            }
            sleep(5);
            _exit(42); // Código único para N1
        }

        // H1 espera a N1
        int status_n1 = 0;
        if (waitpid(n1, &status_n1, 0) == -1) {
            perror("waitpid N1");
            _exit(101);
        }
        if (WIFEXITED(status_n1)) {
            printf("H1: N1 terminó con código %d\n", WEXITSTATUS(status_n1));
        } else if (WIFSIGNALED(status_n1)) {
            printf("H1: N1 terminó por señal %d\n", WTERMSIG(status_n1));
        }
        _exit(21); // Código único para H1
    }

    // Padre P
    printf("PID del proceso P: %ld\n", (long)getpid());
    printf("PID del hijo H1: %ld\n", (long)h1);
    printf("P: SIGUSR1 bloqueada durante 3s...\n");
    sleep(3);

    // Desbloquear SIGUSR1: si estaba pendiente, se entrega ahora
    if (sigprocmask(SIG_UNBLOCK, &block, NULL) == -1) {
        perror("P: sigprocmask UNBLOCK");
        return 1;
    }
    printf("P: SIGUSR1 desbloqueada\n"); // La orden puede variar con el SIGUSR1 caso el ya estuviera pendiente

    int status_h1 = 0;
    if (waitpid(h1, &status_h1, 0) == -1) {
        perror("waitpid H1");
        return 1;
    }
    if (WIFEXITED(status_h1)) {
        printf("P: H1 terminó con código %d\n", WEXITSTATUS(status_h1));
    } else if (WIFSIGNALED(status_h1)) {
        printf("P: H1 terminó por señal %d\n", WTERMSIG(status_h1));
    }

    printf("En otra terminal, envía señales usando 'kill -SIGUSR1 %ld' o 'kill -SIGUSR2 %ld'\n",
           (long)getpid(), (long)getpid());

    for(;;) {
        pause();
    }
}