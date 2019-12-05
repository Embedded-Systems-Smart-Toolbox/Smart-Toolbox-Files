#include <msp430.h> 


#define ENABLE_PINS 0xFFFE // Enables inputs and outputs

unsigned int ADC_value0 = 0;
unsigned int ADC_value1 = 0;

#include <driverlib.h> // Required for the LCD
#include "myGpio.h" // Required for the LCD
#include "myClocks.h" // Required for the LCD
#include "myLcd.h" // Required for the LCD

/**
 * main.c
 */

/*
 * 1.4  button 1
 * 1.5  button 2
 * 1.6  button 3
 * 1.7  button 4
 *
 * 4.7  solenoid / LED string
 *
 * 8.1  buzzer
 *
 * 2.7  LCD button
 *
 * 9.1  force resistor 1
 * 9.2  force resistor 2
 *
 */

void ScrollWords(char words[250])
{
unsigned int length; // Contains length of message to be displayed
unsigned int slot; // Slot to be displayed on LCD (1, 2, 3, 4, 5, or 6)
unsigned int amount_shifted; // Number of times message shifted so far
unsigned int offset; // Used with amount_shifted to get correct character to display
unsigned long delay; // Used to implement delay between scrolling iterations
unsigned char next_char; // Next character from message to be displayed
length = strlen(words); // Get the length of the message stored in words
amount_shifted=0; // We have not shifted the message yet
offset=0; // There is no offset yet
while( amount_shifted < length+7 ) // Loop as long as you haven't shifted all
{ // of the characters off the LCD screen
offset=amount_shifted; // Starting point in message for next LCD update
for(slot = 1;slot<=6;slot++) // Loop 6 times to display 6 characters at a time
{
next_char = words[offset-6]; // Get the current character for LCD slot
if(next_char && (offset>=6) && (offset<=length+6) ) // If character is not null AND
{ // LCD is not filled (offset>=6) AND
// You have not reached end of message
// (offset<=length+6)
myLCD_showChar(next_char,slot); // Show the next character on the LCD
// screen in correct slot
}
else // Else, slot on LCD should be blank
{
myLCD_showChar(' ',slot); // So, add a blank space to slot
}
offset++; // Update as you move across the message
}
for(delay=0 ; delay<123456 ; delay=delay+1); // Delay between shifts
amount_shifted = amount_shifted + 1; // Update times words shifted across LCD
}
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)           //button interrupt
{
    if ((P2IN & BIT7) != BIT7)
    {
        if (((P4IN & BIT2) != BIT2) && ((P4IN & BIT3) != BIT3))
        {
            ScrollWords("SCREWDRIVER HERE HAMMER HERE");
        }
        else if (((P4IN & BIT2) == BIT2) && ((P4IN & BIT3) != BIT3))
        {
            ScrollWords("SCREWDRIVER NOT HERE HAMMER HERE");
        }
        else if (((P4IN & BIT2) != BIT2) && ((P4IN & BIT3) == BIT3))
        {
            ScrollWords("SCREWDRIVER HERE HAMMER NOT HERE");
        }
        else
        {
            ScrollWords("SCREWDRIVER NOT HERE HAMMER NOT HERE");
        }
    }
}


