//P2-SSOO-22/23

// MSH main file
// Write your msh source code here

//#include "parser.h"
#include <stddef.h>         /* NULL */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>
#include <pthread.h>

#define MAX_COMMANDS 8


// ficheros por si hay redirección
char filev[3][64];

//to store the execvp second parameter
char *argv_execvp[8];


void siginthandler(int param) {
    printf("****  Saliendo del MSH **** \n");
//signal(SIGINT, siginthandler);
    exit(0);
}


/* Timer */
pthread_t timer_thread;
unsigned long mytime = 0;

void *timer_run() {
    while (1) {
        usleep(1000);
        mytime++;
    }
}

/**
* Get the command with its parameters for execvp
* Execute this instruction before run an execvp to obtain the complete command
* @param argvv
* @param num_command
* @return
*/
void getCompleteCommand(char ***argvv, int num_command) {
//reset first
    for (int j = 0; j < 8; j++)
        argv_execvp[j] = NULL;

    int i = 0;
    for (i = 0; argvv[num_command][i] != NULL; i++)
        argv_execvp[i] = argvv[num_command][i];
}


/**
* Main sheell  Loop  
*/
int main(int argc, char *argv[]) {
/**** Do not delete this code.****/
    int end = 0;
    int executed_cmd_lines = -1;
    char *cmd_line = NULL;
    char *cmd_lines[10];

    if (!isatty(STDIN_FILENO)) {
        cmd_line = (char *) malloc(100);
        while (scanf(" %[^\n]", cmd_line) != EOF) {
            if (strlen(cmd_line) <= 0) return 0;
            cmd_lines[end] = (char *) malloc(strlen(cmd_line) + 1);
            strcpy(cmd_lines[end], cmd_line);
            end++;
            fflush(stdin);
            fflush(stdout);
        }
    }

    pthread_create(&timer_thread, NULL, timer_run, NULL);

/*********************************/

    char ***argvv = NULL;
    int num_commands;


    while (1) {
        int status = 0;
        int command_counter = 0;
        int in_background = 0;
        signal(SIGINT, siginthandler);

// Prompt 
        write(STDERR_FILENO, "MSH>>", strlen("MSH>>"));

// Get command
//********** DO NOT MODIFY THIS PART. IT DISTINGUISH BETWEEN NORMAL/CORRECTION MODE***************
        executed_cmd_lines++;
        if (end != 0 && executed_cmd_lines < end) {
            command_counter = read_command_correction(&argvv, filev, &in_background, cmd_lines[executed_cmd_lines]);
        } else if (end != 0 && executed_cmd_lines == end) {
            return 0;
        } else {
            command_counter = read_command(&argvv, filev, &in_background); //NORMAL MODE
        }
//************************************************************************************************


/************************ STUDENTS CODE ********************************/
         /* SECCION REDIRECCION DE ERROR*/
         // Esto debe ser lo primero porque si queremos que haya redireccion de error, la redireccion tiene que ocurrir antes del error.
        if (strcmp(filev[2], "0") != 0) {
            close(2);
            int a = open(filev[2], O_WRONLY | O_CREAT, 0666); // Lo crea si no está
            if (open < 0) {
                printf("Error: salida de error especificada no válida");
                exit(-1); // Fin
            }
        }
    
        if (command_counter > 0) {
            if (command_counter > MAX_COMMANDS) {
                printf("Error: Numero máximo de comandos es %d \n", MAX_COMMANDS);
                exit(-1); // Fin
            } else {
                 /* SECCION MANDATOS INTERNOS*/
                if (strcmp(argvv[0][0], "mycalc") == 0 && command_counter == 1) // Necesitamos comprobar número de argumentos
                {
                    setenv("Acc", "0", 0);
                    if (strcmp(argvv[0][2], "add") == 0) 
                    {
                        int operacion = atoi(argvv[0][1]) + atoi(argvv[0][3]);
                        int nuevoEnv = operacion + atoi(getenv("Acc"));
                        char nuevoEnvS[10]; // Numero maximo de cifras en un int
                        // String con nuevo env a guardar
                        sprintf(nuevoEnvS, "%d", nuevoEnv);
                        setenv("Acc", nuevoEnvS, 1);
                        char operacionString[53]; // Maximo tamano en bytes de esta estructura, puesto que como maximo un int pede tener 10 caracteres
                        sprintf(operacionString, "[OK] %s + %s = %d; Acc %s\n", argvv[0][1], argvv[0][3], operacion, nuevoEnvS);

                        write(STDERR_FILENO, operacionString, strlen(operacionString)); // Salida de error al success como indicado en enunciado
                    } // de add
                    else if (strcmp(argvv[0][2], "mul") == 0 && command_counter == 1) 
                    {                   
                        int operacion = atoi(argvv[0][1]) * atoi(argvv[0][3]);
                        char operacionString[37]; // Maximo tamano en bytes de esta estructura, puesto que como maximo un int pede tener 10 caracteres
                        sprintf(operacionString, "[OK] %s * %s =  %d\n", argvv[0][1], argvv[0][3], operacion);
                        write(STDERR_FILENO, operacionString, strlen(operacionString));
                    } // de mul
                    else if (strcmp(argvv[0][2], "div") == 0 && command_counter == 1) 
                    {
                        int operacion = atoi(argvv[0][1]) / atoi(argvv[0][3]);
                        int resto = atoi(argvv[0][1]) % atoi(argvv[0][3]);
                        char operacionString[37]; // Maximo tamano en bytes de esta estructura, puesto que como maximo un int pede tener 10 caracteres
                        sprintf(operacionString, "[OK] %s / %s =  %d; Resto %d\n", argvv[0][1], argvv[0][3], operacion,
                                resto);
                        write(STDERR_FILENO, operacionString, strlen(operacionString));
                    } // del div
                    else // llego aqui si no se ha intrducido correctamente el mandato
                    {
                        printf("[ERROR] La estructura del comando es mycalc <operando_1> <add/mul/div> <operando_2>\n");
                    } // de ERROR
                } // de mycalc
                else if (strcmp(argvv[0][0], "mytime") == 0) 
                {
                    char time[23]; // Siempre ocupara 23 bytes como max (warnings)
                    unsigned long segundos = (mytime / 1000) % 60;
                    unsigned long minutos = (mytime / 60000) % 60;
                    unsigned long horas = (mytime / 360000);

                    if (segundos < 10 && minutos < 10) 
                    {
                        sprintf(time, "0%lu:0%lu:0%lu\n", horas, minutos, segundos);
                    } 
                    else if (segundos < 10) 
                    {
                        sprintf(time, "0%lu:%lu:0%lu\n", horas, minutos, segundos);
                    } 
                    else if (horas < 10) 
                    {
                        sprintf(time, "0%lu:0%lu:%lu\n", horas, minutos, segundos);
                    } 
                    else 
                    {
                        sprintf(time, "0%lu:%lu:%lu\n", horas, minutos, segundos);
                    }
                    write(STDERR_FILENO, time, strlen(time));
                } // de mytime
                /* SECCION PIPES Y MANDATOS EXTERNOS*/
                else 
                {
                    // Array de dos espacios que nos servira como pipe
                    int tub[2];
                    // Variable error
                    int ret;
                    // Variable en la que almacenamos el pid del fork()
                    pid_t pid;
                    // Variable para crear pipes en mismo espacio de memoria
                    int p10;
                    // Ahora si, podemos comprobar si es el ultimo mandato o no
                    for (int i = 0; i < command_counter; i++) 
                    {
                        // Si no soy el ultimo mandato, creamos un pipe nuevo
                        if (i != (command_counter - 1)) 
                        {
                            ret = pipe(tub);
                            // Si hay un error creando el pipe
                            if (ret < 0) 
                            {
                                perror("Error en la creación de la tubería");
                                exit(-1);
                            } // de if: error creacion pipe
                        } // de if: creacion pipe

                        // Hacemos el fork
                        pid = fork();
                        // Si hay un error creando el hijo
                        if (pid < 0) 
                        {
                            perror("Error en la creación del hijo");
                            exit(-1);
                        }// del if: error en el fork

                        // Hacemos redirecciones y limpieza
                        if (pid == 0) // Si soy el hijo
                        {
                            if (i != 0) // No soy el primero, redirijo la entrada
                            {
                                close(0);
                                dup(p10);
                                close(p10); // Aqui no hay limpieza?
                            } // del if no soy el primero
                            else if (strcmp(filev[0], "0") != 0)// Si soy el primero y hay redireccion
                            {
                                close(0);
                                open(filev[0], O_RDONLY);
                            } // del primero
                            if (i != (command_counter - 1)) // No soy el último, redirijo la salida
                            {
                                // Escritura del proceso = tub[1]
                                close(1);
                                dup(tub[1]);
                                // Cerramos descriptores de fichero abiertos por pipe(). Limpieza. 
                                close(tub[0]);
                                close(tub[1]);
                            } 
                            else if (strcmp(filev[1], "0") != 0) 
                            {
                                close(1);
                                open(filev[1], O_WRONLY | O_CREAT, 0666);
                            } // Del else if
                            execvp(argvv[i][0], argvv[i]);
                            // Si ha habido algún problema ejecutando
                            perror("Error al ejecutar\n");
                            exit(-1);
                        } // del hijo
                        else // Soy el padre
                        {
                            
                            if (i != 0) // No soy el primero
                            {
                                close(p10);
                            }
                            if (i != (command_counter - 1)) // No soy el último, salida
                            {
                                close(tub[1]);
                                p10 = tub[0];
                            }
                        } // del else (padre)
                        
                    } // FIN DEL FOR
                    
                    // Si !bg
                    if (!in_background) 
                    {
                        while (pid != wait(&status));
                    }
                    
                } // Fin del if else mandatos internos
                
            }// Zona PROHIBIDA

        } // del if (command_counter > 0)

    } // del while
    
    return 0;
}


	

