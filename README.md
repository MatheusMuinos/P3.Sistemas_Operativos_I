# P3.Sistemas_Operativos_I — README

Breve: conjunto de programas de práctica sobre señales, fork/wait y comunicación por señales (SIGUSR1, SIGUSR2, SIGALRM, SIGINT, SIGTERM). Requieren entorno POSIX (Linux / WSL). Cada fichero contiene comentarios sobre su propósito; abajo se describen uso y resultados esperados.

Requisitos
- Entorno POSIX (Linux o WSL en Windows).
- gcc instalado.
- Ejecutar desde la terminal (no desde cmd.exe puro).

Cómo compilar (ejemplo)
- gcc -Wall -O2 -o Entregable1_1 Entregable1_1.c
- gcc -Wall -O2 -o Entregable2_2 Entregable2_2.c
- gcc -Wall -O2 -o e2 e2.c
- gcc -Wall -O2 -o ex3 ex3.c
- gcc -Wall -O2 -o ex4 ex4.c
- gcc -Wall -O2 -o ex5 ex5.c
- gcc -Wall -O2 -o ex6 ex6.c
- gcc -Wall -O2 -o ex2 ex2.c

Nota: alguns arquivos podem ser versões de estudo — o `Entregable2_2.c` costuma ser a versão completa do exercício sobre bloqueo/pendientes que também aparece em `e2.c`.

Descrição dos ficheiros e uso esperado

1) Entregable1_1.c
- Propósito: demonstra uso de `signal()` y `alarm()`.
- O que faz: imprime seu PID, programa alarm(1) cada segundo e calcula/mostra um número periódico; ao receber SIGUSR1 termina.
- Como testar:
  - ./Entregable1_1
  - Em otra terminal: kill -SIGUSR1 <pid> → o programa termina.
- Resultado esperado:
  - Saídas periódicas com o valor calculado; tras enviar SIGUSR1, imprime mensagem de saída e termina.

2) Entregable2_2.c
- Propósito: versão completa do exercício de P (padre), H1 (hijo) y N1 (nieto) que demonstra bloqueo de SIGUSR1, entrega diferida e orden con SIGUSR2.
- O que faz:
  - P bloquea SIGUSR1 antes de fork.
  - H1 envía SIGUSR1 a P y crea N1.
  - N1 envía SIGUSR2 a P, duerme e sai (código 111).
  - P duerme un tiempo (con SIGUSR1 bloqueada), comprueba `sigpending`, desbloquea SIGUSR1; por diseño verá la entrega de SIGUSR2 antes que SIGUSR1.
  - P espera H1 y muestra el código de salida (11).
- Como testar:
  - ./Entregable2_2
  - Observar las impresiones con timestamps; buscar que la línea informando de SIGUSR2 aparezca antes que la de SIGUSR1 tras el desbloqueo.
- Resultados esperados:
  - Mensajes temporizados de P, H1, N1.
  - Mensaje indicando SIGUSR1 pendente antes de desbloquear.
  - Entrega de SIGUSR2 mientras SIGUSR1 está bloqueada; al desbloquearse, SIGUSR1 se entrega (pero después del mensaje de SIGUSR2).
  - H1 finaliza con exit(11), N1 con exit(111).

3) e2.c
- Propósito: mesma idea que `Entregable2_2.c` (bloqueo/pendientes/orden de entrega).
- Observação: dependendo de la versión en el archivo, puede estar incompleto o simplificado. Si dudas, usa `Entregable2_2.c`.

4) ex3.c
- Propósito: ejemplo de padre/hijo enviando SIGUSR1/SIGUSR2 y terminación con SIGTERM.
- O que fazer:
  - ./ex3
  - Observa las impresiones del padre e hijo ante las señales.
- Resultado esperado:
  - El hijo recibe señales SIGUSR2 varias veces y finalmente SIGTERM y termina.

5) ex4.c
- Propósito: interceptar SIGINT (Ctrl+C) y, tras 3 interrupciones, restaurar comportamiento por defecto.
- Cómo probar:
  - ./ex4
  - Pulsar Ctrl+C tres veces; el programa captura las primeras tres y luego se termina con el comportamiento por defecto.

6) ex5.c
- Propósito: ejemplo con múltiples forks y envío de señales entre hijos (interacción manual por scanf).
- Uso:
  - ./ex5
  - Siga las instrucciones por pantalla para decidir si matar H1 desde H2.
- Resultado esperado:
  - H1 espera indefinidamente hasta recibir SIGUSR1; H2 puede enviar SIGUSR1 y H1 terminar.

7) ex6.c
- Propósito: demostrar `pause()` y envío de señales entre procesos.
- Uso:
  - ./ex6
  - El padre crea hijo que hace pause(); el padre duerme y luego envía SIGUSR1 para despertarlo.
- Resultado esperado:
  - El hijo despierta tras recibir SIGUSR1 y muestra mensaje.

8) ex2.c
- Propósito: fichero de prueba muy simple que imprime pid y espera entrada por scanf.
- Uso:
  - ./ex2
  - Está pensado solo para tests rápidos manuales.

Observaciones importantes
- Señales y fork/wait requieren entorno POSIX. En Windows nativo pueden no funcionar: use WSL o una máquina virtual Linux.
- Algunos handlers usan `printf` dentro de la rutina de señal en versiones del repo. Esto no es async-signal-safe; en entornos reales conviene usar variables sig_atomic_t o write(). No obstante, los ejercicios están diseñados para visualizar comportamiento y pueden usar printf para facilitar salida en prácticas.
- Si quiere validar el orden de entrega (SIGUSR2 antes que SIGUSR1), use `Entregable2_2.c` ó la versión completa de `e2.c` y observe los timestamps.

Pruebas rápidas recomendadas
- Ejecutar `Entregable2_2` y observar:
  - P imprime que SIGUSR1 está bloqueada.
  - H1 manda SIGUSR1.
  - N1 manda SIGUSR2; ver mensaje de SIGUSR2.
  - P comprueba `sigpending` y muestra SIGUSR1 pendiente.
  - P desbloquea SIGUSR1 y se observa la entrega (mensaje de SIGUSR1).
  - P informa exit de H1 (11).
- En Entregable1_1: ejecutar y luego `kill -SIGUSR1 <pid>` para forzar salida inmediata.

¿Quieres que aplique la versión más enxuta de `e2.c` (la proposta anteriormente) directamente ao arquivo c:\Users\mathe\Downloads\P3.Sistemas_Operativos_I\e2.c?