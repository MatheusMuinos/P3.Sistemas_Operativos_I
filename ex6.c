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
    if (signal(SIGUSR1, gestion) == SIG_ERR) printf("Error al crear gestor 1\n");

    pid_t h1=fork();
    if(h1==0){
        pause();//espera a una señal
        printf("\nAcabo la espera");//ver que acabó la espera
    }else{
        sleep(5);//espera para ver que pause funciona
        kill(h1,SIGUSR1);//mandar señal
    }

}

static void gestion(int numero_de_senhal) { 
    switch (numero_de_senhal) {
        case SIGUSR1:
        printf("\nLa señal me saca %d\n", getpid()); 
        default:
            printf("Señal desconocida recibida: %d\n", numero_de_senhal);
    }
}
