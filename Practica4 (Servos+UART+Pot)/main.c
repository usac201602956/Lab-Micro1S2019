/*---------------------------------------------------LIBRERÍAS-----------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "inc/hw_gpio.h"
#include "driverlib/pwm.h"
#include "driverlib/uart.h"

/*-----------------------------------------------DEFINICIONES--------------------------------------------------*/
#define F_Hz 50
uint32_t T_cycles;
uint32_t V_HighTime,H_HighTime = 0;
char TextOne[] = "POSICION: ";
/*--------------------------------------------VARIABLES GLOBALES-----------------------------------------------*/
uint32_t V_Sample[4];
uint32_t H_Sample[4];
int H_Angle,V_Angle,H_Average,V_Average,H_Pos,V_Pos,i = 0;
/*--------------------------------------------CONFIGURACION ADC-----------------------------------------------*/
void ADC(void){
    //TIMER
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE,TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE,TIMER_A,909);
    TimerControlTrigger(TIMER0_BASE, TIMER_A,true);
    TimerEnable(TIMER0_BASE, TIMER_A);
    //ADC1 MOVIMIENTO HORIZONTAL
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeADC(GPIO_PORTD_BASE,GPIO_PIN_1);
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_STRENGTH_12MA ,GPIO_PIN_TYPE_ANALOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
    ADCSequenceConfigure(ADC0_BASE,2,ADC_TRIGGER_TIMER,0);
    ADCSequenceStepConfigure(ADC0_BASE,2,0,ADC_CTL_CH6);
    ADCSequenceStepConfigure(ADC0_BASE,2,1,ADC_CTL_CH6);
    ADCSequenceStepConfigure(ADC0_BASE,2,2,ADC_CTL_CH6);
    ADCSequenceStepConfigure(ADC0_BASE,2,3,ADC_CTL_CH6|ADC_CTL_IE|ADC_CTL_END);
    ADCSequenceEnable(ADC0_BASE,2);
    //ADC2 MOVIMIENTO VERTICAL
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
    GPIOPinTypeADC(GPIO_PORTD_BASE,GPIO_PIN_0);
    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_0, GPIO_STRENGTH_12MA ,GPIO_PIN_TYPE_ANALOG);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
    ADCSequenceConfigure(ADC1_BASE,2,ADC_TRIGGER_TIMER,0);
    ADCSequenceStepConfigure(ADC1_BASE,2,0,ADC_CTL_CH7);
    ADCSequenceStepConfigure(ADC1_BASE,2,1,ADC_CTL_CH7);
    ADCSequenceStepConfigure(ADC1_BASE,2,2,ADC_CTL_CH7);
    ADCSequenceStepConfigure(ADC1_BASE,2,3,ADC_CTL_CH7|ADC_CTL_IE|ADC_CTL_END);
    ADCSequenceEnable(ADC1_BASE,2);
}
/*--------------------------------------------CONFIGURACION PWM-----------------------------------------------*/
void PWM(void){
    //PWM MOVIMIENTO HORIZONTAL
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_4);
    GPIOPinConfigure(GPIO_PC4_M0PWM6);
    //PWM MOVIMIENTO VERTICAL
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    GPIOPinTypePWM(GPIO_PORTC_BASE, GPIO_PIN_5);
    GPIOPinConfigure(GPIO_PC5_M0PWM7);
}
void PWM_Init(void){
    //PWM MOVIMIENTO HORIZONTAL
    T_cycles =  (SysCtlClockGet()/32)/F_Hz;
    PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, T_cycles);
    PWMOutputState(PWM0_BASE, PWM_OUT_6_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_3);
    //PWM MOVIMIENTO VERTICAL
    T_cycles =  (SysCtlClockGet()/32)/F_Hz;
    PWMGenConfigure(PWM0_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_3, T_cycles);
    PWMOutputState(PWM0_BASE, PWM_OUT_7_BIT, true);
    PWMGenEnable(PWM0_BASE, PWM_GEN_3);

}
/*--------------------------------------------CONFIGURACION UART-----------------------------------------------*/
void Config_UART(void){
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1); //Habilitar Módulo 1 del periférico UART
        GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0|GPIO_PIN_1); //Configurar pines 0 y 1 de Puerto B como tipo UART
        GPIOPinConfigure(GPIO_PB0_U1RX); //Configurar PB0 como RX
        GPIOPinConfigure(GPIO_PB1_U1TX); //Configurar PB1 como TX
        UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(), 9600, (UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));
}
/*-------------------------------------------------MAIN--------------------------------------------------------*/

int main(void){
SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_5); //Reloj de Tiva C a 40MHz
PWM();
ADC();
Config_UART();
PWM_Init();

 while(1){
/*_____________________________________________SERVO HORIZONTAL_______________________________________________*/

     ADCSequenceDataGet(ADC0_BASE,2,H_Sample);
         if (H_Angle > 179){
             //H_Pos = 7;
             H_Pos = 8;
            }else{
                H_Pos = (H_Angle*0.1) -10;
                //H_Pos = (H_Angle*0.1) -10;
            }
         H_Average = (H_Sample[0] + H_Sample[1] + H_Sample[2] + H_Sample[3])/4;
         H_Angle = (0.01954*H_Average)+100;
         PWMPulseWidthSet(PWM0_BASE, PWM_OUT_6, H_HighTime);
         H_HighTime = (125*(H_Angle*4+180))/72;

/*_____________________________________________SERVO VERTICAL_______________________________________________*/

         ADCSequenceDataGet(ADC1_BASE,2,V_Sample);
            if (V_Angle > 179){
                //V_Pos = 7;
                V_Pos = 8;
               }else{
                   V_Pos = (0.133333344*(V_Angle-100)) + 1;
                   //V_Pos = 0.133333344*(V_Angle-100);
               }
            V_Average = ( V_Sample[0] +V_Sample[1] + V_Sample[2] + V_Sample[3])/4;
            V_Angle = (0.014652014*V_Average)+100;
            PWMPulseWidthSet(PWM0_BASE, PWM_OUT_7, V_HighTime);
            V_HighTime = (125*(V_Angle*4+180))/72;
/*_____________________________________________ENVIO DE DATOS POR UART_______________________________________________*/

                for(i = 0; i < 10; i++) { //Enviar texto de "POSICION: "
                  UARTCharPut(UART1_BASE, TextOne[i]);
                    }
                  UARTCharPut(UART1_BASE, (65+(7 - H_Pos)));
                  UARTCharPut(UART1_BASE, (48+V_Pos));   //Equivalente en ASCII
                  UARTCharPut(UART1_BASE, '\n');       //Línea nueva
                  UARTCharPut(UART1_BASE, '\r');       //Retornar cursor

 }
}
