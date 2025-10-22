#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void gestion(int); 
int contador=0;
int main() {
    printf("\nMi pid es: %d\n",getpid());
    if (signal(SIGINT, gestion) == SIG_ERR) printf("Error al crear gestor 1\n");

    for(;;)
    if(contador==3){//cuando capturo 3 veces ctrl+C reseteo SIGINT para que sea una señal con su comportamiento predeterminado
        signal(SIGINT, SIG_DFL);
    }

    return 0;
}

static void gestion(int numero_de_senhal) { 
    switch (numero_de_senhal) {
        case SIGINT:
            printf("\nControl+C interceptado\n");
            contador++;
            break;
        default:
            printf("Señal desconocida recibida: %d\n", numero_de_senhal);
    }
}
