/* Incluye las librerías necesarias */
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void gestion(int); /* Declaración de la función de gestión de señales recibidas */

int main() {
    /* Declara las variables necesarias */
    pid_t padre, hijo;
    //llamar a signal para capturar señales de diferentes tipos
    if (signal(SIGUSR1, gestion) == SIG_ERR) printf("Error al crear gestor 1\n");
    if (signal(SIGUSR2, gestion) == SIG_ERR) printf("Error al crear gestor 2\n");
    if (signal(SIGTERM, gestion) == SIG_ERR) printf("Error al crear gestor TERM\n");

    padre = getpid();

    if ((hijo = fork()) == 0) { /* Trabajo del hijo */
        kill(padre, SIGUSR1); /* Envía señal al padre */
        for (;;); /* Espera señal del padre indefinidamente */
    } else { /* Trabajo del padre */
        sleep(1); // Espera breve para asegurar que el hijo envió SIGUSR1
        kill(hijo, SIGUSR2); // Envía señal tipo 2 al hijo
        sleep(1); 
        kill(hijo,SIGUSR1);
        sleep(1);
        kill(hijo, SIGUSR2); // Envía señal tipo 2 al hijo
        sleep(1); 
        kill(hijo,SIGUSR1);
        sleep(1);
        kill(hijo, SIGUSR2); // Envía señal tipo 2 al hijo
        sleep(1); 
        kill(hijo,SIGUSR1);
        sleep(1); // Espera breve para que el hijo procese la señal
        kill(hijo, SIGTERM);  // Termina al hijo
        wait(NULL);           // Espera a que el hijo termine
        printf("El hijo ha terminado.\n");
    }

    return 0;
}

static void gestion(int numero_de_senhal) { /* Función de gestión de señales */
    switch (numero_de_senhal) {
        case SIGUSR1:
            printf("Señal tipo 1 recibida. Soy %d\n", getpid());
            break;
        case SIGUSR2:
            printf("Señal tipo 2 recibida. Soy %d\n", getpid());
            break;
        case SIGTERM:
            printf("Señal de terminación recibida. Soy %d\n", getpid());
            exit(0); // Termina el proceso
            break;
        default:
            printf("Señal desconocida recibida: %d\n", numero_de_senhal);
    }
}
