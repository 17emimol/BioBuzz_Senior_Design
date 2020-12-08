/*
 * Uses a binary search algorithm to find the correct PWM duty cycle.
 * Calibrates using the voltage reference connected to a separate op amp
 * and then measures the temperature sensor.
*/


#define delayMicroseconds bcm_delayMicro
#include <bcm2835.h>
#undef delayMicroseconds
#define delayMicroseconds w_delayMicro
#include <wiringPi.h>
#undef delayMicroseconds
#include <stdio.h>
#include <math.h>

#define PIN RPI_GPIO_P1_12
#define PWM_CHANNEL 0
#define RANGE 1024
#define COMPARE1 23
#define COMPARE2 24
#define delay_length 200
float scale;
float voltage;
 
int main(int argc, char **argv)
{
    if (!bcm2835_init())
        return 1;

    // PWM Setup
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_ALT5);
    bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16);
    bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
    bcm2835_pwm_set_range(PWM_CHANNEL, RANGE);
    
    // GPIO Setup
    wiringPiSetupGpio();
    pinMode(COMPARE1, INPUT);
    pinMode(COMPARE2, INPUT);
 
    // Calibration
    printf("Calibrating...\r\n");
    int upper = 1023;
    int lower = 0;
    int test;
    while(1)
    {
        test = lower + round((upper - lower) / 2);
        if(lower == upper)
        {
            scale = 1.225 / upper;
            printf("Calibration Factor: %f\r\n", scale);
            break;
        }
        bcm2835_pwm_set_data(PWM_CHANNEL, test);
        bcm2835_delay(delay_length);
        if(digitalRead(COMPARE1) == 0)
        {
            upper = test;
        } else {
            lower = test;
        }
    }
    
    
    // Measurement
    printf("Measuring Voltage...\r\n");
    while(1)
    {
        upper = 1023;
        lower = 0;
        test;
        while(1)
        {
            test = lower +((upper - lower) / 2);
            if(lower == upper)
            {
                voltage = scale * upper;
                printf("%i, %f V\r\n", upper, voltage);
                bcm2835_delay(delay_length);
                break;
            }
            bcm2835_pwm_set_data(PWM_CHANNEL, test);
            bcm2835_delay(delay_length);
            if(digitalRead(COMPARE2) == 0)
            {
                upper = test;
            } else {
                lower = test;
            }
        }
    }
    
    bcm2835_close();
    return 0;
}
