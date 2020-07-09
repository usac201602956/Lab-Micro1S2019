/*--------------------------------------------LIBRERÍAS DE C-------------------------------------------------*/
#include<stdint.h>
#include<stdbool.h>
/*-----------------------------------------LIBRERÍAS PARA LA TIVA C------------------------------------------*/
#include"inc/hw_memmap.h"
#include"inc/tm4c123gh6pm.h"
#include"inc/hw_types.h"
#include"driverlib/sysctl.h"
#include "driverlib/gpio.h"      //Periféricos de entrada y salida
#include "inc/hw_gpio.h"         //Para desbloquear pines
#include "driverlib/interrupt.h" //Para interrupciones
#include "driverlib/timer.h"     //Para activar el timer
#include "driverlib/uart.h"      //Para protocolo UART
#include "driverlib/pin_map.h"   //Configurar pines de GPIO para otro fin
#include "driverlib/pwm.h"       //Librería para usar PWM

uint16_t F_Hz = 50; //Frecuencia a la que trabaja ServoMotor
uint16_t T_ciclos;
uint16_t Push;
uint16_t T_alto;
uint16_t Angulo;

void PWM(void) {
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64); //Frecuencia a la que trabajará el PWM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1); //Habilitar modulo 1 de PWM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); //Habilitar puerto F
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1); //Configurar pin 1 de puerto F como tipo PWM
    GPIOPinConfigure(GPIO_PF1_M1PWM5); //PF1 como salida de PWM5
}

void GPIO(void){
    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4); //PF0 y PF4 como entradas.
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD); //Configurar Pines: Driver Strength y Resistencia como pull-down
}

void PWMSignalConfigure(void) {
    T_ciclos = (SysCtlClockGet()/64)/F_Hz; //---> 12,500
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN); //Tipo de conteo para el PWM por bajada.
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, T_ciclos); //Se especifica el periodo de la señal PWM ---> 12,500 equivale a T = 20ms => f = 50Hz.
    //Se establece el ancho de pulso = (ciclo_trabajo*T_ciclos)/100
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, T_alto); //T_alto = 625 --> 1ms ----> 0°
    PWMGenEnable(PWM1_BASE, PWM_GEN_2); //Habilitar generador 2 de modulo 1 PWM
    PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT, true); //switch para activar/desactivar salida de PWM 5
}

int main(void) {
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_5);   //Reloj de Tiva C a 40MHz
    PWM();
    PWMSignalConfigure();
    GPIO();

	while(1){
	    Push = GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);
	    switch(Push) {
	        case 1:
	            Angulo = 90;
	            SysCtlDelay(800000);
	            break;
	        case 16:
	            Angulo = 0;
	            SysCtlDelay(800000);
	            break;
	        default:
	            T_alto = (125*(Angulo + 180))/36; //Determina el tiempo en alto del pulso de la señal PWM.
	            break;
	    }

	}
}
