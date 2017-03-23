#include "i2c.h"

#include <util/delay.h>

#define ENABLE_SCL_EXPAND
#define DEFAULT_DELAY 10  // default 10us (100khz)
static unsigned short clock_delay  = DEFAULT_DELAY;
static unsigned short clock_delay2 = DEFAULT_DELAY/2;
static unsigned short expected;

void
i2c_io_set_sda(uint8_t hi)
{
    if(hi) {
        I2C_DDR  &= ~I2C_SDA;    // high -> input
        I2C_PORT |=  I2C_SDA;    // with pullup
    } else {
        I2C_DDR  |=  I2C_SDA;    // low -> output
        I2C_PORT &= ~I2C_SDA;    // drive low
    }
}

uint8_t
i2c_io_get_sda(void)
{
    return(I2C_PIN & I2C_SDA);
}

void
i2c_io_set_scl(uint8_t hi)
{
#ifdef ENABLE_SCL_EXPAND
  _delay_loop_2(clock_delay2);
  if(hi) {
    I2C_DDR &= ~I2C_SCL;          // port is input
    I2C_PORT |= I2C_SCL;          // enable pullup

    // wait while pin is pulled low by client
    while(!(I2C_PIN & I2C_SCL));
  } else {
    I2C_DDR |= I2C_SCL;           // port is output
    I2C_PORT &= ~I2C_SCL;         // drive it low
  }
  _delay_loop_2(clock_delay);
#else
  _delay_loop_2(clock_delay2);
  if(hi) I2C_PORT |=  I2C_SCL;    // port is high
  else   I2C_PORT &= ~I2C_SCL;    // port is low
  _delay_loop_2(clock_delay);
#endif
}

void
i2c_init(void)
{
    /* init the sda/scl pins */
    I2C_DDR &= ~I2C_SDA;            // port is input
    I2C_PORT |= I2C_SDA;            // enable pullup
#ifdef ENABLE_SCL_EXPAND
    I2C_DDR &= ~I2C_SCL;            // port is input
    I2C_PORT |= I2C_SCL;            // enable pullup
#else
    I2C_DDR |= I2C_SCL;             // port is output
#endif
    /* no bytes to be expected */
    expected = 0;
}

/* clock HI, delay, then LO */
void
i2c_scl_toggle(void)
{
    i2c_io_set_scl(1);
    i2c_io_set_scl(0);
}

/* i2c start condition */
void
i2c_start(void)
{
    i2c_io_set_sda(0);
    i2c_io_set_scl(0);
}

/* i2c repeated start condition */
void
i2c_repstart(void)
{
    /* scl, sda may not be high */
    i2c_io_set_sda(1);
    i2c_io_set_scl(1);

    i2c_io_set_sda(0);
    i2c_io_set_scl(0);
}

/* i2c stop condition */
void
i2c_stop(void)
{
    i2c_io_set_sda(0);
    i2c_io_set_scl(1);
    i2c_io_set_sda(1);
}

uint8_t
i2c_put_u08(uint8_t b)
{
    char i;
    for (i=7;i>=0;i--) {
        if ( b & (1<<i) ){
            i2c_io_set_sda(1);
        }
        else {
            i2c_io_set_sda(0);
        }
        i2c_scl_toggle();           // clock HI, delay, then LO
    }

    i2c_io_set_sda(1);            // leave SDL HI
    i2c_io_set_scl(1);            // clock back up

    b = i2c_io_get_sda();         // get the ACK bit
    i2c_io_set_scl(0);            // not really ??

    return(b == 0);               // return ACK value
}

uint8_t
i2c_get_u08(uint8_t last)
{
    char i;
    uint8_t c,b = 0;

    i2c_io_set_sda(1);            // make sure pullups are activated
    i2c_io_set_scl(0);            // clock LOW

    for(i=7;i>=0;i--) {
        i2c_io_set_scl(1);          // clock HI
        c = i2c_io_get_sda();
        b <<= 1;
        if(c) b |= 1;
        i2c_io_set_scl(0);          // clock LO
    }

    if(last) i2c_io_set_sda(1);   // set NAK
    else     i2c_io_set_sda(0);   // set ACK

    i2c_scl_toggle();             // clock pulse
    i2c_io_set_sda(1);            // leave with SDL HI

    return b;                     // return received byte
}


uint8_t
i2c_read(uint8_t addr, uint8_t* buf, uint8_t n)
{
    //set the R/W bit to read
    addr |= 1;
    i2c_start();
    if(!i2c_put_u08(addr)) {
        i2c_stop();
        return 0;
    }
    //TODO why we need this? I don't know, according to LA
    //there is a 0000001 sent by the device before the ACK...
    i2c_get_u08(0);

    uint8_t i;
    for (i=0; i<n; i++) {
        buf[i] = i2c_get_u08(i+2>n);
    }
    i2c_stop();
    return n;
}

uint8_t
i2c_write(uint8_t addr, uint8_t* buf, uint8_t n)
{
    //set the R/W bit to write
//    addr &= 0xfe;
    i2c_start();
    if(!i2c_put_u08(addr)) {
        i2c_stop();
        return 0;
    }
    uint8_t i;
    for (i=0; i<n; i++) {
        if(!i2c_put_u08(buf[i])) {
            i2c_stop();
            return i+1;
        }
    }
    i2c_stop();
    return n;
}
