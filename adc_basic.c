/*
 * Uses a simple linear search to find the correct PWM duty cycle.
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
#define MUX_CTRL 23
#define COMPARE 24
#define delay_length 500
float scale;
float voltage;

int read(int pinNum);
 
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
    pinMode(MUX_CTRL, OUTPUT);
    pinMode(COMPARE, INPUT);
 
    // Calibration
    printf("Calibrating...\r\n");
    digitalWrite(MUX_CTRL, HIGH);
    for(int i = 0; i < 1024; i += 1)
    {
        bcm2835_pwm_set_data(PWM_CHANNEL, i);
        bcm2835_delay(delay_length);
        if(digitalRead(COMPARE) == 0)
        {
            scale = 1.225 / i;
            printf("Calibration Factor: %f\r\n", scale);
            break;
        }
    }
    
    // Measurement
    printf("Measuring Voltage...\r\n");
    digitalWrite(MUX_CTRL, LOW);
    while(1)
    {
        for(int i = 0; i < 1024; i += 1)
        {
            bcm2835_pwm_set_data(PWM_CHANNEL, i);
            bcm2835_delay(delay_length);
            if(digitalRead(COMPARE) == 0)
            {
                voltage = scale * i;
                printf("%i, %f V\r\n", i, voltage);
                break;
            }
        }
    }
    
    bcm2835_close();
    return 0;
}
