/**********************************LIBRERIAS DE C************************************/
#include <stdint.h>
#include <stdbool.h>
/***********************************LIBRERIAS TIVA C****************************************/
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_gpio.h"  //Para desbloquear pines
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
//*********************************DEFINICIONES*************************************//
#define Rojo  0
#define Azul  1
#define Verde 2

#define LedRojo  2
#define LedAzul  4
#define LedVerde 8

#define F_1Hz 40000000
#define F_2Hz 20000000
#define F_4Hz 10000000
//*********************************ESTRUCTURAS*************************************//
struct General{
    int ColordeLED;
    int Frecuencia;
    int Siguiente;
    int Anterior;
};

typedef const struct General TipoEstado;

TipoEstado SM[3] = {
     {LedRojo, F_1Hz, Azul, Verde},  //Rojo
     {LedAzul, F_2Hz, Verde, Rojo},  //Azul
     {LedVerde, F_4Hz, Rojo, Azul}  //Verde
};
//**********************************VARIABLES***************************************//
int Estado;
bool blink;
//********************************PROCEDIMIENTOS************************************//
void GPIO(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);  //Habilitar puerto F
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); //Pines 1, 2 y 3 del puerto F como salidas.

    GPIOPinTypeGPIOInput(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4);  //Pines 0 y 4 del puerto F como entradas.
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;  // Desbloquear PF0
    GPIO_PORTF_CR_R = 0x0f;
    GPIOPadConfigSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU); //PF0 y PF4 con resistencias PULL-UP
}

void TIMER(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);     //Habiliar Timer 0
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);  //Configurar el tipo de Timer
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void NVIC(void){
    IntMasterEnable();  //Habilitar interrupciones globales del periférico NVIC
    IntEnable(INT_GPIOF);  //Habilitar las interrupciones del puerto F
    GPIOIntEnable(GPIO_PORTF_BASE, GPIO_INT_PIN_0|GPIO_INT_PIN_4);  //Habilitar las interrupciones por PF0 y PF4
    GPIOIntTypeSet(GPIO_PORTF_BASE, GPIO_PIN_0|GPIO_PIN_4, GPIO_FALLING_EDGE); //Configuración de las condiciones para que PF0 y PF4 interrumpan
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Interrupción por Timer 0 al terminar de contar
    //Prioridades de interrupción
    IntPrioritySet(INT_GPIOF, 0);
    IntPrioritySet(INT_TIMER0A, 1);
}

void INICIO(void){
    Estado = Rojo;
    blink = false;
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, SM[Estado].ColordeLED);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 19999999);
}
//********************************RUTINAS DE INTERRUPCIÓN************************************//
void IntGPIO(void){
    uint32_t status;
    status = GPIOIntStatus(GPIO_PORTF_BASE, true);
    GPIOIntClear(GPIO_PORTF_BASE, status);
    switch(status){
    case GPIO_INT_PIN_0:
        Estado = SM[Estado].Siguiente;
        break;
    case GPIO_INT_PIN_4:
        Estado = SM[Estado].Anterior;
        break;
    default:
        break;
    }
}

void IntTIMER(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, (SM[Estado].Frecuencia) - 1);
    if(blink){
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, SM[Estado].ColordeLED);
        blink = false;
    }else{
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, 0);
        blink = true;
    }
}
//************************************MAIN******************************************//
int main(void){
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_5); //40MHz
    GPIO();
    TIMER();
    NVIC();
    INICIO();

	while(true){
	}
}
