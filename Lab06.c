/*
 * Lab06.c
 *
 * Created: 10/4/2022 9:16:57 AM
 * Initial Coder: jfhutton
 * Current Coder: Sammy Mohamed
 * Modified:      10/04/2026
 *
 * This lab uses hardware LEDs wired to PORTA, an interrupt from the joystick center
 * button, and a timer interrupt to build a game loop program that can display
 * different patterns on the LEDs.
 *
 * While there are many "down and dirty" ways to get this coding done, try to
 * remember your coding and data structures classes. Things like ENUM, Arrays, Functions
 * could help make for more elegant coding.
 *
 */

#include <avr/io.h>          // lets me use the AVR registers
#include <avr/interrupt.h>   // lets me use interrupts

#define LEDS PORTA           // calling PORTA LEDS so its easier to read


// global variables used by main and the interrupts
const unsigned char TCNT0_COUNT_SET = 0x8E; // timer start value Prof Hutton gave us


// the 4 LED patterns
enum patterns
{
    RightToLeft,       // A0 to A7
    LeftToRight,       // A7 to A0
    BackAndForth,      // A0 to A7 and back
    MyPattern          // my pattern
};


// tells main what pattern we are using
volatile enum patterns Mode;

// goes up once every 1ms
volatile unsigned char Tick;

// tells main that I changed the mode
volatile unsigned char modeChanged;


int main(void)
{
    // keeps track of what LED I am on
    unsigned char led = 0;

    // used for going back and forth
    // 1 = going up, -1 = going back down
    signed char direction = 1;


    // start the state machine
    Mode = RightToLeft;
    Tick = 0;
    modeChanged = 0;


    // LED setup
    DDRA = 0xFF;      // all PORTA pins are outputs

    // LEDs are active low
    // 1 = off, 0 = on
    LEDS = 0xFF;      // start with all LEDs off


    // Timer0 setup
    // /64 prescaler
    // TCNT0 starts at 0x8E
    // overflow is about every 1ms

    TCCR0 = (1 << CS02);        // start Timer0 with /64
    TCNT0 = TCNT0_COUNT_SET;    // load timer starting value
    TIMSK = (1 << TOIE0);       // enable Timer0 overflow interrupt


    // joystick setup
    // center button is PB0
    // jumper PB0 to PD0 so I can use INT0

    DDRB &= ~(1 << PB0);        // PB0 is input
    PORTB |= (1 << PB0);        // turn on pull-up

    DDRD &= ~(1 << PD0);        // PD0 is input for INT0


    // INT0 setup
    // button is active low, so release is low -> high

    EICRA = (1 << ISC01) | (1 << ISC00);   // rising edge
    EIMSK = (1 << INT0);                   // enable INT0


    // turn all interrupts on
    sei();


    // main game loop
    while (1)
    {
        // if I change modes, restart the new pattern clean
        if (modeChanged)
        {
            modeChanged = 0;
            Tick = 0;
            direction = 1;

            // this pattern needs to start at LED7
            if (Mode == LeftToRight)
            {
                led = 7;
            }
            else
            {
                led = 0;
            }

            // turn everything off before new pattern starts
            LEDS = 0xFF;
        }


        // Tick goes up every 1ms
        // 50 ticks = about 50ms
        if (Tick >= 50)
        {
            Tick = 0;      // start counting the next 50ms


            // check what pattern we are on
            switch (Mode)
            {
                // Pattern 1
                // A0 -> A1 -> ... -> A7
                case RightToLeft:

                    LEDS = (unsigned char)~(1 << led);

                    led++;

                    if (led > 7)
                    {
                        led = 0;
                    }

                    break;


                // Pattern 2
                // A7 -> A6 -> ... -> A0
                case LeftToRight:

                    LEDS = (unsigned char)~(1 << led);

                    if (led == 0)
                    {
                        led = 7;
                    }
                    else
                    {
                        led--;
                    }

                    break;


                // Pattern 3
                // A0 -> A7 then back to A0
                case BackAndForth:

                    LEDS = (unsigned char)~(1 << led);


                    // going toward LED7
                    if (direction == 1)
                    {
                        if (led == 7)
                        {
                            direction = -1;
                            led = 6;
                        }
                        else
                        {
                            led++;
                        }
                    }

                    // going back toward LED0
                    else
                    {
                        if (led == 0)
                        {
                            direction = 1;
                            led = 1;
                        }
                        else
                        {
                            led--;
                        }
                    }

                    break;


                // Pattern 4 - MyPattern
                // outside LEDs move toward the middle
                case MyPattern:

                    switch (led)
                    {
                        case 0:
                            LEDS = 0x7E;    // LED0 and LED7
                            break;

                        case 1:
                            LEDS = 0xBD;    // LED1 and LED6
                            break;

                        case 2:
                            LEDS = 0xDB;    // LED2 and LED5
                            break;

                        case 3:
                            LEDS = 0xE7;    // LED3 and LED4
                            break;
                    }

                    led++;

                    if (led > 3)
                    {
                        led = 0;
                    }

                    break;
            }
        }
    }
}


// Timer0 interrupt
// this happens about every 1ms
ISR(TIMER0_OVF_vect)
{
    Tick++;                      // another 1ms passed
    TCNT0 = TCNT0_COUNT_SET;     // reload timer for next 1ms
}


// joystick interrupt
// every time I release the center button,
// go to the next pattern
ISR(INT0_vect)
{
    switch (Mode)
    {
        case RightToLeft:
            Mode = LeftToRight;
            break;

        case LeftToRight:
            Mode = BackAndForth;
            break;

        case BackAndForth:
            Mode = MyPattern;
            break;

        case MyPattern:
            Mode = RightToLeft;
            break;
    }

    // tell main that the mode changed
    modeChanged = 1;
	}