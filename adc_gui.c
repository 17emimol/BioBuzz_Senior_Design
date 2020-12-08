#define delayMicroseconds bcm_delayMicro
#include <bcm2835.h>
#undef delayMicroseconds
#define delayMicroseconds w_delayMicro
#include <wiringPi.h>
#undef delayMicroseconds
#include <stdio.h>
#include <gtk/gtk.h>

#define PIN RPI_GPIO_P1_12
#define PWM_CHANNEL 0
#define RANGE 1024
#define COMPARE1 23
#define COMPARE2 24
#define HEATER 8
#define delay_length 50

float scale;

struct Timer_Data {
    GtkWidget *lbl;
    GtkWidget *lbl2;
    GtkWidget *lbl3;
};

void setupADC();
float calibrateADC();
float readADC(float scale);
void terminateADC(void);
void end_program(GtkWidget *wid, gpointer ptr);
gboolean timer_handler(gpointer ptr);

int main(int argc, char **argv)
{
    setupADC();
    scale = calibrateADC();
	gtk_init(&argc, &argv);
    struct Timer_Data Data;
    Data.lbl = gtk_label_new("Voltage");
    Data.lbl2 = gtk_label_new("Heater");
    Data.lbl3 = gtk_label_new("Temperature");
	GtkWidget *win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    GtkWidget *box = gtk_vbox_new(FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), Data.lbl, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), Data.lbl3, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), Data.lbl2, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(win), box);
    g_timeout_add(3000, (GSourceFunc) timer_handler, (gpointer) &Data);
    g_signal_connect(win, "delete_event", G_CALLBACK(end_program), NULL);
    gtk_window_set_title(GTK_WINDOW(win), "ADC GUI");
	gtk_widget_show_all(win);
	gtk_main();
	return 0;
}

void setupADC()
{
    if (!bcm2835_init())
        printf("ADC failed to initialize");

    // PWM Setup
    bcm2835_gpio_fsel(PIN, BCM2835_GPIO_FSEL_ALT5);
    bcm2835_pwm_set_clock(BCM2835_PWM_CLOCK_DIVIDER_16);
    bcm2835_pwm_set_mode(PWM_CHANNEL, 1, 1);
    bcm2835_pwm_set_range(PWM_CHANNEL, RANGE);
    
    // GPIO Setup
    wiringPiSetupGpio();
    pinMode(COMPARE1, INPUT);
    pinMode(COMPARE2, INPUT);
    
    pinMode(HEATER, INPUT);
}

float calibrateADC()
{
    // Calibration
    printf("Calibrating...\r\n");
    int upper = 1023;
    int lower = 0;
    int test;
    float scale;
    while(1)
    {
        test = lower + ((upper - lower) / 2);
        if(lower == upper)
        {
            scale = 1.225 / upper;
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
    return scale;
}

float readADC(float scale)
{
    // Measurement
    int upper = 1023;
    int lower = 0;
    int test;
    float voltage_buf[3];
    float voltage;
    for(int i = 0; i < 3; i++)
    {
        while(1)
        {
            test = lower +((upper - lower) / 2);
            if(lower == upper)
            {
                voltage_buf[i] =  scale * upper;
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
    voltage = (voltage_buf[0] + voltage_buf[1] + voltage_buf[2]) / 3;
    return voltage;
}

void terminateADC(void)
{
    bcm2835_close();
}

void end_program(GtkWidget *wid, gpointer ptr)
{
	gtk_main_quit();
}

gboolean timer_handler(gpointer ptr)
{
    struct Timer_Data *Data = ptr;
    char v_buffer[30];
    char h_buffer[30];
    char t_buffer[30];
    float v = readADC(scale);
    float t = v*100;
    int h = digitalRead(HEATER);
    if(h == 1)
    {
        sprintf(h_buffer, "Heater: On");
    }else{
        sprintf(h_buffer, "Heater: Off");
    }
    sprintf(v_buffer, "Voltage: %.4f V", v);
    sprintf(t_buffer, "Temperature: %.0f C", t);
	gtk_label_set_text(GTK_LABEL(Data->lbl), v_buffer);
    gtk_label_set_text(GTK_LABEL(Data->lbl2), h_buffer);
    gtk_label_set_text(GTK_LABEL(Data->lbl3), t_buffer);
    return TRUE;
}
