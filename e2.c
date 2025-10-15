#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <errno.h>

/*
 * logf: función auxiliar para imprimir mensajes con timestamp, PID y una etiqueta (P, H1, N1).
 * Se usa para ver claramente el orden temporal y quién realiza cada acción.
 */
static void logf(const char *tag, const char *fmt, ...) {
    struct timeval tv; gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec; struct tm *tm = localtime(&t);
    char ts[32]; strftime(ts, sizeof(ts), "%H:%M:%S", tm);
    char prefix[128];
    snprintf(prefix, sizeof(prefix), "[%s.%03ld pid=%ld %s] ",
             ts, (long)(tv.tv_usec/1000), (long)getpid(), tag);
    char msg[512];
    va_list ap; va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);
    printf("%s%s\n", prefix, msg);
    fflush(stdout);
}

/*
 * sleep_ms: duerme la cantidad indicada en milisegundos.
 * Reintenta en caso de que nanosleep sea interrumpido por una señal (EINTR).
 */
static void sleep_ms(long ms) {
    struct timespec req = { ms/1000, (ms%1000)*1000000L }, rem;
    while (nanosleep(&req, &rem) == -1 && errno == EINTR) req = rem;
}

/*
 * manejador: gestor de señales para SIGUSR1 y SIGUSR2.
 * Solo imprime qué señal se recibió. Se usa write (async-signal-safe) y no printf.
 */
