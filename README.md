# PIC18F4580-Closed-Loop-Temperature-Controller
An industrial smart cooling system using the PIC18F4580 microcontroller that monitors temperature through an LM35 sensor, controls a DC cooling fan via an L293D motor driver, displays real-time diagnostics on a 16×2 LCD, and performs automatic over-temperature shutdown for equipment protection.
# PIC18F4580 Closed-Loop Temperature Controller

## 💡 Overview

The **Closed-Loop Temperature Controller** is an embedded control system developed using the **PIC18F4580 microcontroller**. The system continuously monitors temperature using an **LM35 temperature sensor**, displays real-time temperature readings on a **16×2 LCD**, controls a **DC cooling fan** through an **L293D motor driver**, and provides automatic thermal protection through an overheat shutdown mechanism.

This project demonstrates practical implementation of:

* Analog-to-Digital Conversion (ADC)
* Sensor Interfacing
* LCD Interfacing (8-bit Mode)
* Motor Control
* Temperature Monitoring
* Industrial Cooling Control
* Safety Shutdown Systems

---

## 🎯 Project Objectives

* Monitor temperature continuously using an LM35 sensor.
* Display real-time temperature readings on an LCD.
* Allow manual Start/Stop control of the cooling system.
* Automatically operate a cooling fan.
* Detect over-temperature conditions.
* Shut down the system safely when overheating occurs.
* Provide visual status indicators using LEDs.

---

## 🚀 Features

### 🌡️ Temperature Monitoring

* LM35 Analog Temperature Sensor
* 10-bit ADC Conversion
* Real-Time Temperature Calculation

### 🖥️ LCD Display

* Current Temperature Display
* Fan Status Monitoring
* Overheat Warning Messages

### 🌀 Cooling Fan Control

* L293D Motor Driver Interface
* Manual Start/Stop Operation
* Automatic Shutdown Protection

### 🚨 Safety Features

* Overheat Detection (45°C Threshold)
* Warning LED Flashing
* Emergency Fan Shutdown

---

## 🛠️ Hardware Requirements

| Component                  | Quantity |
| -------------------------- | -------- |
| PIC18F4580 Microcontroller | 1        |
| LM35 Temperature Sensor    | 1        |
| 16×2 LCD Display           | 1        |
| L293D Motor Driver         | 1        |
| DC Fan Motor               | 1        |
| Push Button                | 1        |
| Green LED (RUN)            | 1        |
| Red LED (WARNING)          | 1        |
| 220Ω Resistors             | 2        |
| 10kΩ Pull-Up Resistor      | 1        |
| 20 MHz Crystal Oscillator  | 1        |
| 22pF Capacitors            | 2        |
| 5V Power Supply            | 1        |

---

## 🔌 Pin Connections

### LM35 Sensor

| PIC Pin   | Function    |
| --------- | ----------- |
| RA0 / AN0 | LM35 Output |

### Push Button

| PIC Pin | Function             |
| ------- | -------------------- |
| RB0     | Start / Stop Control |

### LEDs

| PIC Pin | Function    |
| ------- | ----------- |
| RB4     | RUN LED     |
| RB5     | WARNING LED |

### Motor Driver

| PIC Pin | Function  |
| ------- | --------- |
| RB1     | L293D IN1 |
| RB2     | L293D IN2 |

### LCD Interface

| LCD Pin | PIC18F4580 Pin |
| ------- | -------------- |
| RS      | RC0            |
| EN      | RC1            |
| D0-D7   | PORTD          |

---

## ⚙️ Operating States

### 1️⃣ Standby Mode

Conditions:

* Fan disabled
* System idle

LCD:

```text
Temp: XX°C
FAN: STOPPED
```

Outputs:

* Fan OFF
* RUN LED OFF
* WARNING LED OFF

---

### 2️⃣ Cooling Mode

Conditions:

* System enabled
* Temperature below 45°C

LCD:

```text
Temp: XX°C
FAN: RUNNING
```

Outputs:

* Fan ON
* RUN LED ON
* WARNING LED OFF

---

### 3️⃣ Overheat Protection

Conditions:

```text
Temperature ≥ 45°C
```

LCD:

```text
Temp: XX°C
ALERT: OVERHEAT
```

Outputs:

* Fan OFF
* RUN LED OFF
* WARNING LED Flashing

---

## 🧮 Temperature Conversion

The LM35 outputs:

```text
10mV per °C
```

ADC conversion formula:

