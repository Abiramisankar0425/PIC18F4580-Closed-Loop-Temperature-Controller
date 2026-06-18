#include <xc.h>

// Configuration Bits (20MHz oscillator, Watchdog disabled, LVP disabled)
#pragma config OSC = HS
#pragma config WDT = OFF
#pragma config LVP = OFF

// LCD Pin Mapping
#define LCD_RS LATCbits.LATC0
#define LCD_EN LATCbits.LATC1
#define LCD_DATA LATD

// LED Pin Mapping
#define RUN_LED LATBbits.LATB4
#define WARN_LED LATBbits.LATB5

// Motor Control Pins
#define MOTOR_IN1 LATBbits.LATB1
#define MOTOR_IN2 LATBbits.LATB2

// Software Delay Function
void delay_ms(unsigned int ms) {
    unsigned int i;
    for(i = 0; i < ms; i++) {
        for(int j = 0; j < 165; j++);
    }
}

// LCD Commands
void lcd_command(unsigned char cmd) {
    LCD_DATA = cmd;
    LCD_RS = 0;
    LCD_EN = 1;
    delay_ms(2);
    LCD_EN = 0;
}

void lcd_data(unsigned char data) {
    LCD_DATA = data;
    LCD_RS = 1;
    LCD_EN = 1;
    delay_ms(2);
    LCD_EN = 0;
}

void lcd_init() {
    delay_ms(15);
    lcd_command(0x38); // 8-bit mode, 2-line display
    lcd_command(0x0C); // Display ON, Cursor OFF
    lcd_command(0x06); // Auto-increment cursor
    lcd_command(0x01); // Clear display
    delay_ms(5);
}

void lcd_print(const char* str) {
    while (*str) {
        lcd_data(*str++);
    }
}

// ADC Module Initialization
void adc_init() {
    TRISAbits.TRISA0 = 1; // Configure RA0/AN0 pin as input
    
    // Configure AN0 as Analog, others as Digital
    ADCON1 = 0x0E; 

    // Right Justified result, 4 TAD acquisition time, FOSC/32 clock
    ADCON2 = 0x92;

    // Select Channel 0 (AN0), Power ON ADC
    ADCON0 = 0x01; 
}

// Read Analog Value
unsigned int adc_read() {
    delay_ms(1);
    GODONE = 1;  // Start conversion
    while (GODONE == 1); 
    return ((ADRESH << 8) + ADRESL);
}

// Main Program
void main(void) {
    // Port configurations
    TRISD = 0x00; // PORTD as output (LCD Data Bus)
    TRISC = 0x00; // PORTC as output (LCD Controls RS/EN)
    TRISB = 0x01; // RB0 as Input (Start/Stop button), RB1-RB5 as outputs (Motors & LEDs)
    
    // Digital mode for analog pins (except AN0 configured in adc_init)
    ADCON1 = 0x0E; 
    
    // Clear outputs
    LATB = 0x00;
    LATD = 0x00;

    lcd_init();
    adc_init();

    lcd_print("COOLING CONTROL");
    lcd_command(0xC0);
    lcd_print("SYSTEM ACTIVE");
    delay_ms(2000);
    lcd_command(0x01);

    unsigned int adc_value;
    unsigned int temp_c;
    unsigned char motor_active = 0;
    char buffer[16];

    while (1) {
        // Handle Start/Stop manual button on RB0 (active low with pull-up)
        if (PORTBbits.RB0 == 0) {
            delay_ms(50); // Debounce
            if (PORTBbits.RB0 == 0) {
                motor_active = !motor_active; // Toggle system running state
                lcd_command(0x01);
                while(PORTBbits.RB0 == 0); // Wait for button release
            }
        }

        // Read sensor voltage and scale to temperature (10mV / Degree C, e.g. LM35)
        adc_value = adc_read();
        
        // Vout (mV) = (ADC * 5000) / 1023
        // Temp (C) = Vout / 10
        temp_c = ((unsigned long)adc_value * 500) / 1023;

        // Display current sensor value
        lcd_command(0x80);
        lcd_print("Temp: ");
        buffer[0] = (temp_c / 100) + '0';
        buffer[1] = ((temp_c / 10) % 10) + '0';
        buffer[2] = (temp_c % 10) + '0';
        buffer[3] = 0xDF; // Degree symbol
        buffer[4] = 'C';
        buffer[5] = '\0';
        lcd_print(buffer);

        lcd_command(0xC0);
        if (temp_c >= 45) {
            // Over-temperature Warning Trigger
            motor_active = 0; // Emergency shutdown
            MOTOR_IN1 = 0;
            MOTOR_IN2 = 0;
            RUN_LED = 0;
            
            lcd_print("ALERT: OVERHEAT ");
            
            // Flashing warning strobe
            WARN_LED = 1;
            delay_ms(150);
            WARN_LED = 0;
            delay_ms(150);
        }
        else if (motor_active) {
            // Normal operating cooling state
            WARN_LED = 0;
            RUN_LED = 1;
            
            // Drive cooling fan motor forward
            MOTOR_IN1 = 1;
            MOTOR_IN2 = 0;
            lcd_print("FAN: RUNNING    ");
            delay_ms(250);
        }
        else {
            // Standby state
            WARN_LED = 0;
            RUN_LED = 0;
            MOTOR_IN1 = 0;
            MOTOR_IN2 = 0;
            lcd_print("FAN: STOPPED    ");
            delay_ms(250);
        }
    }
}
