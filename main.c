/**
    Generated Main Source File

    Company:
        Microchip Technology Inc.

    File Name:
        main.c

    Summary:
        This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

    Description:
        This header file provides implementations for driver APIs for all modules selected in the GUI.
        Generation Information :
            Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
            Device            :  PIC18F46K20
            Driver Version    :  2.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "mcc_generated_files/mcc.h"

uint32_t tick = 0;

void clock10ms();
uint8_t spi_read_register(uint8_t reg);
void spi_write_register(uint8_t reg, uint8_t data);

void main(void)
{
    // Initialize the device
    SYSTEM_Initialize();

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    TMR1_SetInterruptHandler(clock10ms);
    IO_RC0_SetDigitalOutput();
    IO_RC0_SetLow();

    IO_RD0_SetDigitalOutput();
    IO_RD0_SetHigh();

    IO_RB0_SetDigitalInput();

    TMR1_StartTimer();
    
    SPI_Enable();

    uint32_t tackle_tick = 0;
    uint32_t last_measurment = 0;
    uint32_t last_send = 0;

    // disable I2C
    spi_write_register(0x0D, 0x80);

    // set mode to measurment
    spi_write_register(0x16, 0x01);
    // uint8_t mode = spi_read_register(0x16);

    while (1)
    {
        if (tick % 100 == 0 && tick != last_send) 
        {
            last_send = tick;
            
            if (EUSART_is_rx_ready() && EUSART_Read() == 'R')
            {
                IO_RC0_SetHigh();
            }
        }
        
        // if a measurment is ready
        if (tick % 4 == 0 && tick != last_measurment)
        {
            last_measurment = tick;

            // get raw data
            int8_t x_acc = (int8_t)spi_read_register(0x06);
            int8_t y_acc = (int8_t)spi_read_register(0x07);
            int8_t z_acc = (int8_t)spi_read_register(0x08);

            // if greater than one g horizontal - 16 per g
            if (x_acc > 16 || x_acc < -16 || y_acc > 16 || y_acc < -16)
            {
                tackle_tick = tick;
                IO_RC0_SetHigh();
            } 
            else if (tick - tackle_tick > 100) 
            {
                IO_RC0_SetLow();
            }
        }
    }
}

uint8_t spi_read_register(uint8_t reg)
{
    IO_RD0_SetLow();
    SPI_ExchangeByte(reg << 1);
    uint8_t data = SPI_ExchangeByte(0);
    IO_RD0_SetHigh();
    return data;
}

void spi_write_register(uint8_t reg, uint8_t data)
{
    IO_RD0_SetLow();
    SPI_ExchangeByte(0x80 | (reg << 1));
    SPI_ExchangeByte(data);
    IO_RD0_SetHigh();
}

void clock10ms() {
    if (tick % 50 == 0)
    {
    }
    tick += 1;
}
