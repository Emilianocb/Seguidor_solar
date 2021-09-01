/*
 * variables.h
 *
 *  Created on: 25 ago. 2021
 *      Author: emili
 */

#ifndef INCLUDE_VARIABLES_H_
#define INCLUDE_VARIABLES_H_

# define PI 3.141592
double t=0; //Tiempo
float Dut = 1; //Duty1 del motor1
float Dut2 = 1; //Duty2 del motor1
float Dut3 = 1; //Duty1 del motor2
float Dut4 = 1; //Duty2 del motor2
int32_t ADCvalues[4]={0,0,0,0}; //vector de los valores adc
adcData_t adc_data[4];
int i_adc=0;
int ch_count=0;
int UpLeft;
int DownRight;
int UpRight;
int DownLeft;
float UP;
float DOWN;
float LEFT;
float RIGHT;

/*
 * Variables temporales
 */

float UP1;
float UP2;
float DOWN1;
float DOWN2;
float LEFT1;
float LEFT2;
float RIGHT1;
float RIGHT2;
float signal1;
float signal2;



#endif /* INCLUDE_VARIABLES_H_ */
