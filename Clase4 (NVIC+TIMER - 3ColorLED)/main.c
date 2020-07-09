#include<stdint.h>
#include<stdbool.h>

#include"inc/hw_memmap.h"
#include"inc/tm4c123gh6pm.h"
#include"inc/hw_types.h"
#include"driverlib/sysctl.h"
#include "driverlib/gpio.h"      //Periféricos de entrada y salida
#include "inc/hw_gpio.h"         //Para desbloquear pines
#include "driverlib/interrupt.h" //Para interrupciones
#include "driverlib/timer.h"     //Para activar el timer

uint32_t Conteo = 0;
volatile uint8_t color = 2;

void TIMER(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);     //Activar Timer 0
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);  //Configurar el tipo de Timer
    TimerLoadSet(TIMER0_BASE, TIMER_A, 39999999);     //Cargar Timer
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void GPIO(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);                              //Activar puerto F
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3); //Pines 1, 2 y 3 del puerto F como salidas
}

void config_Interrupt(void) {
    IntEnable(INT_TIMER0A);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntMasterEnable();
}

void IntTimer(void) {
    if(color < 8) {
            color = color*2;
    }else{
            color = 2;
    }
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, color);
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

int main(void) {
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_5); //40MHz

	TIMER();
	GPIO();
	config_Interrupt();

	while(1){
	    }
}
