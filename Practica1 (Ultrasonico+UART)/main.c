/*--------------------------------------------LIBRERÍAS DE C-------------------------------------------------*/
#include<stdint.h>
#include<stdbool.h>
/*-----------------------------------------LIBRERÍAS PARA LA TIVA C------------------------------------------*/
#include"inc/hw_memmap.h"
#include"inc/tm4c123gh6pm.h"
#include"inc/hw_types.h"
#include"driverlib/sysctl.h"
#include "driverlib/gpio.h"      //Periféricos de entrada y salida
#include "driverlib/interrupt.h" //Para interrupciones
#include "driverlib/timer.h"     //Para activar el timer
#include "driverlib/uart.h"      //Para protocolo UART
#include "driverlib/pin_map.h"   //Configurar pines de GPIO
/*-----------------------------------------------VARIABLES----------------------------------------------------*/
int StateTrigger = 0, Echo = 1;
uint32_t Load = 800000, T1, T2;
double Time, Distancia;

int i, Unidad, Decena, Centena, Decimal;
char Texto1[] = "Distancia: ";
char Texto2[] = " cm";
char Dato;
/*-----------------------------------------RUTINA DE INTERRUPCIÓN---------------------------------------------*/
void Int_Trigger(void){
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);  //Limpiar interrupción por fin de conteo de subTimer A - Timer 0
    if (StateTrigger == 0 ){
        StateTrigger = 128;
        Load = 800;  //Carga para subTimero A de Timer 0 = 10us
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, StateTrigger);  //Establecer Pin PB7 (Trigger) en alto
    }else{
        StateTrigger = 0;
        Load = 800000;  //Carga para subTimer A de Timer 0 = 100ms
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_7, StateTrigger);  //Establecer Pin PB7 (Trigger) en bajo
    }
    TimerDisable(TIMER0_BASE, TIMER_A);  //Deshabilitar subTimer A de Timer 0
    TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar subTimer A de Timer 0
}

void Int_GPIO(void){
    GPIOIntClear(GPIO_PORTB_BASE, GPIO_PIN_6); //Limpiar interrupción por PB6
    if (Echo == 1){
        T1 = TimerValueGet(TIMER1_BASE, TIMER_A);
        Echo = 0;
    }else{
        T2 = TimerValueGet(TIMER1_BASE, TIMER_A);
        Echo = 1;
        Time = (SysCtlClockGet())/(T1 - T2);
        Distancia = ((1/Time)*34320)/2;  //Calculo de la distancia medida

        //Separar número de la Distancia en Unidades, Decenas, Centenas y Decimales
        Centena = Distancia/100;
        Decena = (Distancia - (Centena*100))/10;
        Unidad = Distancia - (Centena*100) - (Decena*10);
        Decimal = (Distancia - (Centena*100) - (Decena*10) - Unidad)*10;
    }
}
/*----------------------------------------------PROCEDIMIENTOS-------------------------------------------------*/
void GPIO_Config(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  //Activar puerto B
    GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_6);  //PB6 = Echo, como entrada
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_7);  //PB7 = Trigger, como salida
    GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_6|GPIO_PIN_7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);  //Echo y Trigger con resistencia PULL-DOWN
}

void Echo_INTERRUP(void) {
    IntMasterEnable();  //Habilitar interrupciones globales del periférico NVICs
    IntEnable(INT_GPIOB);  //Habilitar las interrupciones de puerto B
    GPIOIntEnable(GPIO_PORTB_BASE, GPIO_INT_PIN_6);  //Habilitar la interrupcion por PB6 (Echo)
    GPIOIntTypeSet(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_BOTH_EDGES);  //Tipo de interrupcion por flanco de subida y bajada de PB6 (Echo)
    IntPrioritySet(INT_GPIOB, 0);  //Prioridad de interrupcion 0
}

void TIMERS_Config(void) {
    //TIMER0A - Disparo
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  //Habilitar Timer 0
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);  //Configurar el tipo de Timer como "un disparo"
    TimerLoadSet(TIMER0_BASE, TIMER_A, Load);  //Cargar subTimer A
    TimerEnable(TIMER0_BASE, TIMER_A);  //Habilitar subTimer A de Timer 0
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT); //Interrupción por fin de conteo
    IntEnable(INT_TIMER0A);  //Habilitar interrupción por subTimer A de Timer 0
    IntPrioritySet(INT_TIMER0A, 1);  //Prioridad de interrupcion 1

    //TIMER1A - Periódico
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);  //Habilitar Timer 1
    TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);  //Configurar el tipo de Timer como "periódico"
    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet());  //Cargar subTimer A con 80M
    TimerEnable(TIMER1_BASE, TIMER_A);  //Habilitar subTimer A de Timer 1
}

void UART_Config(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);  //Habilitar Módulo 0 del periférico UART
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);  //Activar puerto A
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0|GPIO_PIN_1);  //Configurar pines 0 y 1 de Puerto A como tipo UART
    GPIOPinConfigure(GPIO_PA0_U0RX);  //Configurar PA0 como RX
    GPIOPinConfigure(GPIO_PA1_U0TX);  //Configurar PA1 como TX
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE)); //Cadena de 8 bits, 1 bit de parada, paridad par
}

void Numeros(int n){ //Se establece el caracter decimal
    Dato = 48 + n;
}
/*--------------------------------------------------MAIN---------------------------------------------------------*/
int main(void) {
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_2_5); //Reloj de Tiva C a 80MHz
    GPIO_Config();
    Echo_INTERRUP();
    TIMERS_Config();
    UART_Config();

    while(true){
        for(i = 0; i < 11; i++) { //Eviar texto de "Distancia: "
            UARTCharPut(UART0_BASE, Texto1[i]);
        }
        Numeros(Centena);  //Establece el caracter equivalente al número de la Centena
        UARTCharPut(UART0_BASE, Dato); //Enviar Centena
        Numeros(Decena);  //Establece el caracter equivalente al número de la Decena
        UARTCharPut(UART0_BASE, Dato); //Enviar Decena
        Numeros(Unidad);  //Establece el caracter equivalente al número de la Unidad
        UARTCharPut(UART0_BASE, Dato); //Enviar Unidad
        UARTCharPut(UART0_BASE, '.'); //Enviar punto decimal
        Numeros(Decimal);  //Establece el caracter equivalente al número decimal
        UARTCharPut(UART0_BASE, Dato); //Enviar Decimal
        for(i = 0; i < 3; i++) { //Eviar texto de " cm"
                    UARTCharPut(UART0_BASE, Texto2[i]);
                }
        UARTCharPut(UART0_BASE, '\n'); //Línea nueva
        UARTCharPut(UART0_BASE, '\r'); //Retornar cursor
    }
}
