
/* Memory Map */
#define CONFIG 0x00
#define EN_AA 0x01
#define EN_RXADDR 0x02
#define SETUP_AW 0x03
#define SETUP_RETR 0x04
#define RF_CH 0x05
#define RF_SETUP 0x06
#define STATUS 0x07
#define OBSERVE_TX 0x08
#define CD 0x09
#define RX_ADDR_P0 0x0A
#define RX_ADDR_P1 0x0B
#define RX_ADDR_P2 0x0C
#define RX_ADDR_P3 0x0D
#define RX_ADDR_P4 0x0E
#define RX_ADDR_P5 0x0F
#define TX_ADDR 0x10
#define RX_PW_P0 0x11
#define RX_PW_P1 0x12
#define RX_PW_P2 0x13
#define RX_PW_P3 0x14
#define RX_PW_P4 0x15
#define RX_PW_P5 0x16
#define FIFO_STATUS 0x17
#define DYNPD 0x1C
#define FEATURE 0x1D

/* Bit Mnemonics */
#define MASK_RX_DR 6
#define MASK_TX_DS 5
#define MASK_MAX_RT 4
#define EN_CRC 3
#define CRCO 2
#define PWR_UP 1
#define PRIM_RX 0
#define ENAA_P5 5
#define ENAA_P4 4
#define ENAA_P3 3
#define ENAA_P2 2
#define ENAA_P1 1
#define ENAA_P0 0
#define ERX_P5 5
#define ERX_P4 4
#define ERX_P3 3
#define ERX_P2 2
#define ERX_P1 1
#define ERX_P0 0
#define AW 0
#define ARD 4
#define ARC 0
#define PLL_LOCK 4
#define RF_DR 3
#define RF_PWR 6
#define RX_DR 6
#define TX_DS 5
#define MAX_RT 4
#define RX_P_NO 1
#define TX_FULL 0
#define PLOS_CNT 4
#define ARC_CNT 0
#define TX_REUSE 6
#define FIFO_FULL 5
#define TX_EMPTY 4
#define RX_FULL 1
#define RX_EMPTY 0
#define DPL_P5 5
#define DPL_P4 4
#define DPL_P3 3
#define DPL_P2 2
#define DPL_P1 1
#define DPL_P0 0
#define EN_DPL 2
#define EN_ACK_PAY 1
#define EN_DYN_ACK 0

/* Instruction Mnemonics */
#define R_REGISTER 0x00
#define W_REGISTER 0x20
#define REGISTER_MASK 0x1F
#define ACTIVATE 0x50
#define R_RX_PL_WID 0x60
#define R_RX_PAYLOAD 0x61
#define W_TX_PAYLOAD 0xA0
#define W_ACK_PAYLOAD 0xA8
#define FLUSH_TX 0xE1
#define FLUSH_RX 0xE2
#define REUSE_TX_PL 0xE3
#define NOP 0xFF

/* Non-P omissions */
#define LNA_HCURR 0

/* P model memory Map */
#define RPD 0x09

/* P model bit Mnemonics */
#define RF_DR_LOW 5
#define RF_DR_HIGH 3
#define RF_PWR_LOW 1
#define RF_PWR_HIGH 2

#define CE_DDR DDRB
#define CE_PORT PORTB
#define CSN_DDR DDRB
#define CSN_PORT PORTB
#define CE_PIN (1 << 1)
#define CSN_PIN (1 << 2)



#define CONFIG_VAL ((1 << MASK_RX_DR) | (1 << MASK_TX_DS) | \
                    (1 << MASK_MAX_RT) | (1 << CRCO) | \
                    (1 << EN_CRC))


#include <avr/wdt.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

const int Radio_CSN = 10; // pinnenummer brukt for CSN på radio (ikke endre denne)
const int Radio_CE = 9;   //

// Watchdog functions. These are only safe with interrupts turned off.
void watchdogReset() {
  __asm__ __volatile__ (
    "wdr\n"
  );
}

