#pragma once
#include <avr/io.h>
#define I2C_PORT   PORTB
#define I2C_PIN    PINB
#define I2C_DDR    DDRB
#define I2C_SDA    _BV(1)
#define I2C_SCL    _BV(4)


void i2c_io_set_sda(uint8_t hi);
uint8_t i2c_io_get_sda(void);
void i2c_io_set_scl(uint8_t hi);
void i2c_init(void);
void i2c_scl_toggle(void);
void i2c_start(void);
void i2c_repstart(void);
void i2c_stop(void);
uint8_t i2c_put_u08(uint8_t b);
uint8_t i2c_get_u08(uint8_t last);

uint8_t i2c_read(uint8_t addr, uint8_t* buf, uint8_t n);
uint8_t i2c_write(uint8_t addr, uint8_t* buf, uint8_t n);
