/** @file sys_main.c 
*   @brief Application main file
*   @date 11-Dec-2018
*   @version 04.07.01
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

/* 
* Copyright (C) 2009-2018 Texas Instruments Incorporated - www.ti.com 
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


/* USER CODE BEGIN (0) */
/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */
#include "gio.h"
#include "het.h"
#include "FreeRTOS.h"
#include "sci.h"
#include "os_task.h"
#include "os_semphr.h"
#include "adc.h"
#include "stdlib.h"
#include "stdio.h"
#include "math.h"
/*
 * Añadimos la libreria que contiene nuestras variables
 * Asi como se declaran las variables para menejo de las tareas
 * y las cabeceras de las funciones y / o tareas.
 */
#include "variables.h"

xTaskHandle xTask1Handle;
xTaskHandle xTask2Handle;
xTaskHandle xTask3Handle;

void vTarea1(void *pvParameters);
void vTarea2(void *pvParameters);
void vTarea3(void *pvParameters);
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */


/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */
    adcInit();
    hetInit();
    gioInit();

    /*
     * Ejemplo de la cracion de tareas y explicación de sus diferentes parametros
     *
     * Se crearon las variables con la misma prioridad para que sea el scheduler el que
     * dice cuando debenejecutarse.
     *
        if (xTaskCreate(vTarea1,                    // Puntero de la funcion que implementa la tarea
                    "Sensar",                   // Nombre de la tarea en el sistema
                    configMINIMAL_STACK_SIZE,   // Tamaño del STACK
                    NULL,                       // texto a imprimir en la tarea utilizando el parámetro de tarea
                    2,                          // Prioridad
                    &xTask1Handle               // Manejador de la prioridad
                    ) != pdTRUE){
        while(1);   //error
        }
     */

    if (xTaskCreate(vTarea1, "Sensar",  configMINIMAL_STACK_SIZE, NULL, 1, &xTask1Handle) != pdTRUE){
         while(1);   //error
     }
    //Creamos tarea 2 encargada de calcular la posicion
    if (xTaskCreate(vTarea2, "Calcular", configMINIMAL_STACK_SIZE, NULL, 1, &xTask2Handle) != pdTRUE){
        while(1);   //error
    }

    //Creamos tarea 3 encargada de mover el servo a la posicion calculada
    if (xTaskCreate(vTarea3, "Mover", configMINIMAL_STACK_SIZE, NULL, 1, &xTask3Handle) != pdTRUE){
        while(1);   //error
    }

    // Iniciar el scheduler
    vTaskStartScheduler();   // Inicia el sistema operativo
    //while(1);               //nunca debera llegar aqui el PC
/* USER CODE END */

    return 0;
}


/* USER CODE BEGIN (4) */
// Funci\'on de servicio de inactividad
void vApplicationIdleHook(void){
    //Inactividad
}

void vTarea1(void *pvParameters){
    //Variable para manejo de prioridad
    unsigned portBASE_TYPE uxPrioridad;

    //leer la prioridad actual de la tarea
    uxPrioridad = uxTaskPriorityGet(NULL);

    /*
     * Entramos a un ciclo infinito de donde no debe de salir
     * De esta manera la tarea siempre estara leyendo datos
     */
    for(;;){
        //Realiza la medicion de los sensores
        adcStartConversion(adcREG1, adcGROUP1);
        while((adcIsConversionComplete(adcREG1,adcGROUP1))==0);
        ch_count = adcGetData(adcREG1, adcGROUP1,&adc_data[0]);
        i_adc=0;

        /*
         * Asignamos los datos a las variables
         * Se asigna de acuerdo a la posición y orientación
         */

        UpLeft = ((int)((unsigned int)(adc_data[2].value)));    //Arriba izquierda  = adc[2]
        DownRight = ((int)((unsigned int)(adc_data[1].value))); //Abajo derecha     = adc[1]
        UpRight = ((int)((unsigned int)(adc_data[0].value)));   //Arriba derecha    = adc[0]
        DownLeft = ((int)((unsigned int)(adc_data[3].value)));  //Abajo izquierda   = adc[3]
    }
}

void vTarea2(void *pvParameters)
{
    unsigned portBASE_TYPE uxPrioridad;

    //leer la prioridad actual
    uxPrioridad = uxTaskPriorityGet(NULL);
    for(;;){

        /*
         * Promediamos los valores de arriba y los de abajo, este promedio se asigna a otra
         * variable
         * Mantenemos los datos de dos muestras anteriores, debido a la sensibilidad de los
         * LDR y su alta fluctuación, este es el error.
         */

        UP2 = UP1;
        UP1 = UP;
        UP = (UpLeft + UpRight) / 2;

        DOWN2 = DOWN1;
        DOWN1 = DOWN;
        DOWN = (DownRight + DownLeft) / 2;

        /*
         * Debido a la fulctuación antes mencionada tenemos un Duty anterior el cual ayuda
         * a comprobar que efectivamente este esta subiendo o bajando y no es solo un error
         * por la fluctuación.
         *
         * Se calculan porcentajes de duty y se promedia con el duty anterior, se le da un
         * rango de +- 0.05 de valor de cambio de duty anterior con el actual y si se cumple
         * se calcula la señal sacando el porcentaje de duty.
         */

        Dut2 = Dut;
        Dut = (((UP - DOWN)/UP) + ((UP2 - DOWN2)/UP2))/2;
        if(Dut >= Dut2 + 0.05 || Dut >= Dut2 - 0.05){
            signal1 = (Dut * 4095)/100;
            pwmSetDuty(hetRAM1, pwm2, signal1);
        }
    }
}

void vTarea3(void *pvParameters)
{
    unsigned portBASE_TYPE uxPrioridad;

    //leer la prioridad actual
    uxPrioridad = uxTaskPriorityGet(NULL);
    for(;;){

        LEFT2 = LEFT1;
        LEFT1 = LEFT;
        LEFT = (UpLeft + DownLeft) / 2;

        RIGHT2 = RIGHT1;
        RIGHT1 = RIGHT;
        RIGHT = (DownRight + UpRight) / 2;

        Dut4 = Dut3;
        Dut3 = (((LEFT - RIGHT)/LEFT) + ((LEFT2 - RIGHT2)/LEFT2))/2;
        if(Dut3 >= Dut4 + 0.05 || Dut3 >= Dut4 - 0.05){
            signal2 = (Dut3 * 4095)/100;
            pwmSetDuty(hetRAM1, pwm1, signal2);
        }

    }

}
/* USER CODE END */