```c
temp_c = ((unsigned long)adc_value * 500) / 1023;
```

Where:

```text
Voltage = (ADC × 5000) / 1023
Temperature = Voltage / 10
```

---

## 🔄 Control Flow

```text
START
   |
Initialize System
   |
Read Temperature
   |
Display Temperature
   |
Temperature >= 45°C ?
   |
 YES -----------------> OVERHEAT SHUTDOWN
   |                           |
 NO                            |
   |                           |
Motor Active ?                 |
   |                           |
 YES --> RUN FAN               |
   |                           |
 NO --> STOP FAN <-------------+
   |
Repeat
```

---

## 📂 Source Code

```c
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

void delay_ms(unsigned int ms);
void lcd_command(unsigned char cmd);
void lcd_data(unsigned char data);
void lcd_init();
void lcd_print(const char* str);
void adc_init();
unsigned int adc_read();

void delay_ms(unsigned int ms) {
    unsigned int i;
    for(i = 0; i < ms; i++) {
        for(int j = 0; j < 165; j++);
    }
}

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
    lcd_command(0x38);
    lcd_command(0x0C);
    lcd_command(0x06);
    lcd_command(0x01);
    delay_ms(5);
}

void lcd_print(const char* str) {
    while (*str) {
        lcd_data(*str++);
    }
}

void adc_init() {
    TRISAbits.TRISA0 = 1;
    ADCON1 = 0x0E;
    ADCON2 = 0x92;
    ADCON0 = 0x01;
}

unsigned int adc_read() {
    delay_ms(1);
    GODONE = 1;
    while (GODONE == 1);
    return ((ADRESH << 8) + ADRESL);
}

void main(void) {

    TRISD = 0x00;
    TRISC = 0x00;
    TRISB = 0x01;

    ADCON1 = 0x0E;

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

        if (PORTBbits.RB0 == 0) {
            delay_ms(50);

            if (PORTBbits.RB0 == 0) {
                motor_active = !motor_active;
                lcd_command(0x01);

                while(PORTBbits.RB0 == 0);
            }
        }

        adc_value = adc_read();
        temp_c = ((unsigned long)adc_value * 500) / 1023;

        lcd_command(0x80);
        lcd_print("Temp: ");

        buffer[0] = (temp_c / 100) + '0';
        buffer[1] = ((temp_c / 10) % 10) + '0';
        buffer[2] = (temp_c % 10) + '0';
        buffer[3] = 0xDF;
        buffer[4] = 'C';
        buffer[5] = '\0';

        lcd_print(buffer);

        lcd_command(0xC0);

        if (temp_c >= 45) {

            motor_active = 0;

            MOTOR_IN1 = 0;
            MOTOR_IN2 = 0;

            RUN_LED = 0;

            lcd_print("ALERT: OVERHEAT ");

            WARN_LED = 1;
            delay_ms(150);

            WARN_LED = 0;
            delay_ms(150);
        }
        else if (motor_active) {

            WARN_LED = 0;
            RUN_LED = 1;

            MOTOR_IN1 = 1;
            MOTOR_IN2 = 0;

            lcd_print("FAN: RUNNING    ");

            delay_ms(250);
        }
        else {

            WARN_LED = 0;
            RUN_LED = 0;

            MOTOR_IN1 = 0;
            MOTOR_IN2 = 0;

            lcd_print("FAN: STOPPED    ");

            delay_ms(250);
        }
    }
}
```

---


## Circuit Diagram
![image alt](

---

## 🏭 Applications

* Industrial Cooling Systems
* Electronic Equipment Protection
* HVAC Monitoring
* Smart Ventilation Systems
* Server Rack Cooling
* Embedded Control Systems

---

## ⚠️ Limitations

* Fixed temperature threshold
* Single temperature sensor
* No PWM fan speed control
* No data logging

---

## 🔮 Future Enhancements

* PWM Fan Control
* Multiple Sensors
* UART Monitoring
* IoT Dashboard Integration
* PID Temperature Control
* Wireless Monitoring

---


## 🙏 Acknowledgments

* PIC18F4580 Microcontroller
* MPLAB X IDE
* XC8 Compiler
* Proteus Design Suite
* LM35 Temperature Sensor
* L293D Motor Driver

---

## 📚 Resources

* PIC18F4580 Datasheet
* LM35 Datasheet
* L293D Datasheet
* MPLAB X IDE Documentation
* XC8 Compiler Documentation

```
```

