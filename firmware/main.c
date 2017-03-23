/* Name: main.c
 */

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <string.h>

#include "usbdrv.h"
#include "oddebug.h"

#include "i2c.h"
#include "pn532.h"

/*
Pin assignment:
PB0, PB2 = USB data lines
*/


#ifndef NULL
#define NULL    ((void *)0)
#endif

static uint8_t    reportBuffer[2];    /* buffer for HID reports */
static uint8_t    idleRate;           /* in 4 ms units */

static uint8_t    valueBuffer[32];
static uint8_t    *nextDigit;

static uint8_t  request_needed = 1;

const PROGMEM char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor */
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};
/* We use a simplifed keyboard report descriptor which does not support the
 * boot protocol. We don't allow setting status LEDs and we only allow one
 * simultaneous key press (except modifiers). We can therefore use short
 * 2 byte input reports.
 * The report descriptor has been created with usb.org's "HID Descriptor Tool"
 * which can be downloaded from http://www.usb.org/developers/hidpage/.
 * Redundant entries (such as LOGICAL_MINIMUM and USAGE_PAGE) have been omitted
 * for the second INPUT item.
 */

#define KEY_0       39
#define KEY_1       30
#define KEY_RETURN  40


static void
buildReport(uint8_t key)
{
    reportBuffer[0] = 0;    /* no modifiers */
    reportBuffer[1] = key;
}

uint8_t
usbFunctionSetup(uint8_t data[8])
{
    usbRequest_t    *rq = (void *)data;
    usbMsgPtr = reportBuffer;

    if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
        if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
            /* we only have one report type, so don't look at wValue */
            return sizeof(reportBuffer);
        }else if(rq->bRequest == USBRQ_HID_GET_IDLE){
            usbMsgPtr = &idleRate;
            return 1;
        }else if(rq->bRequest == USBRQ_HID_SET_IDLE){
            idleRate = rq->wValue.bytes[1];
        }
    }else{
        /* no vendor specific requests implemented */
    }
    return 0;
}

/* Calibrate the RC oscillator to 8.25 MHz. The core clock of 16.5 MHz is
 * derived from the 66 MHz peripheral clock by dividing. Our timing reference
 * is the Start Of Frame signal (a single SE0 bit) available immediately after
 * a USB RESET. We first do a binary search for the OSCCAL value and then
 * optimize this value with a neighboorhod search.
 * This algorithm may also be used to calibrate the RC oscillator directly to
 * 12 MHz (no PLL involved, can therefore be used on almost ALL AVRs), but this
 * is wide outside the spec for the OSCCAL value and the required precision for
 * the 12 MHz clock! Use the RC oscillator calibrated to 12 MHz for
 * experimental purposes only!
 */
static void
calibrateOscillator(void)
{
    uint8_t       step = 128;
    uint8_t       trialValue = 0, optimumValue;
    int         x, optimumDev, targetValue = (unsigned)(1499 * (double)F_CPU / 10.5e6 + 0.5);
    /* do a binary search: */
    do {
        OSCCAL = trialValue + step;
        x = usbMeasureFrameLength();    /* proportional to current real frequency */
        if(x < targetValue)             /* frequency still too low */
            trialValue += step;
        step >>= 1;
    } while(step > 0);
    /* We have a precision of +/- 1 for optimum OSCCAL here */
    /* now do a neighborhood search for optimum value */
    optimumValue = trialValue;
    optimumDev = x; /* this is certainly far away from optimum */
    for(OSCCAL = trialValue - 1; OSCCAL <= trialValue + 1; OSCCAL++){
        x = usbMeasureFrameLength() - targetValue;
        if(x < 0)
            x = -x;
        if(x < optimumDev){
            optimumDev = x;
            optimumValue = OSCCAL;
        }
    }
    OSCCAL = optimumValue;
}