void timer_setup(void)
{
    TA0CCR0 = 1000;       //Duty Cycle @ 100%
    TA0CCTL1 = OUTMOD_7;  //Sets output mode for timer
    TA0CCR1 = 0;       //Duty Cycle @ 0%
    TA0CTL = TASSEL_2 + MC_1; //Timer Instantiation

}
void lock_control(void)
{
    P8DIR |= BIT1;                          //Sets P1.0 as an output
    P8SEL0 |= BIT1;                         //Sets P1.0 to select PWM

    P1REN |= BIT4;                          // enable resistor
    P1OUT |= BIT4;                          // set P8.4 to input

    P1REN |= BIT5;                          // enable resistor
    P1OUT |= BIT5;                          // set P8.5 to input

    P1REN |= BIT6;                          // enable resistor
    P1OUT |= BIT6;                          // set P8.6 to input

    P1REN |= BIT7;                          // enable resistor
    P1OUT |= BIT7;                          // set P8.7 to input

    P1IE |= BIT4;                           //enable interrupt
    P1IES |= BIT4;                          //enable interrupt on rising edge
    P1IFG &= ~BIT4;                         //clear IFG

    P1IE |= BIT5;                           //enable interrupt
    P1IES |= BIT5;                          //enable interrupt on rising edge
    P1IFG &= ~BIT5;                         //clear IFG

    P1IE |= BIT6;                           //enable interrupt
    P1IES |= BIT6;                          //enable interrupt on rising edge
    P1IFG &= ~BIT6;                         //clear IFG

    P1IE |= BIT7;                           //enable interrupt
    P1IES |= BIT7;                          //enable interrupt on rising edge
    P1IFG &= ~BIT7;                         //clear IFG

    P4OUT &= ~BIT7;                          //lock

    __enable_interrupt();


}
void ADC_SETUP(void)
{
    P9SEL1 |= BIT1; // Configure P9.1 for ADC
    P9SEL0 |= BIT1;

    P9SEL1 |= BIT2; // Configure P9.2 for ADC
    P9SEL0 |= BIT2;

 #define ADC12_SHT_16 0x0200 // 16 clock cycles for sample and hold
 #define ADC12_ON 0x0010 // Used to turn ADC12 peripheral on
 #define ADC12_SHT_SRC_SEL 0x0200 // Selects source for sample & hold
 #define ADC12_12BIT 0x0020 // Selects 12-bits of resolution
 #define ADC12_P92 0x000A // Use input P9.2 for analog input
#define ADC12_P91  0x0009 //Use input P9.1 for second input
 ADC12CTL0 = ADC12_SHT_16 | ADC12MSC | ADC12_ON ; // Turn on, set sample & hold time
 ADC12CTL1 = ADC12CONSEQ1 + ADC12_SHT_SRC_SEL; // Specify sample & hold clock source
 ADC12CTL2 = ADC12_12BIT; // 12-bit conversion results
 ADC12MCTL0 = ADC12_P92; // P9.2 is analog input
 ADC12MCTL1 = ADC12_P91 | ADC12EOS; //P9.1 second input, end of sequence
 ADC12IER0 |= ADC12IE1; // Enable ADC interrupt on MEM1

}

#pragma vector=PORT1_VECTOR
__interrupt void Por1_1(void)           //button interrupt
{
    if (((P1IN & BIT6) != BIT6) && ((P1IN & BIT5) != BIT5) && ((P1IN & BIT7) == BIT7) && ((P1IN & BIT3) == BIT3)) // if button only 1 and 4 are pressed
    {
        P4OUT |= BIT7;                          //unlock and lights on
        P1IFG &= ~BIT4;                         //clear IFG
        P1IFG &= ~BIT5;                         //clear IFG
        P1IFG &= ~BIT6;                         //clear IFG
        P1IFG &= ~BIT7;                         //clear IFG
        __delay_cycles(3000000);                 //delay before locking again
        P4OUT &= ~BIT7;
    }
    else
    {
        __delay_cycles(1000000);
        TA0CCR1 = 1000;                          //buzzer on
        __delay_cycles(100000);
        TA0CCR1 = 0;                         //buzzer off
    }

    if ((P2IN & BIT7) != BIT7)
    {
        if (((P9IN & BIT2) != BIT2) && ((P9IN & BIT1) != BIT1))
        {
            ScrollWords("SCREWDRIVER HERE HAMMER HERE");
        }
        else if (((P9IN & BIT2) == BIT2) && ((P9IN & BIT1) != BIT1))
        {
            ScrollWords("SCREWDRIVER NOT HERE HAMMER HERE");
        }
        else if (((P9IN & BIT2) != BIT2) && ((P9IN & BIT1) == BIT1))
        {
            ScrollWords("SCREWDRIVER HERE HAMMER NOT HERE");
        }
        else
        {
            ScrollWords("SCREWDRIVER NOT HERE HAMMER NOT HERE");
        }
    }
}


int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    PM5CTL0 = ENABLE_PINS; // Enable inputs and outputs
    ADC_SETUP(); // Sets up ADC peripheral
    timer_setup();                          //instantiates timer_setup
    lock_control();                         //instantiate lock_control function

    while(1)
    {
            ADC12CTL0 |= ADC12ENC;
            ADC12CTL0 = ADC12CTL0 | ADC12SC; // Start conversion
            ADC12CTL1 |= ADC12ENC;
            ADC12CTL1 = ADC12CTL1 | ADC12SC; // Start conversion
            ADC_value0 = ADC12MEM0; // Save MEM0
            ADC_value1 = ADC12MEM1; // Save MEM1

    }
}
