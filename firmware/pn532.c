#include "pn532.h"
#include "i2c.h"
#include <util/delay.h>
#include <string.h>
#define PN532_PACKBUFFSIZ 64
// IRQ input config
#define PN532_IRQ_PORT   PORTB
#define PN532_IRQ_PIN    PINB
#define PN532_IRQ_DDR    DDRB
#define PN532_IRQ        _BV(3)

void
pn532_init()
{
    // set up IRQ as input
    PN532_IRQ_DDR  &= ~PN532_IRQ;    // high -> input
    PN532_IRQ_PORT |=  PN532_IRQ;    // with pullup

    i2c_init();

#if 0
    // Reset the PN532
    digitalWrite(_reset, HIGH);
    digitalWrite(_reset, LOW);
    delay(400);
    digitalWrite(_reset, HIGH);
#endif
}


/**************************************************************************/
/*! 
    @brief  Sends a command and waits a specified period for the ACK

    @param  cmd       Pointer to the command buffer
    @param  cmdlen    The size of the command in bytes 
    @param  timeout   timeout before giving up
    
    @returns  1 if everything is OK, 0 if timeout occured before an
              ACK was recieved
*/
/**************************************************************************/
uint8_t
pn532_send_cmd_ack(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout)
{
    uint16_t timer = 0;
    // write the command
    pn532_send_cmd(cmd, cmdlen);
    // Wait for chip to say its ready
    while ((PN532_IRQ_PIN & PN532_IRQ)) {
        if (!timeout || (++timer > timeout)){
            return 0;
        }
        _delay_ms(1);
    }
    // read acknowledgement
    if (!pn532_read_ack()) {
        return 0;
    }
    return 1; // ack'd command
}

uint8_t
pn532_read_ack(void)
{

    uint8_t ackbuff[6];
    uint8_t pn532ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
    i2c_read(PN532_I2C_ADDRESS, ackbuff, 7);
    return (0 == strncmp((char *)ackbuff, (char *)pn532ack, 6));
}

/*!
    @brief  Writes a command to the PN532, automatically inserting the
            preamble and required frame details (checksum, len, etc.)

    @param  cmd       Pointer to the command buffer
    @param  cmdlen    Command length in bytes 
*/
void
pn532_send_cmd(uint8_t* cmd, uint8_t cmdlen)
{
    uint8_t checksum;
    uint8_t msg[16] = {0};
    uint8_t i = 0;
    cmdlen++;
    _delay_ms(2);

    // I2C START
    checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
    msg[i++] = PN532_PREAMBLE;
    msg[i++] = PN532_PREAMBLE;
    msg[i++] = PN532_STARTCODE2;

    msg[i++]= cmdlen;
    msg[i++]= ~cmdlen + 1;
    msg[i++]= PN532_HOSTTOPN532;

    checksum += PN532_HOSTTOPN532;

    uint8_t j;
    for (j=0; j<cmdlen-1; j++) {
        msg[i++]= cmd[j];
        checksum += cmd[j];
    }
    msg[i++] = ~checksum;
    msg[i++] = PN532_POSTAMBLE;

    i2c_write(PN532_I2C_ADDRESS,msg,i);
}

uint8_t
pn532_sam_config(void)
{
    uint8_t msg[8] = {0};
    msg[0] = PN532_COMMAND_SAMCONFIGURATION;
    msg[1] = 0x01; // normal mode;
    msg[2] = 0x14; // timeout 50ms * 20 = 1 second
    msg[3] = 0x01; // use IRQ pin!

    if (!pn532_send_cmd_ack(msg, 4, 2)){
        return 0;
    }

    uint8_t timeout = 100;
    uint8_t timer = 0;
    // Wait for chip to say its ready!
    while ((PN532_IRQ_PIN & PN532_IRQ)) {
        if (!timeout || (++timer > timeout)){
            return 0;
        }
        _delay_ms(1);
    }
    // read data packet
    i2c_read(PN532_I2C_ADDRESS, msg, 8);
    return  (msg[6] == 0x15);
}

/**************************************************************************/
/*!
    Waits for an ISO14443A target to enter the field
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t
pn532_request_passive_target_id()
{
    uint8_t msg[3] = {0};
    msg[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    msg[1] = 1;  // max 1 cards at once (we can set this to 2 later)
    msg[2] = PN532_MIFARE_ISO14443A;

    if (!pn532_send_cmd_ack(msg, 3, 2)){
        return 0;  // no connection to the PN532!?
    }
    return 1;
}

uint8_t
pn532_readout_passive_target_id(uint64_t * uid, uint8_t * uidLength)
{
    uint8_t msg[32] = {0};
    if(PN532_IRQ_PIN & PN532_IRQ) {
        return 0; //called at the wrong time
    }

    // read data packet
    i2c_read(PN532_I2C_ADDRESS, msg, 20);

    // check some basic stuff
    /* ISO14443A card response should be in the following format:

        byte            Description
        -------------   ------------------------------------------
        b0..6           Frame header and preamble
        b7              Tags Found
        b8              Tag Number (only one used in this example)
        b9..10          SENS_RES
        b11             SEL_RES
        b12             NFCID Length
        b13..NFCIDLen   NFCID
    */

    if (msg[7] != 1){
        return 0;
    }
    /* Card appears to be Mifare Classic */
    *uidLength = msg[12];
//    uint8_t i;
    memcpy(uid,&msg[13],msg[12]);
//    for (i=0; i < 8; i++) {
//        uid[i] = msg[13+i];
//    }
    return 1;
}






#if 0
/**************************************************************************/
/*!
    @brief  Checks the firmware version of the PN5xx chip

    @returns  The chip's firmware version and ID
*/
/**************************************************************************/
uint32_t
pn532_get_fw_version(void)
{
    uint8_t pn532response_firmwarevers[] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};
    uint8_t pn532_packetbuffer[PN532_PACKBUFFSIZ];
    uint32_t response;
    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

    if (!pn532_send_cmd_ack(pn532_packetbuffer, 1, 1000))
        return 0;

    _delay_ms(1);
    // read data packet
    i2c_read(PN532_I2C_ADDRESS, pn532_packetbuffer, 13);

    // check some basic stuff
    if (0 != strncmp((char *)pn532_packetbuffer, (char *)pn532response_firmwarevers, 6)) {
        return 0;
    }

    response = pn532_packetbuffer[7];
    response <<= 8;
    response |= pn532_packetbuffer[8];
    response <<= 8;
    response |= pn532_packetbuffer[9];
    response <<= 8;
    response |= pn532_packetbuffer[10];
    return response;
}
#endif
