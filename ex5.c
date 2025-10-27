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
    if(h1!=0){
        int est;
        pid_t h2=fork();
        if(h2==0){//Hijo 2
            char op;
            printf("\nMatar a h1(S/N): ");scanf("%c",&op);//Pregunto para matar a hijo1
            if(op=='S'){
            kill(h1,SIGUSR1);
            }
            printf("\nMuriendo %d",getpid());
        }else{//Padre
          waitpid(h1, &est, 0);//Espera a que el hijo 1 muera
          printf("\nMuriendo %d\n",getpid());
        }
        
    }else{//Hijo1
        printf("\nEsperando a morir %d\n",getpid());
        for(;;);//bucle infinito para esperar a recibir señal de kill
        printf("\nNo se imprime esto");//No se imprime puesto que ya murió
    }

}

static void gestion(int numero_de_senhal) { 
    switch (numero_de_senhal) {
        case SIGUSR1:
         printf("\nMuriendo %d\n", getpid()); 
        exit(0);
        default:
            printf("Señal desconocida recibida: %d\n", numero_de_senhal);
    }
}