static void manejador (int sig) {
    const char *msg;
    if (sig == SIGUSR1) {
        msg = "Señal recibida: SIGUSR1\n";
    } else if (sig == SIGUSR2) {
        msg = "Señal recibida: SIGUSR2\n";
    } else {
        msg = "Señal recibida: Desconocida\n";
    }
    // write es seguro dentro de un manejador de señales
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main (void) {
    // Desactiva el buffer de stdout para que los logs salgan inmediatamente
    setvbuf(stdout, NULL, _IONBF, 0);

    /*
     * Instala el manejador de señales con sigaction.
     * SA_RESTART reintenta llamadas al sistema interrumpidas por señales cuando es posible.
     */
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

    /*
     * Padre P: bloquea SIGUSR1 ANTES del fork.
     * Objetivo: que cuando H1 envíe SIGUSR1, esta quede pendiente en P y no se entregue
     * hasta que P decida desbloquearla más tarde.
     */
    sigset_t block;
    sigemptyset(&block);
    sigaddset(&block, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &block, NULL) == -1) {
        perror("sigprocmask BLOCK");
        return 1;
    }
    logf("P", "SIGUSR1 bloqueada");
    logf("P", "PID P=%ld", (long)getpid());

    /*
     * fork de H1:
     * - Proceso hijo (H1): enviará SIGUSR1 a P, creará N1 y esperará a N1.
     * - Proceso padre (P): dormirá un tiempo, comprobará que SIGUSR1 está pendiente,
     *   luego la desbloqueará y esperará a H1.
     */
    pid_t h1 = fork();
    if (h1 == -1) {
        perror("fork H1");
        return 1;
    }

    if (h1 == 0) {
        // ---- Rama del HIJO H1 ----

        // En H1 no necesitamos SIGUSR1 bloqueada; la desbloqueamos por claridad/herencia.
        if (sigprocmask(SIG_UNBLOCK, &block, NULL) == -1) {
            perror("H1: sigprocmask UNBLOCK");
            _exit(102);
        }
        logf("H1", "PID H1=%ld; SIGUSR1 desbloqueada en H1", (long)getpid());

        // H1 obtiene el PID del abuelo (que es P) y le envía SIGUSR1.
        pid_t pid_abuelo = getppid(); // P
        logf("H1", "Enviando SIGUSR1 a P=%ld", (long)pid_abuelo);
        if (kill(pid_abuelo, SIGUSR1) == -1) {
            perror("H1: kill SIGUSR1 a P");
        }

        // H1 crea al nieto N1
        pid_t n1 = fork();
        if (n1 == -1) {
            perror("fork N1");
            _exit(100);
        }
        if (n1 == 0) {
            // ---- Rama del NIETO N1 ----
            // N1 envía SIGUSR2 al abuelo (P), luego duerme 5 segundos y termina con código 42.
            logf("N1", "PID N1=%ld; enviando SIGUSR2 a abuelo P=%ld y durmiendo 5s",
                 (long)getpid(), (long)pid_abuelo);
            if (kill(pid_abuelo, SIGUSR2) == -1) {
                perror("N1: kill SIGUSR2 a P");
            }
            sleep_ms(5000);
            logf("N1", "Saliendo con código 42");
            _exit(42);
        }

        // H1 espera a que termine N1 y reporta su causa de terminación
        logf("H1", "Esperando a N1=%ld", (long)n1);
        int status_n1 = 0;
        if (waitpid(n1, &status_n1, 0) == -1) {
            perror("waitpid N1");
            _exit(101);
        }
        if (WIFEXITED(status_n1)) {
            logf("H1", "N1 terminó con código %d", WEXITSTATUS(status_n1));
        } else if (WIFSIGNALED(status_n1)) {
            logf("H1", "N1 terminó por señal %d", WTERMSIG(status_n1));
        }

        // H1 finaliza con un código distinto (21) para que el padre lo distinga
        logf("H1", "Saliendo con código 21");
        _exit(21);
    }

    // ---- Rama del PADRE P ----
    logf("P", "H1 creado con PID=%ld", (long)h1);

    /*
     * P duerme ~3s con SIGUSR1 bloqueada.
     * Durante este tiempo:
     *  - Puede llegar SIGUSR2 y su manejador se ejecutará inmediatamente (no está bloqueada).
     *  - SIGUSR1 puede llegar, pero quedará PENDIENTE por estar bloqueada.
     */
    logf("P", "Duermo ~3s para que N1 envíe SIGUSR2 mientras SIGUSR1 permanece bloqueada");
    sleep_ms(3000);

    /*
     * Antes de desbloquear, P comprueba si SIGUSR1 está pendiente usando sigpending+sigismember.
     * Si está pendiente, significa que H1 ya la envió mientras estaba bloqueada.
     */
    sigset_t pending;
    if (sigpending(&pending) == -1) {
        perror("sigpending");
        return 1;
    }
    int is_pend = sigismember(&pending, SIGUSR1);
    if (is_pend == 1) {
        logf("P", "SIGUSR1 está PENDIENTE (bloqueada y recibida)");
    } else if (is_pend == 0) {
        logf("P", "SIGUSR1 NO está pendiente (aún no recibida)");
    } else {
        perror("sigismember");
        return 1;
    }

    /*
     * P desbloquea SIGUSR1. Si estaba pendiente, el kernel entrega la señal ahora
     * y se ejecuta el manejador, por lo que se verá el mensaje de SIGUSR1 después de SIGUSR2,
     * aunque SIGUSR1 pudiera haber llegado antes.
     */
    logf("P", "Desbloqueando SIGUSR1 (si estaba pendiente, se entregará ahora)");
    if (sigprocmask(SIG_UNBLOCK, &block, NULL) == -1) {
        perror("P: sigprocmask UNBLOCK");
        return 1;
    }
    logf("P", "SIGUSR1 desbloqueada");

    // P espera al hijo H1 y muestra su forma de terminación (debería ser exit con código 21)
    int status_h1 = 0;
    logf("P", "Esperando a H1=%ld", (long)h1);
    if (waitpid(h1, &status_h1, 0) == -1) {
        perror("waitpid H1");
        return 1;
    }
    if (WIFEXITED(status_h1)) {
        logf("P", "H1 terminó con código %d", WEXITSTATUS(status_h1));
    } else if (WIFSIGNALED(status_h1)) {
        logf("P", "H1 terminó por señal %d", WTERMSIG(status_h1));
    }

    // El proceso P queda vivo esperando señales extra que el usuario quiera enviar
    logf("P", "Puedes enviar señales extra: kill -SIGUSR1 %ld | kill -SIGUSR2 %ld",
         (long)getpid(), (long)getpid());

    for(;;) pause();
}