/*
Note: This calibration algorithm may try OSCCAL values of up to 192 even if
the optimum value is far below 192. It may therefore exceed the allowed clock
frequency of the CPU in low voltage designs!
You may replace this search algorithm with any other algorithm you like if
you have additional constraints such as a maximum CPU clock.
For version 5.x RC oscillators (those with a split range of 2x128 steps, e.g.
ATTiny25, ATTiny45, ATTiny85), it may be useful to search for the optimum in
both regions.
*/
void
usbEventResetReady(void)
{
    /* Disable interrupts during oscillator calibration since
     * usbMeasureFrameLength() counts CPU cycles.
     */
    cli();
    calibrateOscillator();
    sei();
    eeprom_write_byte(0, OSCCAL);   /* store the calibrated value in EEPROM */
}

static void
fillMsgBuffer(uint64_t value)
{
    uint64_t   digit;
    nextDigit = &valueBuffer[sizeof(valueBuffer)];
    *--nextDigit = 0xff;/* terminate with 0xff */
    *--nextDigit = 0;
    *--nextDigit = KEY_RETURN;
    do {
        digit = value % 10;
        value /= 10;
        *--nextDigit = 0;
        if(digit == 0){
            *--nextDigit = KEY_0;
        }else{
            *--nextDigit = KEY_1 - 1 + digit;
        }
    } while(value > 0);
}

static void
timer_tick(void)
{
    static uint8_t timer_cnt;
    if(TIFR & (1 << TOV1)) {
        TIFR = (1 << TOV1); /* clear overflow */
        if(++timer_cnt >= 126){       /* ~ 2 second interval */
            timer_cnt = 0;
            if(request_needed){
                request_needed = 0;
                pn532_request_passive_target_id();
            }
        }
    }
}

#define PN532_IRQ_PORT   PORTB
#define PN532_IRQ_PIN    PINB
#define PN532_IRQ_DDR    DDRB
#define PN532_IRQ        _BV(3)

int
main(void)
{
    uint8_t   i;
    uint8_t   calibrationValue;

    calibrationValue = eeprom_read_byte(0); /* calibration value from last time */
    //calibrationValue = 0xff;
    if(calibrationValue != 0xff){
        OSCCAL = calibrationValue;
    }
    //init i2c and PN532
    pn532_init();
    for(i=0;i<20;i++){  /* 400 ms */
        _delay_ms(20);
    }
    //fwver = pn532_get_fw_version();
    pn532_sam_config();
    pn532_request_passive_target_id();

    //init the USB stack
    odDebugInit();
    usbDeviceDisconnect();
    for(i=0;i<20;i++){  /* 300 ms disconnect */
        _delay_ms(15);
    }
    usbDeviceConnect();
    wdt_enable(WDTO_2S);
    usbInit();

    //setting up timer which used to not re-read the RFID card right
    //away and to have break between keys
    TCCR1 = 0x0b;           /* select clock: 16.5M/1k -> overflow rate = 16.5M/256k = 62.94 Hz */
    sei();
    uint8_t skip = 3;
    for(;;){    /* main event loop */
        wdt_reset();
        usbPoll();
        if(usbInterruptIsReady() && nextDigit != NULL && (skip-- > 0)){ /* we can send another key */
//        if(usbInterruptIsReady() && nextDigit != NULL){ /* we can send another key */
            skip = 3;
            buildReport(*nextDigit);
            usbSetInterrupt(reportBuffer, sizeof(reportBuffer));
            if(*++nextDigit == 0xff){ /* this was terminator character */
                nextDigit = NULL;
            }
        }
        else if ((!(PN532_IRQ_PIN & PN532_IRQ)) && (nextDigit == NULL)){ //we got a card IRQ while we don't work on one
            uint64_t uid = 0; // Buffer to store the returned UID
            uint8_t uid_len = 0;      // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
            if(!pn532_readout_passive_target_id(&uid,&uid_len)){
                continue;
            }
            fillMsgBuffer(uid);
            TCNT1 = 0;
            request_needed = 1;
        }

        timer_tick();
    }
}
