//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
// This project pulses the IR diode and waits for the detector to go low,
// then an interrupt is fired on the falling edge and an LED turns on.
//****************************************************************************

#include "msp.h"
#include "driverlib.h"

#define TIMER_PERIOD    0x0300

uint16_t count;

const Timer_A_UpModeConfig upConfig =
{
        TIMER_A_CLOCKSOURCE_SMCLK,              // SMCLK Clock Source
        TIMER_A_CLOCKSOURCE_DIVIDER_64,          // SMCLK/1 = 3MHz
        TIMER_PERIOD,                           // 5000 tick period
        TIMER_A_TAIE_INTERRUPT_DISABLE,         // Disable Timer interrupt
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE ,    // Enable CCR0 interrupt
        TIMER_A_DO_CLEAR                        // Clear value
};

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

    count = 0;

    GPIO_setAsOutputPin(GPIO_PORT_P6, GPIO_PIN0); //IR diode
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN0); //LED
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN0);  // Set detector input

    GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0); //set IR diode off intially
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0); //set LED off intially

    Timer_A_configureUpMode(TIMER_A0_BASE, &upConfig); //configure timer to trigger interrupt every TIMER_PERIOD clk cycles

    Interrupt_enableInterrupt(INT_TA0_0); //enables timer interrupt
    Interrupt_enableInterrupt(INT_PORT5);   //enable port5 interrupts
    GPIO_enableInterrupt(GPIO_PORT_P5, GPIO_PIN0);  //enables interrupt for detector
    GPIO_interruptEdgeSelect(GPIO_PORT_P5, GPIO_PIN0, GPIO_HIGH_TO_LOW_TRANSITION); //Trigger on falling edge of detector signal

    Timer_A_startCounter(TIMER_A0_BASE, TIMER_A_UP_MODE); //starts timer A
    __enable_irq();
    while(1);
}


void TA0_0_IRQHandler(void) //Pulse IR diode
{
    int i = 0;
    for(i=0; i<20; i++){
        GPIO_setOutputHighOnPin(GPIO_PORT_P6, GPIO_PIN0);
        __delay_cycles(10);
        GPIO_setOutputLowOnPin(GPIO_PORT_P6, GPIO_PIN0);
    }
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
            TIMER_A_CAPTURECOMPARE_REGISTER_0);
}


void PORT5_IRQHandler(){ //waits for falling edge of detector
    count++;
    if(count > 1){
        int i = 0;
        for(i=0; i<20; i++){
            GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN0);
            __delay_cycles(5000); //Create a 1 ms pulse
            GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN0);
            __delay_cycles(150000); //About an 8ms delay between pulses
        }
    }
    GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN0);
}
