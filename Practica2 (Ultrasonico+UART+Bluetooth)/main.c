/*--------------------------------------------LIBRERÍAS DE C-------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
/*-----------------------------------------LIBRERÍAS PARA LA TIVA C------------------------------------------*/
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"      //Periféricos de entrada y salida
#include "inc/hw_gpio.h"         //Para desbloquear pines
#include "driverlib/interrupt.h" //Para interrupciones
#include "driverlib/timer.h"     //Para activar el timer
#include "driverlib/uart.h"      //Para protocolo UART
#include "driverlib/pin_map.h"   //Configurar pines de GPIO
/*------------------------------------------------VARIABLES--------------------------------------------------*/
uint32_t echo = 1;
uint32_t v1 = 0;
uint32_t v2 = 0;

int i, Unidad, Decena, Centena, Decimal;
double Distancia, tiempo;
char Texto1[] = "Distancia: ";
char Texto2[] = " in";
char Dato;
/*-------------------------------------------------MÉTODOS---------------------------------------------------*/
void GPIO_Config(void);
void Timer_Config(void);
void UART_Config(void);
void Numeros(int n);
/*----------------------------------------------INTURRUPCIONES-----------------------------------------------*/
void Int_Trigger(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 0);
    TimerDisable(TIMER2_BASE, TIMER_A);
    TimerEnable(TIMER2_BASE, TIMER_A);
}

void Int_Delay(void){
    TimerIntClear(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, 128);
    TimerDisable(TIMER0_BASE, TIMER_A);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void Int_DataGet(void){
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_6); //Limpiar interrupción por PB6.
    if (echo == 1){
        v1 = TimerValueGet(TIMER1_BASE, TIMER_A);
        echo = 0;
    }else{
        v2 = TimerValueGet(TIMER1_BASE, TIMER_A);
        echo = 1;
        tiempo = (SysCtlClockGet())/(v1 - v2);
        Distancia = (((1/tiempo)*34320)/2)/2.54;   //Calculo de la distancia medida.

        //Separar número de la Distancia en Unidades, Decenas, Centenas y Decimales.
        Centena = Distancia/100;
        Decena = (Distancia - (Centena*100))/10;
        Unidad = Distancia - (Centena*100) - (Decena*10);
        Decimal = (Distancia - (Centena*100) - (Decena*10) - Unidad)*10;
    }
}
/*----------------------------------------------PROCEDIMIENTOS-----------------------------------------------*/
void GPIO_Config(void){
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  //Habilitar el puerto B
        GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_6);  //PB6 = Echo
        GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);  //Echo con resistencia PULL-DOWN
        GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7);  //PB7 = Trigger

        IntMasterEnable();  //Habilitar interrupciones globales del periférico NVIC
        GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_BOTH_EDGES);  //Tipo de interrupcion por flancos de subida y bajada de PB6
        GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_6);  //Habilita la interrupcion por PB6
        IntEnable(INT_GPIOB);  //Habilita las interrupciones de puerto B
        IntPrioritySet(INT_GPIOB, 0);  //Prioridad de interrupcion
}

void Timer_Config(void){
//TIMERA A0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER0_BASE, TIMER_A, 268);                     //10us
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntPrioritySet(INT_TIMER0A, 0);
    IntEnable(INT_TIMER0A);
//TIMER A1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet());        //1 seg
    TimerEnable(TIMER1_BASE, TIMER_A);
//TIMER A2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    TimerConfigure(TIMER2_BASE, TIMER_CFG_ONE_SHOT);
    TimerLoadSet(TIMER2_BASE, TIMER_A, (0.1*SysCtlClockGet()));  //0.1 seg
    TimerEnable(TIMER2_BASE, TIMER_A);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);
    IntPrioritySet(INT_TIMER2A, 0);
    IntEnable(INT_TIMER2A);
}

void UART_Config(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);  //Habilitar Módulo 1 del periférico UART
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1);  //Configurar pines 0 y 1 de Puerto B como tipo UART
    GPIOPinConfigure(GPIO_PB0_U1RX);  //Configurar PB0 como RX
    GPIOPinConfigure(GPIO_PB1_U1TX);  //Configurar PB1 como TX
    UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE)); //Cadena de 8 bits, 1 bit de parada, paridad par
}

void Numeros(int n){ //Selección de caracter decimal
    switch(n){
    case 1:
        Dato = '1';
        break;
    case 2:
        Dato = '2';
        break;
    case 3:
        Dato = '3';
        break;
    case 4:
        Dato = '4';
        break;
    case 5:
        Dato = '5';
        break;
    case 6:
        Dato = '6';
        break;
    case 7:
        Dato = '7';
        break;
    case 8:
        Dato = '8';
        break;
    case 9:
        Dato = '9';
        break;
    default:
        Dato = '0';
        break;
    }
}
/*------------------------------------------------------MAIN------------------------------------------------------*/
int main(void) {
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_2_5);  //Reloj de Tiva C a 80MHz
    GPIO_Config();
    Timer_Config();
    UART_Config();

    while(true){
        for(i = 0; i < 11; i++) { //Eviar texto de "Distancia: "
            UARTCharPut(UART1_BASE, Texto1[i]);
        }
        Numeros(Centena);  //Escoge el caracter equivalente al número de la Centena
        UARTCharPut(UART1_BASE, Dato); //Enviar Centena
        Numeros(Decena);  //Escoge el caracter equivalente al número de la Decena
        UARTCharPut(UART1_BASE, Dato); //Enviar Decena
        Numeros(Unidad);  //Escoge el caracter equivalente al número de la Unidad
        UARTCharPut(UART1_BASE, Dato); //Enviar Unidad
        UARTCharPut(UART1_BASE, '.'); //Enviar punto decimal
        Numeros(Decimal);  //Escoge el caracter equivalente al número decimal
        UARTCharPut(UART1_BASE, Dato); //Enviar Decimal
        for(i = 0; i < 3; i++) { //Eviar texto de " cm"
                    UARTCharPut(UART1_BASE, Texto2[i]);
                }
        UARTCharPut(UART1_BASE, '\n'); //Línea nueva
        UARTCharPut(UART1_BASE, '\r'); //Retornar cursor
    }
}