#ifndef _SOFT_RESTART_H
#define _SOFT_RESTART_H


//Enter to bootloader state for OTA
#define soft_restart()         \
  \
  do                      \
    \
  {                       \
    wdt_enable(WDTO_15MS); \
    for (;;)               \
    {                      \
    }                      \
    \
  }                       \
  while (0)

#endif

//void(* resetFunc) (void) = 0; //declare reset function @ address 0 // only restart the code does not go into bootloader
char cmd[4];

void startRF(void)
{
  cmd[3] = 0;
  uint8_t address[6];
  Serial.println("our address:");
  address[0] = EEPROM.read(0);
  Serial.print(address[0]);
  address[1] = EEPROM.read(1);
  Serial.print(address[1]);
  address[2] = EEPROM.read(2);
  Serial.println(address[2]);

  Serial.println("burner address:");
  address[3] = EEPROM.read(3);
  Serial.print(address[3]);
  address[4] = EEPROM.read(4);
  Serial.print(address[4]);
  address[5] = EEPROM.read(5);
  Serial.println(address[5]);

  Mirf.csnPin = Radio_CSN;
  Mirf.cePin = Radio_CE;
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.configRegister(SETUP_RETR, 0x7f);

  /* Maximum Tx power, 250kbps data rate */
  Mirf.configRegister(RF_SETUP, (1 << RF_PWR_LOW) | (1 << RF_PWR_HIGH) | (1 << RF_DR_LOW));
  /* Dynamic payload length for TX & RX (pipes 0 and 1) */
  Mirf.configRegister(DYNPD, 0x03);
  Mirf.configRegister(FEATURE, 1 << EN_DPL);
  /* Reset status bits */
  Mirf.configRegister(STATUS, (1 << RX_DR) | (1 << TX_DS) | (1 << MAX_RT));
  /* Set some RF channel number */
  Mirf.configRegister(RF_CH, 42);
  /* 3-byte addresses */
  Mirf.configRegister(SETUP_AW, 0x01);
  /* Enable ACKing on both pipe 0 & 1 for TX & RX ACK support */
  Mirf.configRegister(EN_AA, 0x03);

  /*
    Configure reciving address.
  */
  Serial.print("address size: ");
  Serial.println(Mirf.adderSize);
  //Mirf.setRADDR("002"/*(byte *)address*/);
  Mirf.setRADDR((byte *)address);
  //Mirf.setTADDR((byte *)"000");//answer to ota station in bootloader mode? TBD
  Mirf.setTADDR((byte *)"001"); //answer to dispatch station on command? TBD
  /*
    Set the payload length to sizeof(unsigned long) the
    return type of millis().

    NB: payload on client and server must be the same.
  */
  Mirf.payload = 3;//sizeof(unsigned long);//3;//sizeof(cmd); //8;//sizeof(uint8_t);
  Serial.print("payload size: ");
  Serial.println(Mirf.payload);
  Mirf.channel = 42;
  Serial.print("channel: ");
  Serial.println(Mirf.channel);
  /* Write channel and payload config then power up reciver. */
  Mirf.config();
  /* Enable 16-bit CRC */
  //Mirf.configRegister(CONFIG, CONFIG_VAL | (1 << PWR_UP) | (1 << PRIM_RX));
  /* Only use data pipe 1 for receiving, pipe 0 is for TX ACKs */
  //Mirf.configRegister(EN_RXADDR, 0x02);
}

void checkIfOtaRequestOrLoadCommand(char* data)
{
  Mirf.getData(data);
  if (data[0] == 0xff)
  {
    while (1)
    {
      soft_restart();
    }
  }
  Serial.print("Data content: ");
  for (int i = 0; i < Mirf.payload; i++)
  {
    Serial.print(data[i]);
  }
  Serial.println("");
  Serial.println("==============");
  data[Mirf.payload] = 0;
}
