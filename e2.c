#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdlib.h>

/*
 * logf: função auxiliar para imprimir mensagens com timestamp, PID e uma etiqueta (P, H1, N1).
 * Se usa para ver claramente o orden temporal e quem realiza cada ação.
 */
static void logf(const char *tag, const char *fmt, ...) {
    struct timeval tv; gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec; struct tm *tm = localtime(&t);
    char ts[32]; strftime(ts, sizeof(ts), "%H:%M:%S", tm);
    printf("[%s pid=%ld %s] ", ts, (long)getpid(), tag);
    va_list ap; va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    printf("\n");
}

/*
 * manejador: gestor de sinais para SIGUSR1 e SIGUSR2.
 * Apenas escreve mensagens para indicar qual sinal foi recebido.
 */
void manejador(int sig) {
    if (sig == SIGUSR1)
        printf("Sinal recebido: SIGUSR1\n");
    else if (sig == SIGUSR2)
        printf("Sinal recebido: SIGUSR2\n");
    else
        printf("Sinal recebido: desconhecido\n");
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    /*
     * Instala o manejador de sinais com sigaction.
     * SA_RESTART reintenta chamadas ao sistema interrompidas por sinais.
     */
    struct sigaction sa;
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = manejador;
    sa.sa_flags = SA_RESTART;

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);

    /*
     * Pai (P): bloqueia SIGUSR1 antes do fork.
     * Objetivo: quando H1 enviar SIGUSR1, o sinal ficará pendente e só será entregue
     * quando P decidir desbloqueá-lo.
     */
    sigset_t block;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    sigprocmask(SIG_BLOCK, &block, NULL);
    logf("P", "SIGUSR1 bloqueada; PID P=%ld", (long)getpid());

    /*
     * fork de H1:
     * - H1 enviará SIGUSR1 a P, criará N1 e esperará por ele.
     * - P dormirá, verificará se SIGUSR1 está pendente, desbloqueará e esperará H1.
     */
    pid_t h1 = fork();

    if (h1 == 0) {
        // ---- Filho (H1) ----
        sigprocmask(SIG_UNBLOCK, &block, NULL);
        logf("H1", "PID H1=%ld; SIGUSR1 desbloqueada em H1", (long)getpid());

        // H1 envia SIGUSR1 ao pai (P)
        pid_t pid_pai = getppid();
        logf("H1", "Enviando SIGUSR1 a P=%ld", (long)pid_pai);
        kill(pid_pai, SIGUSR1);

        // Cria o neto (N1)
        pid_t n1 = fork();
        if (n1 == 0) {
            // ---- Neto (N1) ----
            logf("N1", "PID N1=%ld; enviando SIGUSR2 a P=%ld e dormindo 5s",
                 (long)getpid(), (long)pid_pai);
            kill(pid_pai, SIGUSR2);
            sleep(5);
            logf("N1", "Terminando execução");
            return 42;
        }

        // H1 espera N1 terminar
        logf("H1", "Esperando N1=%ld terminar", (long)n1);
        wait(NULL);
        logf("H1", "N1 terminou; H1 saindo");
        return 21;
    }

    // ---- Pai (P) ----
    logf("P", "H1 criado com PID=%ld", (long)h1);
    logf("P", "Dormindo 3s enquanto SIGUSR1 permanece bloqueada");
    sleep(3);

    /*
     * Antes de desbloquear, P verifica se SIGUSR1 está pendente.
     * Se sim, significa que H1 já enviou o sinal enquanto estava bloqueado.
     */
    sigset_t pending;
    sigpending(&pending);
    if (sigismember(&pending, SIGUSR1))
        logf("P", "SIGUSR1 está PENDENTE");
    else
        logf("P", "SIGUSR1 não está pendente");

    /*
     * Ao desbloquear SIGUSR1, o kernel entrega o sinal imediatamente se ele estiver pendente.
     */
    logf("P", "Desbloqueando SIGUSR1");
    sigprocmask(SIG_UNBLOCK, &block, NULL);

    // P espera H1 terminar
    logf("P", "Esperando H1=%ld", (long)h1);
    wait(NULL);
    logf("P", "H1 terminou; encerrando programa");

    return 0;
}
