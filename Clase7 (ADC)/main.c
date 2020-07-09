/**********************************LIBRERIAS DE C************************************/
#include <stdint.h>
#include <stdbool.h>
/***********************************LIBRERIAS****************************************/
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/interrupt.h"
#include "driverlib/adc.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"

uint32_t sample[4];
volatile float TempTiva;
volatile float TempLM35;

void ADC(void){
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);  //Habilitar módulo 0 de ADC
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  //Habilitar puerto B
    GPIOPinTypeADC(GPIO_PORTB_BASE, GPIO_PIN_5);  //Pin PB5 de tipo ADC

    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 39999);  //Establecer periodo de muestreo por Timer 0
    TimerControlTrigger(TIMER0_BASE, TIMER_A, true);  //Trigger para muestreo por Timer
    TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar Timer 0 y subtimer A
    ADCSequenceConfigure(ADC0_BASE, 1, ADC_TRIGGER_TIMER, 0);  //Se configura secuenciador 1
    ADCSequenceStepConfigure(ADC0_BASE, 1, 0, ADC_CTL_CH11);  //Se lee canal 11 --> PB5 ----> LM35
    ADCSequenceStepConfigure(ADC0_BASE, 1, 1, ADC_CTL_CH11);
    ADCSequenceStepConfigure(ADC0_BASE, 1, 2, ADC_CTL_TS);  //Se lee el sensor de temperatura interno de la Tiva C
    ADCSequenceStepConfigure(ADC0_BASE, 1, 3, ADC_CTL_TS|ADC_CTL_END|ADC_CTL_IE); //Terminar de realizar la última muestra el ADC y levanta un interrupción
    ADCSequenceEnable(ADC0_BASE, 1);  //Habilitar secuenciador 1
}

void Config_Interrupt(void){
    ADCIntEnable(ADC0_BASE, 1);
    IntEnable(INT_ADC0SS1);
    IntMasterEnable();  //Habilitar interrupciones globales del periférico NVIC
}

void IntADC(void){
    ADCIntClear(ADC0_BASE, 1);
    ADCSequenceDataGet(ADC0_BASE, 1, sample);  //Toma el valor de la muestra y lo va almacenando en el vector "Sample".
    TempTiva = (147.5 - ((247.5*sample[2])) / 4096);
    TempLM35 = ((sample[0]*330)/4096)/10;
}

int main(void){
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_5);  //Reloj de Tiva C a 40MHz
    ADC();
    Config_Interrupt();

    while(1){
    }
}
