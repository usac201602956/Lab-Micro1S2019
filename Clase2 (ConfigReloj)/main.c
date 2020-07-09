#include<stdint.h>
#include<stdbool.h>
#include"inc/tm4c123gh6pm.h"
#include"inc/hw_types.h"
#include"driverlib/sysctl.h"

int r;
/*-------------------------------------------MAIN-----------------------------------------------*/
int main(void)
{
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_2_5);
    r = SysCtlClockGet();

    while(1);
}
