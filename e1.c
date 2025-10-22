#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

//Programa realizado por Matheus Muiños Kruschewsky y Hugo Veiga Couselo

static void gestion(int); 
double n = 0;

int main() {
    printf("\nMi pid es: %d\n", getpid());

    // Registrar manejadores de señales
    if (signal(SIGUSR1, gestion) == SIG_ERR) printf("Error al crear gestor SIGUSR1\n");
    if (signal(SIGALRM, gestion) == SIG_ERR) printf("Error al crear gestor SIGALRM\n");

    alarm(1);//Programa una SIGALRM en 1 segundo, si fuese alarm(x) en x segundos 

    for (int i = 0;; i++) {
        n = pow(i, 0.5);
        sleep(1); 
    }

    return 0;
}

static void gestion(int numero_de_senhal) {
    switch (numero_de_senhal) {
        case SIGUSR1:
            printf("\nLa señal me saca %d\n", getpid());
            exit(0);
            break;
        case SIGALRM:
            printf("\nEL número actual es: %lf y mi pid es: %d \n", n,getpid());//imprimimos pid tambien para terminar con kill
            alarm(1);//Vuelve a mandar una señal en 1 segundo
            break;
        default:
            printf("Señal desconocida recibida: %d\n", numero_de_senhal);
    }
}