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
#include "driverlib/pin_map.h"   //Configurar pines de GPIO

void UART(void);

void UART(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);     //Activar Puerto A
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);     //Activar UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0|GPIO_PIN_1);
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
}

/*void Interrupcion(void) {
    UARTIntEnable(UART0_BASE, UART_INT_RX|UART_INT_TX);
    IntEnable(INT_UART0);
    IntMasterEnable();
}

void IntUART(void) {
    UARTIntClear(UART0_BASE, UARTIntStatus(UART0_BASE, true));
    if (UARTCharsAvail(UART0_BASE)) {   //Verifica si hay un dato en el buffer de recepeción
        UARTCharPut(UART0_BASE, UARTCharGet(UART0_BASE));
    }
}*/

int main(void) {
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_5);   //Reloj de Tiva C a 40MHz
    UART();
    //Interrupcion();

	while(1) {
	    UARTCharPut(UART0_BASE, 'J');
	}
}
