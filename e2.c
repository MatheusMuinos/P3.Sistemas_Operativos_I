#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>


static void manejador (int sig) {
    const char *msg;
    if (sig == SIGUSR1) {
        msg = "Señal recibida: SIGUSR1\n";
    } else if (sig == SIGUSR2) {
        msg = "Señal recibida: SIGUSR2\n";
    } else {
        msg = "Señal recibida: Desconocida\n";
    }
    write(STDOUT_FILENO, msg, strlen(msg)); // Uso de write en lugar de printf en un manejador de señales
}

int main (void) {
    struct sigaction sa; // Estructura para especificar la acción de la señal
    sa.sa_handler = manejador; // Asignar el manejador de señales
    sigemptyset(&sa.sa_mask); // Inicializar la máscara de señales
    sa.sa_flags = SA_RESTART; // Reiniciar llamadas al sistema interrumpidas

if (sigaction(SIGUSR1, &sa, NULL) == -1) { // Registrar el manejador para SIGUSR1
    perror("sigaction SIGUSR1");
    return 1;
}

if (sigaction(SIGUSR2, &sa, NULL) == -1) { // Registrar el manejador para SIGUSR2
    perror("sigaction SIGUSR2");
    return 1;
}

printf("PID del proceso: %ld\n", (long)getpid());
printf("En otra terminal, envía señales usando 'kill -SIGUSR1 %ld' o 'kill -SIGUSR2 %ld'\n",
       (long)getpid(), (long)getpid());

for(;;) {
    pause(); // Espera señales sin consumir CPU
}

}