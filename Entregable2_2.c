#define _POSIX_C_SOURCE 200809L
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
#include <stdlib.h>

//Programa realizado por Matheus Muiños Kruschewsky y Hugo Veiga Couselo


void gestion(int sig){
    if (sig == SIGUSR1) {
        printf("\nSeñal recibida: SIGUSR1, %d\n",getpid());
    } else if (sig == SIGUSR2) {
        printf("\nSeñal recibida: SIGUSR2 %d\n",getpid());
    } else {
        printf("\nSeñal recibida: Desconocida\n");
    }
}

int main(){
    //Inicialización de variables de tiempo
    struct timeval tiempo;
    gettimeofday(&tiempo,NULL);
    time_t t=time(NULL);
    struct tm *info = localtime(&t);
    //Inicialización de la estructura sa usando el gestor "gestion"
    struct sigaction sa;
    sa.sa_handler = gestion;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    //Inicializamos estructura de bloqueo y pendientes
    sigset_t block, pendientes;
    sigemptyset(&block);
    //Añadimos SIGUSR1 a la lista de señales bloqueadas
    sigaddset(&block, SIGUSR1);
    //bloqueamos
    sigprocmask(SIG_BLOCK, &block, NULL);

    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("sigaction SIGUSR2");
        return 1;
    }
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("sigaction SIGUSR1");
        return 1;
    }

    info=localtime(&t);
    gettimeofday(&tiempo,NULL);
    printf("\nSoy el padre y voy a bloquear SIGUSR1, mi pi y mi hora es: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
    

    t=time(NULL);
    info=localtime(&t);
    gettimeofday(&tiempo,NULL);
    printf("\nSoy el padre y voy a crear un hijo, mi pi y mi hora es: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
    sleep(1);
    //conseguimos pid padre y creamos al hijo 1
    pid_t pidpadre=getpid();
    pid_t h1=fork();
    
    if(h1==0){//parte de hijo 1
        pid_t n1=fork();//creamos al nieto
        if(n1!=0){//Hijo 1
            int estn1;


            t=time(NULL);
            info=localtime(&t);
            gettimeofday(&tiempo,NULL);
            printf("\nSoy el hijo y voy a mandar SIGUSR1 a mi padre, mi numero y mi hora son: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
            
            //mandamos SIGUSR1 al padre por eso usamos getppid en la seccion del hijo
            kill(getppid(),SIGUSR1);


            t=time(NULL);
            info=localtime(&t);
            gettimeofday(&tiempo,NULL);
            printf("\nSoy el hijo y espero a que muera el nieto 1, mi pid y mi hora son: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
            
            //esperamos a que muera el nieto
            waitpid(n1,&estn1,0);


            t=time(NULL);
            info=localtime(&t);
            gettimeofday(&tiempo,NULL);
            printf("\nSoy el hijo y me voy a morir, mi pid y mi hora son: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
            
            //Muere
            exit(11);

        }else{//Nieto 1
            //Si duerme 3 segundos veremos que del sleep(100) se dormiran aproximadamente 4, si fuese sleep(n) el padre dormiria n+1 s.
            //sleep(3);


            t=time(NULL);
            info=localtime(&t);
            gettimeofday(&tiempo,NULL);
            printf("\nSoy el nieto 1 y voy a matar al padre SIGUSR2, mi pi y mi hora es: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
            
            //Manda SIGUSR2 al padre (su respectivo abuelo)
            kill(pidpadre,SIGUSR2);


            t=time(NULL);
            info=localtime(&t);
            gettimeofday(&tiempo,NULL);
            printf("\nSoy el nieto 1 y voy a dormir 5s, mi pi y mi hora es: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
            
            //Duerme 5 segundos
            int n=5;
            sleep(n);

            t=time(NULL);
            info=localtime(&t);
            gettimeofday(&tiempo,NULL);
            printf("\nSoy el nieto 1 y voy a morir, mi pi y mi hora es: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
            
            
            //Muere
            exit(333);
        }
    }else{
        t=time(NULL);
        info=localtime(&t);
        gettimeofday(&tiempo,NULL);
        printf("\nSoy el padre  y voy a dormir, mi pi y mi hora es: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
        
        //Duerme el rato necesario (no se duerme todo el teimpo porque en cuanto llegue SIGUSR2 continuara el codigo tomando el sleep ya hecho)
        int n=sleep(100);
        printf("\nSolo se durmieron %d segundos de 100\n",100-n);

        t=time(NULL);
        info=localtime(&t);
        gettimeofday(&tiempo,NULL);
        printf("\nSoy el padre  y voy a comprobar si SIGUSR1 está pendiente, mi pi y mi hora es: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
        
        //Seccion de mirar las señales pendiente de entrada
        sigpending(&pendientes);
        if (sigismember(&pendientes, SIGUSR1)) {
            printf("\nSIGUSR1 está pendiente\n");
        } 


        printf("\nestoy desbloqueando\n");


        t=time(NULL);
        info=localtime(&t);
        gettimeofday(&tiempo,NULL);
        printf("\nSoy el padre  y voy a desbloquear SIGUSR1, mi pi y mi hora es: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
        
        
        //Desbloqueamos la señal SIGUSR1 y la capturamos
        sigprocmask(SIG_UNBLOCK, &block, NULL);
        //sigaction(SIGUSR1, &sa, NULL);
        int esth1;


        t=time(NULL);
        info=localtime(&t);
        gettimeofday(&tiempo,NULL);
        printf("\nSoy el padre  y voy a esperar la muerte de hijo 1, mi pi y mi hora es: %d, %02d:%02d:%02ld:%06ld\n",getpid(),info->tm_hour,info->tm_min,tiempo.tv_sec%60,tiempo.tv_usec);
        
        
        //Espera a que mura hijo 1 e imprime su codigo de salida
        waitpid(h1,&esth1,0);
        printf("\nexit de hijo 1 con valor: %d\n",WEXITSTATUS(esth1));
    }
    return 0;
}