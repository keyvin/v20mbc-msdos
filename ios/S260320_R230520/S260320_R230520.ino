/* ------------------------------------------------------------------------------

S260320-R230520 - HW ref: A250220 (V20-MBC)

IOS - I/O Subsystem for the V20-MBC (V20HL CPU - 128/512/1024kB RAM - 4/8MHz)



Notes:

1: Supported CMOS CPU: uPD70108H (V20HL), 80C88

3: Supported reset at boot time for uTerm (A071218-R250119) 

2: Tested on Atmega32A @ Arduino IDE 1.8.12

3: Embedded FW: S200220 (iLoad)

4: Utilities:   S230220 (iLoad-80)
                S170420 (Switch-80)
                S160420 (Hex formatting utility)





IMPORTANT NOTE:

The "Virtual I/O engine" is tuned to work at CPU clock frequencies not less than 4MHz. 
Do  not attempt to use a CPU clock less than 4MHz.
To use a CPU clock less than 4MHz the "Virtual I/O engine" requires a new tune-up.



---------------------------------------------------------------------------------



CHANGELOG:


S260320           First revision.
S260320-R180520   Fixed a bug preventig a running program to read the RTC as expected;
                  Added support for CP/M 2.2 (8080).
S260320-R210520   Added extended serial Rx buffer check for XMODEM support.
S260320-R230520   Added support for CP/M-86.


---------------------------------------------------------------------------------

Credits:

SD library from: https://github.com/greiman/PetitFS (based on 
PetitFS: http://elm-chan.org/fsw/ff/00index_p.html)

PetitFS licence:
/-----------------------------------------------------------------------------/
/  Petit FatFs - FAT file system module  R0.03                  (C)ChaN, 2014
/-----------------------------------------------------------------------------/
/ Petit FatFs module is a generic FAT file system module for small embedded
/ systems. This is a free software that opened for education, research and
/ commercial developments under license policy of following trems.
/
/  Copyright (C) 2014, ChaN, all right reserved.
/
/ * The Petit FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-----------------------------------------------------------------------------/


--------------------------------------------------------------------------------- */


// ------------------------------------------------------------------------------
//
// Hardware definitions for A250220 (see A250220 schematic) - Base system 
//
// ------------------------------------------------------------------------------

// Data bus
#define   D0            27    // PA3 pin 37
#define   D1            25    // PA1 pin 39 
#define   D2            26    // PA2 pin 38 
#define   D3            24    // PA0 pin 40
#define   D4            28    // PA4 pin 36  
#define   D5            29    // PA5 pin 35
#define   D6            30    // PA6 pin 34   
#define   D7            31    // PA7 pin 33   

// Address bus (only A0 and A1 used)
#define   AD0           21    // PC5 pin 27            
#define   AD1           22    // PC6 pin 28 

// Control lines
#define   CLK           15    // PD7 pin 21            
#define   RES           2     // PB2 pin 3
#define   INTA_         23    // PC7 pin 29
#define   INTR          3     // PB3 pin 4
#define   RAMEN_        14    // PD6 pin 20
#define   HOLDRES_      1     // PB1 pin 2
#define   READY         10    // PD2 pin 16
#define   RDYRES_       0     // PB0 pin 1
#define   IOM_          20    // PC4 pin 26
#define   WR_           19    // PC3 pin 25
#define   RD_           18    // PC2 pin 24
#define   MCU_RTS_      11    // PD3 pin 17   * RESERVED - NOT USED *
#define   MCU_CTS_      12    // PD4 pin 18   Used only to reset uTerm at boot time
     // USES RXD1
// Led and key
#define   USER          13    // PD5 pin 19   Led USER and key (led USER is ON if LOW)
#define   LED_IOS       0     // PB0 pin 1    NOTE: it shares the same pin of RDYRES_

// Definitions for fast port access
#define   PIN_READY         PIND & B00000100                // Read READY
#define   PIN_WR_           PINC & B00001000                // Read WR_
#define   PIN_RD_           PINC & B00000100                // Read RD_
#define   PIN_A1A0          (PINC >> 5 ) & B00000011        // Read A1-A0
#define   POUT_0_INTR       PORTB = PORTB & B11110111       // Set INTR to 0
#define   POUT_0_RDYRES_    PORTB = PORTB & B11111110       // Set RDYRES_ to 0
#define   POUT_1_RDYRES_    PORTB = PORTB | B00000001       // Set RDYRES_ to 1
#define   POUT_0_HOLDRES_   PORTB = PORTB & B11111101       // Set HOLDRES_ to 0
#define   POUT_1_HOLRES_    PORTB = PORTB | B00000010       // Set HOLRES_ to 1

// ------------------------------------------------------------------------------
//
// Hardware definitions for A250220 GPE Option (see MCP23017 datasheet)
//
// ------------------------------------------------------------------------------

#define   GPIOEXP_ADDR  0x20  // I2C module address
#define   IODIRA_REG    0x00  // MCP23017 internal register IODIRA
#define   IODIRB_REG    0x01  // MCP23017 internal register IODIRB
#define   GPPUA_REG     0x0C  // MCP23017 internal register GPPUA
#define   GPPUB_REG     0x0D  // MCP23017 internal register GPPUB
#define   GPIOA_REG     0x12  // MCP23017 internal register GPIOA
#define   GPIOB_REG     0x13  // MCP23017 internal register GPIOB

// ------------------------------------------------------------------------------
//
// Hardware definitions for A250220 RTC Module Option (see DS3231 datasheet)
//
// ------------------------------------------------------------------------------

#define   DS3231_RTC    0x68  // DS3231 I2C address
#define   DS3231_SECRG  0x00  // DS3231 Seconds Register
#define   DS3231_STATRG 0x0F  // DS3231 Status Register

// ------------------------------------------------------------------------------
//
// S170420 - Switch-80 utility definitions
//
// ------------------------------------------------------------------------------

#define   SWCH80FN          "SWITCH80.BIN"      // Switch-80 executable binary file
#define   SWCH80STRADDR     0x0000              // Starting address for the SWITCH80.BIN file
#define   SWITCH80_STRADDR  0x00C1              // Data area. Must match exactly with the S170420 executable
#define   SWITCH80_SIZE     0x00C3              // Data area. Must match exactly with the S170420 executable

// ------------------------------------------------------------------------------
//
// Others file names and starting addresses
//
// ------------------------------------------------------------------------------

#define   ILOAD80FN         "ILOAD80.BIN"       // iLoad-80 executable binary file
#define   AUTOFN            "AUTOBOOT.BIN"      // User executable binary file
#define   AUTO80FN          "AU80BOOT.BIN"      // User executable binary file (8080 machine code)
#define   CPMFN             "CPM22.BIN"         // CP/M 2.2 (8080)
#define   CPM86FN           "CPM86.BIN"         // It is the CPM.SYS file without the first 128 bytes (header)
#define   V20DISK           "DSxNyy.DSK"        // Generic V20 disk name (from DS0N00.DSK to DS9N99.DSK)
#define   DS_OSNAME         "DSxNAM.DAT"        // File with the OS name for Disk Set "x" (from DS0NAM.DAT to DS9NAM.DAT)
#define   SEG_8080CODE      0x0FF0              // Segment used for 8080 code
#define   ILOAD80ADDR       0x0000              // Starting address for ILD80.BIN
#define   AUTSTRADDR        0x0000              // Starting address for the AUTOBOOT.BIN file
#define   AUTO80STRADDR     0x0000              // Starting address for the AU80BOOT.BIN file
#define   CPM22CBASE        0xD200              // CBASE value for CP/M 2.2
#define   CPMSTRADDR        (CPM22CBASE - 32)   // Starting address for CP/M 2.2
#define   CPM86STRADDR      0x0400              // Load Starting address for CP/M-86
#define   CPM86JMPADDR      0x2500              // Address (offset) of CP/M-86 for the first instr. to execute
#define   CPM86JMPSEG       0x0040              // Segment of CP/M-86 for the first instr. to execute

// ------------------------------------------------------------------------------
//
// V20 CPU opcodes definitions
//
// ------------------------------------------------------------------------------

#define   HLT_OPCODE            0xF4
#define   MOV_AX_IMM_OPCODE     0xB8
#define   MOV_DS_AX_OPCODE      0x8ED8
#define   MOV_MEM_IMM_OPCODE    0xC606
#define   JMPF_OPCODE           0xEA

// ------------------------------------------------------------------------------
//
// Others definitions
//
// ------------------------------------------------------------------------------
#define   MAX_RD_CLKPULSES      12                  // Max clock pulses before finding a Read Bus Cycle

// ------------------------------------------------------------------------------
//
//  Inline functions
//
// ------------------------------------------------------------------------------

inline void execWriteOpcode() __attribute__((always_inline));
inline void execReadOpcode() __attribute__((always_inline));

// ------------------------------------------------------------------------------
//
//  Libraries
//
// ------------------------------------------------------------------------------

#include <avr/pgmspace.h>                 // Needed for PROGMEM
#include "Wire.h"                         // Needed for I2C bus
#include <EEPROM.h>                       // Needed for internal EEPROM R/W
#include "PetitFS.h"                      // Light handler for FAT16 and FAT32 filesystems on SD

// ------------------------------------------------------------------------------
//
// Atmega clock speed check
//
// ------------------------------------------------------------------------------

#if F_CPU == 20000000
  #define CLOCK_LOW   "5"
  #define CLOCK_HIGH  "10"
#elif F_CPU == 24000000
  #define CLOCK_LOW "6"
  #define CLOCK_HIGH "12"
#else
  #define CLOCK_LOW   "4"
  #define CLOCK_HIGH  "8"
#endif

// ------------------------------------------------------------------------------
//
//  Constants
//
// ------------------------------------------------------------------------------

const String  compTimeStr  = __TIME__;    // Compile timestamp string
const String  compDateStr  = __DATE__;    // Compile datestamp string
const byte    debug        = 0;           // Debug off = 0, low = 1, high = 2
const byte    daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const byte    maxDiskSet   = 2;           // Number of configured Disk Sets
const byte    bootModeAddr = 10;          // Internal EEPROM address for boot mode storage
const byte    autoexecFlagAddr  = 12;     // Internal EEPROM address for AUTOEXEC flag storage
const byte    clockModeAddr     = 13;     // Internal EEPROM address for the V20 clock high/low speed switch
                                          //  (1 = low speed, 0 = high speed)
const byte    diskSetAddr  = 14;          // Internal EEPROM address for the current Disk Set [0..9]
const byte    maxDiskNum   = 99;          // Max number of virtual disks

// V20 binary images into flash and related constants
const word  boot_A_StrAddr = 0xFCF0;      // Payload A image starting address
const byte  boot_A_[] PROGMEM = {         // Payload A image (S200220 iLoad)
  0xEB, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0xBC, 0x10, 0xFD, 0x8C, 0xC8, 0x8E, 0xD8, 0x8E, 0xD0, 0xBB, 0x65, 0xFD, 0xE8, 0x21, 0x02, 0xE8, 
  0x61, 0x02, 0xE8, 0xE2, 0x00, 0xB0, 0xFF, 0x38, 0xF8, 0x75, 0x11, 0x38, 0xD8, 0x75, 0x0D, 0xBB, 
  0xEC, 0xFD, 0xE8, 0x0B, 0x02, 0xBB, 0x9B, 0xFD, 0xE8, 0x05, 0x02, 0xF4, 0x53, 0xBB, 0xEC, 0xFD, 
  0xE8, 0xFD, 0x01, 0xBB, 0x88, 0xFD, 0xE8, 0xF7, 0x01, 0x8C, 0xCB, 0xE8, 0x03, 0x02, 0xB0, 0x3A, 
  0xE8, 0x3D, 0x02, 0x5B, 0xE8, 0xFA, 0x01, 0xE8, 0x29, 0x02, 0xE8, 0x26, 0x02, 0xE4, 0x01, 0x3C, 
  0xFF, 0x75, 0xFA, 0xFF, 0xE3, 0x69, 0x4C, 0x6F, 0x61, 0x64, 0x20, 0x2D, 0x20, 0x49, 0x6E, 0x74, 
  0x65, 0x6C, 0x2D, 0x48, 0x65, 0x78, 0x20, 0x4C, 0x6F, 0x61, 0x64, 0x65, 0x72, 0x20, 0x2D, 0x20, 
  0x53, 0x32, 0x30, 0x30, 0x32, 0x32, 0x30, 0x00, 0x53, 0x74, 0x61, 0x72, 0x74, 0x69, 0x6E, 0x67, 
  0x20, 0x41, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x3A, 0x20, 0x00, 0x4C, 0x6F, 0x61, 0x64, 0x20, 
  0x65, 0x72, 0x72, 0x6F, 0x72, 0x20, 0x2D, 0x20, 0x53, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x20, 0x68, 
  0x61, 0x6C, 0x74, 0x65, 0x64, 0x00, 0x57, 0x61, 0x69, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x69, 0x6E, 
  0x70, 0x75, 0x74, 0x20, 0x73, 0x74, 0x72, 0x65, 0x61, 0x6D, 0x2E, 0x2E, 0x2E, 0x00, 0x53, 0x79, 
  0x6E, 0x74, 0x61, 0x78, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0x21, 0x00, 0x43, 0x68, 0x65, 0x63, 
  0x6B, 0x73, 0x75, 0x6D, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72, 0x21, 0x00, 0x69, 0x4C, 0x6F, 0x61, 
  0x64, 0x3A, 0x20, 0x00, 0x41, 0x64, 0x64, 0x72, 0x65, 0x73, 0x73, 0x20, 0x76, 0x69, 0x6F, 0x6C, 
  0x61, 0x74, 0x69, 0x6F, 0x6E, 0x21, 0x00, 0x50, 0x52, 0x51, 0xB9, 0xFF, 0xFF, 0xBB, 0xB6, 0xFD, 
  0xE8, 0x2D, 0x01, 0xE8, 0x6D, 0x01, 0xE8, 0x82, 0x01, 0x3C, 0x0D, 0x74, 0xF9, 0x3C, 0x0A, 0x74, 
  0xF5, 0x3C, 0x20, 0x74, 0xF1, 0xE8, 0x00, 0x01, 0xE8, 0x65, 0x01, 0x3C, 0x3A, 0x75, 0x4D, 0xE8, 
  0xBB, 0x00, 0x88, 0xC6, 0xB2, 0x00, 0xE8, 0xA4, 0x00, 0xE8, 0xA4, 0x00, 0xB0, 0xFF, 0x38, 0xE8, 
  0x75, 0x06, 0x38, 0xC8, 0x75, 0x02, 0x89, 0xD9, 0x88, 0xF8, 0xE8, 0x90, 0x00, 0x88, 0xD8, 0xE8, 
  0x8B, 0x00, 0xE8, 0x98, 0x00, 0xE8, 0x85, 0x00, 0x3C, 0x01, 0x75, 0x32, 0xE8, 0x8E, 0x00, 0xE8, 
  0x7B, 0x00, 0x88, 0xD0, 0x20, 0xC0, 0x74, 0x6C, 0xE8, 0x18, 0x01, 0xBB, 0xEC, 0xFD, 0xE8, 0xCF, 
  0x00, 0xBB, 0xDC, 0xFD, 0xE8, 0xC9, 0x00, 0xB9, 0xFF, 0xFF, 0xEB, 0x58, 0xE8, 0x04, 0x01, 0xBB, 
  0xEC, 0xFD, 0xE8, 0xBB, 0x00, 0xBB, 0xCE, 0xFD, 0xE8, 0xB5, 0x00, 0xB9, 0xFF, 0xFF, 0x88, 0xF0, 
  0x20, 0xC0, 0x74, 0x2E, 0xE8, 0x56, 0x00, 0xE8, 0x43, 0x00, 0x53, 0x51, 0x20, 0xC0, 0xB9, 0xF0, 
  0xFC, 0x29, 0xCB, 0x59, 0x5B, 0x72, 0x14, 0xE8, 0xD9, 0x00, 0xBB, 0xEC, 0xFD, 0xE8, 0x90, 0x00, 
  0xBB, 0xF4, 0xFD, 0xE8, 0x8A, 0x00, 0xB9, 0xFF, 0xFF, 0xEB, 0x19, 0x88, 0x07, 0x43, 0xFE, 0xCE, 
  0xEB, 0xCC, 0xE8, 0x28, 0x00, 0xE8, 0x15, 0x00, 0x88, 0xD0, 0x20, 0xC0, 0x75, 0x9A, 0xE8, 0xB2, 
  0x00, 0xE9, 0x42, 0xFF, 0xE8, 0xAC, 0x00, 0x89, 0xCB, 0x59, 0x5A, 0x58, 0xC3, 0x28, 0xC2, 0xC3, 
  0x50, 0xE8, 0x09, 0x00, 0x88, 0xC7, 0xE8, 0x04, 0x00, 0x88, 0xC3, 0x58, 0xC3, 0x51, 0xE8, 0x0D, 
  0x00, 0xB1, 0x04, 0xD2, 0xC0, 0x88, 0xC4, 0xE8, 0x04, 0x00, 0x08, 0xE0, 0x59, 0xC3, 0xE8, 0x9A, 
  0x00, 0xE8, 0x24, 0x00, 0xE8, 0x09, 0x00, 0x73, 0xF5, 0xE8, 0x29, 0x00, 0xE8, 0x64, 0x00, 0xC3, 
  0x3C, 0x47, 0x72, 0x01, 0xC3, 0x3C, 0x30, 0x73, 0x02, 0xF8, 0xC3, 0x3C, 0x3A, 0x73, 0x01, 0xC3, 
  0x3C, 0x41, 0x73, 0x02, 0xF8, 0xC3, 0xF9, 0xC3, 0x3C, 0x61, 0x73, 0x01, 0xC3, 0x3C, 0x7B, 0x72, 
  0x01, 0xC3, 0x24, 0x5F, 0xC3, 0x3C, 0x3A, 0x72, 0x02, 0x2C, 0x07, 0x2C, 0x30, 0x24, 0x0F, 0xC3, 
  0x50, 0x53, 0x8A, 0x07, 0x3C, 0x00, 0x74, 0x06, 0xE8, 0x45, 0x00, 0x43, 0xEB, 0xF4, 0x5B, 0x58, 
  0xC3, 0x53, 0x50, 0x88, 0xF8, 0xE8, 0x08, 0x00, 0x88, 0xD8, 0xE8, 0x03, 0x00, 0x58, 0x5B, 0xC3, 
  0x50, 0x51, 0x88, 0xC4, 0xB1, 0x04, 0xD2, 0xC8, 0xE8, 0x08, 0x00, 0x88, 0xE0, 0xE8, 0x03, 0x00, 
  0x59, 0x58, 0xC3, 0x50, 0x24, 0x0F, 0x04, 0x30, 0x3C, 0x3A, 0x72, 0x02, 0x04, 0x07, 0xE8, 0x0F, 
  0x00, 0x58, 0xC3, 0x50, 0xB0, 0x0D, 0xE8, 0x07, 0x00, 0xB0, 0x0A, 0xE8, 0x02, 0x00, 0x58, 0xC3, 
  0x88, 0xC4, 0xB0, 0x01, 0xE6, 0x01, 0x88, 0xE0, 0xE6, 0x00, 0xC3, 0xE4, 0x01, 0x3C, 0xFF, 0x74, 
  0xFA, 0xC3
  };

const byte * const flahBootTable[1] PROGMEM = {boot_A_}; // Payload pointers table (flash)


// ------------------------------------------------------------------------------
//
//  Global variables
//
// ------------------------------------------------------------------------------

// General purpose variables
byte          ioAddress;                  // Virtual I/O address. Only four possible addresses are valid [0x00..0x03]
byte          ioData;                     // Data byte used for the I/O operation
unsigned long timeStamp;                  // Timestamp for led blinking
char          inChar;                     // Input char from serial
byte          tempByte;                   // Temporary variable (buffer)
byte          haltFlag = 0;               // Set to 1 if a CPU halt state is detected
byte          ioOpcode       = 0xFF;      // I/O operation code or Opcode (0xFF means "No Operation")
word          ioByteCnt;                  // Exchanged bytes counter during an I/O operation
byte          V20IntEnFlag = 0;           // INT enable on serial Rx flag
byte          iCount;                     // Temporary variable (counter)
byte          clockMode;                  // V20 clock HI/LO speed selector (0 = 8/10MHz, 1 = 4/5MHz)
byte          moduleGPIO     = 0;         // Set to 1 if the module is found, 0 otherwise
byte          bootMode       = 0;         // Set the program to boot (from flash or SD)
byte *        BootImage;                  // Pointer to selected flash payload array (image) to boot
word          BootImageSize  = 0;         // Size of the selected flash payload array (image) to boot
word          BootStrAddr;                // Starting address of the selected program to boot (from flash or SD)

// DS3231 RTC variables
byte          foundRTC;                   // Set to 1 if RTC is found, 0 otherwise
byte          seconds, minutes, hours, day, month, year;
byte          tempC;  

// SD disk and CP/M support variables
FATFS         filesysSD;                  // Filesystem object (PetitFS library)
byte          bufferSD[32];               // I/O buffer for SD disk operations (store a "segment" of a SD sector).
                                          //  Each SD sector (512 bytes) is divided into 16 segments (32 bytes each)
const char *  fileNameSD;                 // Pointer to the string with the currently used file name
byte          autobootFlag;               // Set to 1 if "autoboot.bin" must be executed at boot, 0 otherwise
byte          autoexecFlag;               // Set to 1 if AUTOEXEC must be executed at CP/M cold boot, 0 otherwise
byte          errCodeSD;                  // Temporary variable to store error codes from the PetitFS
byte          numReadBytes;               // Number of read bytes after a readSD() call

// Disk emulation on SD
char          diskName[11]    = V20DISK;  // String used for virtual disk file name
char          OsName[11]      = DS_OSNAME;// String used for file holding the OS name
word          trackSel;                   // Store the current track number [0..5
byte          sectSel;                    // Store the current sector number [0..31]
byte          diskErr         = 19;       // SELDISK, SELSECT, SELTRACK, WRITESECT, READSECT or SDMOUNT resulting 
                                          //  error code
byte          numWriBytes;                // Number of written bytes after a writeSD() call
byte          diskSet;                    // Current "Disk Set"

byte          timer_event = 0;           //time tic occured
byte          timer_interrupt_enabled;             //interval timer enabled?

// ------------------------------------------------------------------------------

void setup() 
{

// ------------------------------------------------------------------------------
//
//  Local variables
//
// ------------------------------------------------------------------------------

  word    loadAddress;                    // SRAM current address (for boot program load);
  word    loadSegment = 0x0000;           // Segment used for boot program load
  word    V20initJumpAddr;                // Offeset for the "far jump" after the V20 reset
  word    V20initJumpSegm = 0x0000;       // Segment for the "far jump" after the V20 reset
  word    loadedBinSize;                  // Size in bytes of the loaded code
  word    FirstStageStrAddr;              // Starting address of the loaded code at the first stage
  word    JumpStrAddr = 0;                // Starting execution address of the selected program to boot
                                          // (if different from the load starting address) 
  byte    bootSelection = 0;              // Flag to enter into the boot mode selection [0..1]
  byte    autoboot80    = 0;              // Flag to activate the dual-stage boot mode for 8080 binaries [0..1]
  char    minBootChar   = '1';            // Minimum allowed ASCII value selection (boot selection)
  char    maxSelChar    = '8';            // Maximum allowed ASCII value selection (boot selection)
  byte    maxBootMode   = 4;              // Default maximum allowed value for bootMode [0..4]

// ------------------------------------------------------------------------------

  // ----------------------------------------
  // MCU INITIALIZATION
  // ----------------------------------------
  
  // Initialize RES, CLK  and RDYRES_
  pinMode(CLK, OUTPUT);                         // Configure CLK and set it NOT ACTIVE
  digitalWrite(CLK, LOW);
  pinMode(RES, OUTPUT);                         // Configure RES and set it NOT ACTIVE
  digitalWrite(RES, LOW);
  pinMode(RDYRES_, OUTPUT);                     // Configure RDYRES_ and set it ACTIVE to reset the READY FF
  digitalWrite(RDYRES_, LOW);

  // Initialize INTR, RAMEN_, READY and HOLDRES_
  pinMode(INTR, OUTPUT);                        // Configure INTR and set it NOT ACTIVE
  digitalWrite(INTR, LOW);
  pinMode(RAMEN_, OUTPUT);                      // Configure RAMEN_ as output and set it NOT ACTIVE
  digitalWrite(RAMEN_, HIGH);
  pinMode(READY, INPUT);                        // Configure READY as input
  pinMode(HOLDRES_, OUTPUT);                    // Configure HOLDRES_ as output and set it ACTIVE to reset the HOLREQ FF
  digitalWrite(HOLDRES_, LOW);

  // Initialize D0-D7, A0-A2, IOM_, RD_ and WR_
  pinMode(D0, INPUT_PULLUP);                    // Configure D0-D7 as input with pull-up
  pinMode(D1, INPUT_PULLUP); 
  pinMode(D2, INPUT_PULLUP); 
  pinMode(D3, INPUT_PULLUP); 
  pinMode(D4, INPUT_PULLUP); 
  pinMode(D5, INPUT_PULLUP); 
  pinMode(D6, INPUT_PULLUP); 
  pinMode(D7, INPUT_PULLUP); 
  pinMode(AD0, INPUT_PULLUP);                   // Configure A0-A1 as input with pull-up
  pinMode(AD1, INPUT_PULLUP);
  pinMode(IOM_, INPUT_PULLUP);                  // Configure IOM_ as input with pull-up
  pinMode(RD_, INPUT_PULLUP);                   // Configure RD_ as input with pull-up
  pinMode(WR_, INPUT_PULLUP);                   // Configure WR_ as input with pull-up

  // Reset the V20 CPU and load a byte into RAM just to pulse the ALE line resetting the HALT 
  // status led if turned on at power up (see U7 in the A250220 schematic).
  // I don't like to see the red HALT led on after a power on when the RTC is asking for
  // a valid date/time...
  singlePulsesResetV20();
  digitalWrite(RDYRES_, HIGH);                  // Set RDYRES_ NOT ACTIVE
  storeOneByte(0x00, 0x0000);

  // Initialize MCU_RTS and MCU_CTS and reset uTerm (A071218-R250119) if present
  pinMode(MCU_CTS_, INPUT_PULLUP);              // MCU_CTS_ parked (not used)
  pinMode(MCU_RTS_, OUTPUT);
  digitalWrite(MCU_RTS_, LOW);                  // Reset uTerm (A071218-R250119)
  delay(100); 
  digitalWrite(MCU_RTS_, HIGH); 
  delay(500);                                   // Give the time to exit from reset

  // Check USER Key for boot mode changes 
  pinMode(USER, INPUT_PULLUP);                  // Read USER Key to enter into the boot mode selection
  if (!digitalRead(USER)) bootSelection = 1;
  pinMode(USER, OUTPUT);                        // Set USER as oupt for USER led...
  digitalWrite(USER, HIGH);                     // ...and set it OFF

  // Read the stored Disk Set. If not valid set it to 0
  diskSet = EEPROM.read(diskSetAddr);
  if (diskSet >= maxDiskSet) 
  {
    EEPROM.update(diskSetAddr, 0);
    diskSet =0;
  }

  // Print some system information
  Serial.begin(115200);
  Serial.print(F("\r\n\nV20-MBC - A250220\r\n - I/O Subsystem - S260320-R230520\r\n"));
  Serial.println();

  // Print if the input serial buffer is 128 bytes wide (this is needed for xmodem protocol support)
  if (SERIAL_RX_BUFFER_SIZE >= 128) Serial.println(F(": Found extended serial Rx buffer"));

  // Read the CPU speed mode
  clockMode = EEPROM.read(clockModeAddr);       // Read the previous stored value
  if (clockMode > 1)                            // Check if it is a valid value, otherwise set it to low speed
  // Not a valid value. Set it to low speed
  {
    EEPROM.update(clockModeAddr, 1);
    clockMode = 1;
  }

  // Print the V20 clock speed mode
  Serial.print(F(": V20 clock set at "));
  if (clockMode) Serial.print(CLOCK_LOW);
  else Serial.print(CLOCK_HIGH);
  Serial.println("MHz");

  // Initialize the I2C (IOEXP port) and search for the GPIO optional modules
  Wire.begin();                                 // Wake up I2C bus
  Wire.beginTransmission(GPIOEXP_ADDR);
  if (Wire.endTransmission() == 0) moduleGPIO = 1;  // Set to 1 if GPIO Module is found

  // Print RTC and GPIO informations if found
  foundRTC = autoSetRTC();                      // Check if RTC is present and initialize it as needed
  if (moduleGPIO) Serial.println(F(": Found GPE Option"));
  
  // Print CP/M Autoexec on cold boot status
  Serial.print(F(": CP/M Autoexec is "));
  if (EEPROM.read(autoexecFlagAddr) > 1) EEPROM.update(autoexecFlagAddr, 0); // Reset AUTOEXEC flag to OFF if invalid
  autoexecFlag = EEPROM.read(autoexecFlagAddr); // Read the previous stored AUTOEXEC flag
  if (autoexecFlag) Serial.println("ON");
  else Serial.println("OFF");

  // ----------------------------------------
  // BOOT SELECTION AND SYS PARAMETERS MENU
  // ----------------------------------------

  // Boot selection and system parameters menu if requested
  mountSD(&filesysSD); mountSD(&filesysSD);     // Try to mount the SD volume
  bootMode = EEPROM.read(bootModeAddr);         // Read the previous stored boot mode
  if ((bootSelection == 1 ) || (bootMode > maxBootMode))
  // Enter in the boot selection menu if USER key was pressed at startup 
  // or an invalid bootMode code was read from internal EEPROM
  {
    while (Serial.available() > 0)              // Flush input serial Rx buffer
    {
      Serial.read();
    }
    Serial.println();
    Serial.println(F(": Select boot mode or system parameters:\r\n"));
    if (bootMode <= maxBootMode)
    // Previous valid boot mode read, so enable '0' selection
    {
      minBootChar = '0';
      Serial.print(F(" 0: No change ("));
      Serial.print(bootMode + 1);
      Serial.println(")");
    }
    Serial.println(F(" 1: iLoad"));
    Serial.println(F(" 2: iLoad-80"));
    Serial.println(F(" 3: Autoboot"));
    Serial.println(F(" 4: Autoboot-80"));
    Serial.print(F(" 5: Load OS from "));
    printOsName(diskSet);
    Serial.print("\r\n 6: Change ");
    printOsName(diskSet);
    Serial.println();
    Serial.print(F(" 7: Change V20 clock speed (->"));
    if (clockMode) Serial.print(CLOCK_HIGH);
    else Serial.print(CLOCK_LOW);
    Serial.println("MHz)");
    Serial.print(F(" 8: Change CP/M Autoexec (->"));
    if (!autoexecFlag) Serial.print("ON");
    else Serial.print("OFF");
    Serial.println(")");

    // If RTC module is present add a menu choice for it
    if (foundRTC)
    {
      Serial.println(F(" 9: Change RTC time/date"));
      maxSelChar = '9';
    }

    // Ask a choice
    Serial.println();
    timeStamp = millis();
    Serial.print("Enter your choice >");
    do
    {
      blinkIOSled(&timeStamp);
      inChar = Serial.read();
    }               
    while ((inChar < minBootChar) || (inChar > maxSelChar));
    Serial.print(inChar);
    Serial.println("  Ok");

    // Make the selected action for the system paramters choice
    switch (inChar)
    {
      case '7':                                 // Change the clock speed of the V20 CPU
        clockMode = !clockMode;                 // Toggle V20 clock speed mode (High/Low)
        EEPROM.update(clockModeAddr, clockMode);// Save it to the internal EEPROM
      break;

      case '8':                                 // Toggle CP/M AUTOEXEC execution on cold boot
        autoexecFlag = !autoexecFlag;           // Toggle AUTOEXEC executiont status
        EEPROM.update(autoexecFlagAddr, autoexecFlag); // Save it to the internal EEPROM
      break;

      case '6':                                 // Change current Disk Set
        Serial.println(F("\r\nPress CR to accept, ESC to exit or any other key to change"));
        iCount = diskSet;
        do
        {
          // Print the OS name of the next Disk Set
          iCount = (iCount + 1) % maxDiskSet;
          Serial.print("\r ->");
          printOsName(iCount);
          Serial.print(F("                 \r"));
          while (Serial.available() > 0) Serial.read();   // Flush serial Rx buffer
          while(Serial.available() < 1) blinkIOSled(&timeStamp);  // Wait a key
          inChar = Serial.read();
        }
        while ((inChar != 13) && (inChar != 27)); // Continue until a CR or ESC is pressed
        Serial.println();
        Serial.println();
        if (inChar == 13)                       // Set and store the new Disk Set if required
        {
           diskSet = iCount;
           EEPROM.update(diskSetAddr, iCount);
        }
      break;

      case '9':                                 // Change RTC Date/Time
        ChangeRTC();                            // Change RTC Date/Time if requested
      break;
    };
    
    // Save selected boot program if changed
    bootMode = inChar - '1';                    // Calculate bootMode from inChar. Note that bootMode store only
                                                //  a value to select what program to load and boot. Value 0 corresponds
                                                //  to the menu choice '1' and so on. Max allowed value is maxBootMode
                                                //  that corresponds to the highest boot menu selection ('5' in this case).
                                                //  Others menu selection are ignored because are system parameters setting.
    if (bootMode <= maxBootMode) EEPROM.update(bootModeAddr, bootMode); // Save to the internal EEPROM if required
    else bootMode = EEPROM.read(bootModeAddr);  // Reload boot mode if it was selected '0' or > '5' (system parameter
                                                //  or no change)
  }

  // Print current Disk Set and OS name (if OS boot is enabled)
  if (bootMode == 4)
  {
    Serial.print(F("IOS: Current "));
    printOsName(diskSet);
    Serial.println();
  }
 
  // ----------------------------------------
  // BOOT LOAD INITIALIZATION
  // ----------------------------------------

  // Get the load starting address of the binary executable (flat binary model) to load and boot, and its size if 
  // stored in the flash. After the load phase a "far jump" is made to the load starting address, so the first executed
  // instruction of the binary executable must start at the first loaded byte.
  //
  // When related to 8088 executables, the load starting address is the displacement of the segment at 0x0000.
  // When related to 8080 executables, the load starting address is the displacement of the segment SEG_8080CODE.
  //
  // NOTE: all the binary files must be inside the 64KB (65536 bytes) size limit
  //
  switch (bootMode)
  {
    case 0:                                     // Load iLoad from flash
      BootImage = (byte *) pgm_read_word (&flahBootTable[0]); 
      BootImageSize = sizeof(boot_A_);
      BootStrAddr = boot_A_StrAddr;
    break;
    
    case 1:                                     // Load iLoad-80 from SD
      fileNameSD = ILOAD80FN;
      BootStrAddr = ILOAD80ADDR;
    break;
        
    case 2:                                     // Load AUTOBOOT.BIN from SD (load an user executable binary file)
      fileNameSD = AUTOFN;
      BootStrAddr = AUTSTRADDR;
    break;

    case 3:                                     // Load AU80BOOT.BIN from SD (load an user 8080 executable binary file)
      fileNameSD = AUTO80FN;
      BootStrAddr = AUTO80STRADDR;
      autoboot80 = 1;                           // Set the flag for the dual-stage boot (required for 8080 binaries)
      loadSegment = SEG_8080CODE;               // Set the destination segment to load the 8080 code into RAM
    break;

    case 4:                                     // Load an OS from current Disk Set on SD
      switch (diskSet)
      {
      case 0:                                   // CP/M 2.2 (8080 mode)
        fileNameSD = CPMFN;
        BootStrAddr = CPMSTRADDR;
        autoboot80 = 1;                         // Set the flag for the dual-stage boot (required for 8080 binaries)
        loadSegment = SEG_8080CODE;             // Set the destination segment to load the 8080 code into RAM
      break;

      case 1:                                   // CP/M-86
      //
      // NOTE:
      // CP/M-86 has the load starting address (address where to start the loading of the code into RAM)
      // different from the address (offset) of the first instruction to execute (CPM86JMPADDR), so both
      // the address must be specified.
      // More, also the segment of the first starting address (CPM86JMPSEG) is not 0x0000 and must be specified
      //
        fileNameSD = CPM86FN;
        JumpStrAddr = CPM86JMPADDR;             // Starting addres of the fisrt instruction to execute ("far" jump)
        V20initJumpSegm = CPM86JMPSEG;          // Segment used for the isrt instruction to execute ("far" jump)
        BootStrAddr = CPM86STRADDR;             // Starting address where start to load the code 
      break;
      }
    break;
  }
  
  // ----------------------------------------
  // V20 INITIALIZATION
  // ----------------------------------------

  singlePulsesResetV20();                       // Reset the V20 CPU
  digitalWrite(RDYRES_, HIGH);                  // Set RDYRES_ NOT ACTIVE
  
  // Set the offset for the "far jump" after the CPU reset
  if (!autoboot80)
  // It is 8088 code
  {
    if (JumpStrAddr == 0)
    // The starting address where start to load the code is equal to the starting address
    // of the first instruction to execute
    {
      V20initJumpAddr = BootStrAddr;
    }
    else
    // The starting address where start to load the code is differen from the starting address
    // of the first instruction to execute
    {
      V20initJumpAddr = JumpStrAddr;
    }
  }
  else
  // It is 8080 code, so the offset is the one for the SWITCH-80 utility (dual-stage boot)
  {
    V20initJumpAddr = SWCH80STRADDR;
    FirstStageStrAddr = BootStrAddr;            // Save the fist tage starting address
  }

  // Set the register segment DS = 0xFFFF (for storing the "far jump" instruction)
  if (debug > 0) Serial.println(F("* * * LOAD DS 0xFFFF * * *\r\n"));
  loadDS(0xffff);
  
  // Store a "far jump" to 0x0000:BootStrAddr at address 0xFFFF:0x0000 (first instruction after a reset)
  if (debug > 0) Serial.println(F("* * * STORE JMPF 0x0000:V20initJumpAddr * * *\r\n"));
  storeOneByte(JMPF_OPCODE, 0x0000);            // Store: JMPF
  storeOneByte(lowByte(V20initJumpAddr), 0x0001);   // Store low byte of the starting address
  storeOneByte(highByte(V20initJumpAddr), 0x0002);  // Store high byte of the starting address
  storeOneByte(lowByte(V20initJumpSegm), 0x0003);   // Store low byte of the starting segment
  storeOneByte(highByte(V20initJumpSegm), 0x0004);   // Store high byte of the starting segment

  // Set the register segment DS = loadSegment  (destination segment for program load)
  if (debug > 0) Serial.println(F("* * * LOAD DS loadSegment * * *\r\n"));
  loadDS(loadSegment);

  // ----------------------------------------
  // V20 BOOT LOAD
  // ----------------------------------------

  // Execute the load of the selected file on SD or image on flash
  if (bootMode > 0)
  // Load from SD
  {
    openFileSD(fileNameSD);

    // Read the selected file from SD and load it into RAM until an EOF is reached
    Serial.print("IOS: Loading boot program (");
    Serial.print(fileNameSD);
    Serial.print(")...");
    loadedBinSize = loadSDfileToRAM(BootStrAddr);
  }
  else
  // Load from flash
  {
    Serial.print("IOS: Loading boot program...");
    for (word i = 0; i < BootImageSize; i++)
    // Write boot program into external RAM
    {
      storeOneByte(pgm_read_byte(BootImage + i), BootStrAddr + i);  // Write current data byte into RAM
    }
  }
  Serial.println(" Done");
   
  //  End of RAM store. Now append an HLT to be sure to execute only the "feeded" stream
  if (debug > 0) Serial.println(F("* * * EXECUTE HALT * * *\r\n"));
  FeedOneByte(HLT_OPCODE);                      // Feed the HLT instruction
  pulseClock(15);                               // Some clock pulses to let the CPU execute the HLT instruction
  delay(300);                                   // Give a visual feedback of the HALT state at the end of RAM load

  // Do a second stage boot if needed (for 8080 code)
  if (autoboot80)
  // A dual-stage boot is needed for an 8080 binary previously loaded. Now load the Switch-80 support utility
  {
    
    // Load the Switch-80 utility
    singlePulsesResetV20();                     // Reset the V20 CPU
    fileNameSD = SWCH80FN;                      // Set the Switch-80 utility binary file
    loadDS(0x0000);                             // Set the destination segment to load Switch-80 into RAM
    BootStrAddr = SWCH80STRADDR;                // Set the starting address
    openFileSD(fileNameSD);                     // Open the binary file to load...
    Serial.print("IOS: Loading boot program (");
    Serial.print(fileNameSD);
    Serial.print(")...");
    loadSDfileToRAM(BootStrAddr);               // ...and load it
    Serial.println(" Done");

    // Store the previusly loaded 8080 code starting address into the corresponding Switch-80 data area
    storeOneByte(lowByte(FirstStageStrAddr), SWITCH80_STRADDR);
    storeOneByte(highByte(FirstStageStrAddr), SWITCH80_STRADDR + 1);
    
    // Store the previusly loaded 8080 code size (in bytes) into the corresponding Switch-80 data area
    storeOneByte(lowByte(loadedBinSize), SWITCH80_SIZE);
    storeOneByte(highByte(loadedBinSize), SWITCH80_SIZE + 1);
    
    // Complete the load adding a HLT and executing it
    FeedOneByte(HLT_OPCODE);                    // Feed the HLT instruction
    pulseClock(15);                             // Some clock pulses to let the CPU execute the HLT instruction
    delay(300);                                 // Give a visual feedback of the HALT state at the end of RAM load
  }

  // ----------------------------------------
  // V20 RUN START
  // ----------------------------------------
  
  // Reset the CPU again before let it run from RAM
  digitalWrite(RDYRES_, LOW);                   // WAIT FF forced to reset
  singlePulsesResetV20();
  if (debug > 0) Serial.println(F(" * V20 CPU 2nd RESET Done * \r\n"));
  digitalWrite(RDYRES_, HIGH);                  // WAIT FF active from now
  digitalWrite(HOLDRES_, HIGH);                 // HOLREQ FF active from now
  digitalWrite(RES, HIGH);                      // Activate the RESET signal

  // Initialize CLK @ 4/8MHz (@ Fosc = 16MHz). CPU clock_freq = (Atmega_clock) / ((OCR2 + 1) * 2)
  ASSR &= ~(1 << AS2);                          // Set Timer2 clock from system clock
  TCCR2 |= (1 << CS20);                         // Set Timer2 clock to "no prescaling"
  TCCR2 &= ~((1 << CS21) | (1 << CS22));
  TCCR2 |= (1 << WGM21);                        // Set Timer2 CTC mode
  TCCR2 &= ~(1 << WGM20);
  TCCR2 |= (1 <<  COM20);                       // Set "toggle OC2 on compare match"
  TCCR2 &= ~(1 << COM21);
  OCR2 = clockMode;                             // Set the compare value to toggle OC2 (1 = low or 0 = high)
  pinMode(CLK, OUTPUT);                         // Set OC2 as output and start to output the clock
  noInterrupts();
  TCCR1A=0;
  TCCR1B=0;
  TCNT1=0;
  OCR1A=1288;
  TCCR1B|= (1 << WGM12);
  TCCR1B|= (1 << CS12) | (1 << CS10);  //1024 prescaler
  TIMSK |=( 1 << OCIE1A);
  //disable the timer interrupt;
  timer_interrupt_enabled=0;
  interrupts();
  Serial.println("IOS: V20 CPU is running from now\r\n");

  // Flush serial Rx buffer
  while (Serial.available() > 0) Serial.read();

  // Leave the V20 CPU running
  delay(1);                                     // Just to be sure...
  digitalWrite(RES, LOW);                       // Release V20 from reset and let it run
}

// ------------------------------------------------------------------------------

ISR(TIMER1_COMPA_vect)
{
  timer_event = 1;
}

void loop()
{
  if ((PIN_READY) == 0)                         // READY = 0 ?
  // I/O , interrupt operaton requested, or CPU HALT
  {
    if ((PIN_WR_) == 0)                         // WR_ = 0 ?
    // I/O WRITE bus operation requested

    // ----------------------------------------
    // VIRTUAL I/O WRITE OPERATIONS ENGINE
    // ----------------------------------------
    
    {
      // Read D0-D7 and A0-A1
      ioAddress = PIN_A1A0;                     // Read V20 address bus line A1-A0
      ioData = PINA & B11110110;                // Read V20 data bus D7-D0 (D0 and D3 cleared)
      ioData = ioData | ((PINA >> 3) & B00000001); // Adjust D0
      ioData = ioData | ((PINA << 3) & B00001000); // Adjust D3

      // Execute a write operation
      switch (ioAddress)
      {
        case 0:
          // .........................................................................................................
          //
          // AD1-AD0 = 0 (I/O write address = 0x00): EXECUTE WRITE OPCODE.
          //
          // Execute the previously stored I/O write opcode with the current data.
          // The code of the I/O write operation (Opcode) must be previously stored with a STORE OPCODE operation.
          // .........................................................................................................
          //
          
          execWriteOpcode();
        break;

        case 1:
          // .........................................................................................................
          //
          // AD1-AD0 = 1 (I/O write address = 0x01): STORE OPCODE.
          //
          // Store (write) an "I/O operation code" (Opcode) and reset the exchanged bytes counter.
          //
          // NOTE 1: An Opcode can be a write or read Opcode, if the I/O operation is read or write.
          // NOTE 2: the STORE OPCODE operation must always precede an EXECUTE WRITE OPCODE or EXECUTE READ OPCODE 
          //         operation.
          // NOTE 3: For multi-byte read opcode (as DATETIME) read sequentially all the data bytes without to send
          //         a STORE OPCODE operation before each data byte after the first one.
          // .........................................................................................................
          //
          // .........................................................................................................
          //
          // Currently defined Opcodes for I/O write operations:
          //
          //   Opcode     Name            Exchanged bytes
          // -------------------------------------------------
          // Opcode 0x00  USER LED        1
          // Opcode 0x01  SERIAL TX       1
          // Opcode 0x03  GPIOA Write     1
          // Opcode 0x04  GPIOB Write     1
          // Opcode 0x05  IODIRA Write    1
          // Opcode 0x06  IODIRB Write    1
          // Opcode 0x07  GPPUA Write     1
          // Opcode 0x08  GPPUB Write     1
          // Opcode 0x09  SELDISK         1
          // Opcode 0x0A  SELTRACK        2
          // Opcode 0x0B  SELSECT         1  
          // Opcode 0x0C  WRITESECT       512
          // Opcode 0xFF  No operation    1
          // Opcode 0x0D  Enable/disable timer interrupt.
          //
          // Currently defined Opcodes for I/O read operations:
          //
          //   Opcode     Name            Exchanged bytes
          // -------------------------------------------------
          // Opcode 0x80  USER KEY        1
          // Opcode 0x81  GPIOA Read      1
          // Opcode 0x82  GPIOB Read      1
          // Opcode 0x84  DATETIME        7
          // Opcode 0x85  ERRDISK         1
          // Opcode 0x86  READSECT        512
          // Opcode 0x87  SDMOUNT         1
          // Opcode 0x88  ATXBUFF         1
          // Opcode 0xFF  No operation    1
          //
          // See details in the varius Opcodes implementation.
          //
          
          ioOpcode = ioData;                    // Store the I/O operation code (Opcode)
          ioByteCnt = 0;                        // Reset the exchanged bytes counter
        break;
        
        case 2:

          // NOT USED - RESERVED
        
        break;

        case 3:
          uint8_t checkpoint = ioData;     //the byte written 
          

              Serial.print("Checkpoint: ");
              Serial.print(checkpoint);
              Serial.print("\r\n");
          
          
         
          
         break;
      }
      if (debug > 1) 
      {
        Serial.print("\r\n ioData = ");
        Serial.println(ioData);
      }
      
      // Control bus sequence to exit from a wait state (I/O bus write cycle)
      POUT_0_RDYRES_;                           // RDYRES_ = LOW: Now is safe reset WAIT FF (exiting from WAIT state)
      POUT_1_RDYRES_;                           // RDYRES_ = HIGH
      
      // Time critical section!!!
      noInterrupts();                           // !!! Start of a time critical section. No interrupt allowed
      POUT_0_HOLDRES_;                          // !!! HOLDRES_ = LOW: Resume V20 from HiZ (reset HOLD FF)
      POUT_1_HOLRES_;                           // !!! HOLDRES_ = HIGH
      interrupts();                             // !!! End of a time critical section. Interrupt resumed
    }
    else 
      if ((PIN_RD_) == 0)                       // RD_ == 0 ?
      // I/O READ bus operation requested


      // ----------------------------------------
      // VIRTUAL I/O READ OPERATIONS ENGINE
      // ----------------------------------------
     
      {
        ioAddress = PIN_A1A0;                   // Read V20 address bus line A1-A0
        ioData = 0;                             // Clear input data buffer
      
        switch (ioAddress)
        {
          case 0:
            // .........................................................................................................
            //
            // AD1-AD0 = 0 (I/O read address = 0x00): EXECUTE READ OPCODE.
            //
            // Execute the previously stored I/O read operation with the current data.
            // The code of the I/O operation (Opcode) must be previously stored with a STORE OPCODE operation.
            //
            // NOTE: For multi-byte read opcode (as DATETIME) read sequentially all the data bytes without to send
            //       a STORE OPCODE operation before each data byte after the first one.
            // .........................................................................................................
            //
            
            execReadOpcode();
          break;
          
          case 1:
            // .........................................................................................................
            //
            // AD1-AD0 = 1 (I/O read address = 0x01): SERIAL RX.
            //
            // Execute a Serial I/O Read operation.
            // .........................................................................................................
            //
            //
            // SERIAL RX:
            //
            //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
            //                            ---------------------------------------------------------
            //                             D7 D6 D5 D4 D3 D2 D1 D0    ASCII char read from serial
            //
            // NOTE 1: If there is no input char, a value 0xFF is forced as input char.
            // NOTE 2: The INTR signal is always reset (set to LOW) after this I/O operation.
            // NOTE 3: This I/O do not require any previous STORE OPCODE operation.
            //
            
            ioData = 0xFF;
            if (Serial.available() > 0)
            {
              ioData = Serial.read();
            }
            POUT_0_INTR;                        // INTR = LOW: Reset the INTR signal (if used).
          break;

          case 2:
            // .........................................................................................................
            //
            // AD1-AD0 = 2 (I/O read address = 0x02): SYSFLAGS.
            //
            // Read various system flags.
            // .........................................................................................................
            //
            //
            // SYSFLAGS (Various system flags):
            //
            //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
            //                            ---------------------------------------------------------
            //                              X  X  X  X  X  X  X  0    AUTOEXEC not enabled
            //                              X  X  X  X  X  X  X  1    AUTOEXEC enabled
            //                              X  X  X  X  X  X  0  X    DS3231 RTC not found
            //                              X  X  X  X  X  X  1  X    DS3231 RTC found
            //                              X  X  X  X  X  0  X  X    Serial RX buffer empty
            //                              X  X  X  X  X  1  X  X    Serial RX char available
            //
            // NOTE 1: Currently only D0-D2 are used.
            // NOTE 2: This I/O do not require any previous STORE OPCODE operation.
            //
            
            ioData = autoexecFlag | (foundRTC << 1) | ((Serial.available() > 0) << 2) | (timer_interrupt_enabled << 5);
          break;

          case 3:

            // NOT USED - RESERVED

          break;
        }

        // Current output on data bus
        DDRA = 0xFF;                            // Configure data bus D0-D7 (PA0-PA7) as output
        PORTA = ioData & B11110110;             // Current output on data bus (D0 and D3 cleared)
        PORTA = PORTA | ((ioData >> 3) & B00000001); // Adjust D0
        PORTA = PORTA | ((ioData << 3) & B00001000); // Adjust D3

        // Control bus sequence to exit from a wait state (M I/O read cycle)
        POUT_0_RDYRES_;                         // RDYRES_ = LOW: Now is safe reset WAIT FF (exiting from WAIT state)
        delayMicroseconds(1);                   // Wait 2us just to be sure that the V20 reads the data and goes HiZ
        DDRA = 0x00;                            // Set data bus D0-D7 as input with pull-up
        PORTA = 0xFF;
        POUT_1_RDYRES_;                         // RDYRES_ = HIGH: Now V20 in HiZ (HOLD), so it's safe deactivate RDYRES_
        
        // Time critical section!!!
        noInterrupts();                         // !!! Start of a time critical section. No interrupt allowed
        POUT_0_HOLDRES_;                        // !!! HOLDRES_ = LOW: Resume V20 from HiZ (reset HOLD FF)
        POUT_1_HOLRES_;                         // !!! HOLDRES_ = HIGH
        interrupts();                           // !!! End of a time critical section. Interrupt resumed
      }
      else
        if (!digitalRead(INTA_))
        // I/O INTERRUPT ACKNOLEDGE bus operation requested

        // ----------------------------------------
        // VIRTUAL I/O INTERRUPT ACKNOLEDGE ENGINE
        // ----------------------------------------

        {

          // Now CPU is in the first of the two INTA_ bus cycles
          POUT_0_INTR;                          // INTR = LOW: Reset the INTR signal
          
          // Current output on data bus
          ioData = 33;                          // For now it is fixed
          DDRA = 0xFF;                          // Configure data bus D0-D7 (PA0-PA7) as output
          PORTA = ioData & B11110110;           // Current output on data bus (D0 and D3 cleared)
          PORTA = PORTA | ((ioData >> 3) & B00000001); // Adjust D0
          PORTA = PORTA | ((ioData << 3) & B00001000); // Adjust D3

          // Control bus sequence to exit from a wait state (Interrupt)
          POUT_0_RDYRES_;                       // RDYRES_ = LOW: Now is safe reset WAIT FF (exiting from WAIT state)
          delayMicroseconds(2);                 // Wait 2us (8 bus cycles @ 4MHz) just to be sure to execute both
                                                //  the two INTA bus cycles
          DDRA = 0x00;                          // Set data bus D0-D7 as input with pull-up
          PORTA = 0xFF;
          POUT_1_RDYRES_;                       // RDYRES_ = HIGH: Now V20 in HiZ (HOLD), so it's safe deactivate RDYRES_
          
          // Time critical section!!!
          noInterrupts();                       // !!! Start of a time critical section. No interrupt allowed
          POUT_0_HOLDRES_;                      // !!! HOLDRES_ = LOW: Resume V20 from HiZ (reset HOLD FF)
          POUT_1_HOLRES_;                       // !!! HOLDRES_ = HIGH
          interrupts();// !!! End of a time critical section. Interrupt resumed
          timer_event = 0;
        }
        else
        // CPU HALT state

        // ----------------------------------------
        // CPU HALT
        // ----------------------------------------
        
        {
          if (!haltFlag)
          {
            Serial.println("\r\nIOS: CPU HALT detected\r\n");
            haltFlag = 1;
          }
        }
  }
  if (Serial.available() && V20IntEnFlag) digitalWrite(INTR, HIGH);  // Generate an interrupt on serial Rx if enabled
  if (timer_event==1 && timer_interrupt_enabled) digitalWrite(INTR,HIGH);
}


// ------------------------------------------------------------------------------

// Generic routines

// ------------------------------------------------------------------------------


void blinkIOSled(unsigned long *timestamp)
// Blink led IOS using a timestamp
{
  if ((millis() - *timestamp) > 200)
  {
    digitalWrite(LED_IOS,!digitalRead(LED_IOS));
    *timestamp = millis();
  }
}


// ------------------------------------------------------------------------------

// RTC Module routines

// ------------------------------------------------------------------------------


byte decToBcd(byte val)
// Convert a binary byte to a two digits BCD byte
{
  return( (val/10*16) + (val%10) );
}

// ------------------------------------------------------------------------------

byte bcdToDec(byte val)
// Convert binary coded decimal to normal decimal numbers
{
  return( (val/16*10) + (val%16) );
}

// ------------------------------------------------------------------------------

void readRTC(byte *second, byte *minute, byte *hour, byte *day, byte *month, byte *year, byte *tempC)
// Read current date/time binary values and the temprerature (2 complement) from the DS3231 RTC
{
  byte    i;
  Wire.beginTransmission(DS3231_RTC);
  Wire.write(DS3231_SECRG);                     // Set the DS3231 Seconds Register
  Wire.endTransmission();
  // Read from RTC and convert to binary
  Wire.requestFrom(DS3231_RTC, 18);
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  Wire.read();                                  // Jump over the DoW
  *day = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
  for (i = 0; i < 10; i++) Wire.read();         // Jump over 10 registers
  *tempC = Wire.read();
}

// ------------------------------------------------------------------------------

void writeRTC(byte second, byte minute, byte hour, byte day, byte month, byte year)
// Write given date/time binary values to the DS3231 RTC
{
  Wire.beginTransmission(DS3231_RTC);
  Wire.write(DS3231_SECRG);                     // Set the DS3231 Seconds Register
  Wire.write(decToBcd(seconds));
  Wire.write(decToBcd(minutes));
  Wire.write(decToBcd(hours));
  Wire.write(1);                                // Day of week not used (always set to 1 = Sunday)
  Wire.write(decToBcd(day));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

// ------------------------------------------------------------------------------

byte autoSetRTC()
// Check if the DS3231 RTC is present and set the date/time at compile date/time if 
// the RTC "Oscillator Stop Flag" is set (= date/time failure).
// Return value: 0 if RTC not present, 1 if found.
{
  byte    OscStopFlag;

  Wire.beginTransmission(DS3231_RTC);
  if (Wire.endTransmission() != 0) return 0;    // RTC not found
  Serial.print("IOS: Found RTC DS3231 Module (");
  printDateTime(1);
  Serial.println(")");

  // Print the temperaturefrom the RTC sensor
  Serial.print("IOS: RTC DS3231 temperature sensor: ");
  Serial.print((int8_t)tempC);
  Serial.println("C");
  
  // Read the "Oscillator Stop Flag"
  Wire.beginTransmission(DS3231_RTC);
  Wire.write(DS3231_STATRG);                    // Set the DS3231 Status Register
  Wire.endTransmission();
  Wire.requestFrom(DS3231_RTC, 1);
  OscStopFlag = Wire.read() & 0x80;             // Read the "Oscillator Stop Flag"

  if (OscStopFlag)
  // RTC oscillator stopped. RTC must be set at compile date/time
  {
    // Convert compile time strings to numeric values
    seconds = compTimeStr.substring(6,8).toInt();
    minutes = compTimeStr.substring(3,5).toInt();
    hours = compTimeStr.substring(0,2).toInt();
    day = compDateStr.substring(4,6).toInt();
    switch (compDateStr[0]) 
      {
        case 'J': month = compDateStr[1] == 'a' ? 1 : month = compDateStr[2] == 'n' ? 6 : 7; break;
        case 'F': month = 2; break;
        case 'A': month = compDateStr[2] == 'r' ? 4 : 8; break;
        case 'M': month = compDateStr[2] == 'r' ? 3 : 5; break;
        case 'S': month = 9; break;
        case 'O': month = 10; break;
        case 'N': month = 11; break;
        case 'D': month = 12; break;
      };
    year = compDateStr.substring(9,11).toInt();

    // Ask for RTC setting al compile date/time
    Serial.println("IOS: RTC clock failure!");
    Serial.print("\nDo you want set RTC at IOS compile time (");
    printDateTime(0);
    Serial.print(")? [Y/N] >");
    timeStamp = millis();
    do
    {
      blinkIOSled(&timeStamp);
      inChar = Serial.read();
    }
    while ((inChar != 'y') && (inChar != 'Y') && (inChar != 'n') &&(inChar != 'N'));
    Serial.println(inChar);
 
    // Set the RTC at the compile date/time and print a message
    if ((inChar == 'y') || (inChar == 'Y'))
    {
      writeRTC(seconds, minutes, hours, day, month, year);
      Serial.print("IOS: RTC set at compile time - Now: ");
      printDateTime(1);
      Serial.println();
    }

    // Reset the "Oscillator Stop Flag" 
    Wire.beginTransmission(DS3231_RTC);
    Wire.write(DS3231_STATRG);                  // Set the DS3231 Status Register
    Wire.write(0x08);                           // Reset the "Oscillator Stop Flag" (32KHz output left enabled)
    Wire.endTransmission();
  }
  return 1;
}

// ------------------------------------------------------------------------------

void printDateTime(byte readSourceFlag)
// Print to serial the current date/time from the global variables.
//
// Flag readSourceFlag [0..1] usage:
//    If readSourceFlag = 0 the RTC read is not done
//    If readSourceFlag = 1 the RTC read is done (global variables are updated)
{
  if (readSourceFlag) readRTC(&seconds, &minutes, &hours, &day,  &month,  &year, &tempC);
  print2digit(day);
  Serial.print("/");
  print2digit(month);
  Serial.print("/");
  print2digit(year);
  Serial.print(" ");
  print2digit(hours);
  Serial.print(":");
  print2digit(minutes);
  Serial.print(":");
  print2digit(seconds);
}

// ------------------------------------------------------------------------------

void print2digit(byte data)
// Print a byte [0..99] using 2 digit with leading zeros if needed
{
  if (data < 10) Serial.print("0");
  Serial.print(data);
}

// ------------------------------------------------------------------------------

byte isLeapYear(byte yearXX)
// Check if the year 2000+XX (where XX is the argument yearXX [00..99]) is a leap year.
// Returns 1 if it is leap, 0 otherwise.
// This function works in the [2000..2099] years range. It should be enough...
{
  if (((2000 + yearXX) % 4) == 0) return 1;
  else return 0;
}

// ------------------------------------------------------------------------------

void ChangeRTC()
// Change manually the RTC Date/Time from keyboard
{
  byte    leapYear;   //  Set to 1 if the selected year is bissextile, 0 otherwise [0..1]

  // Read RTC
  readRTC(&seconds, &minutes, &hours, &day,  &month,  &year, &tempC);

  // Change RTC date/time from keyboard
  tempByte = 0;
  Serial.println("\nIOS: RTC manual setting:");
  Serial.println("\nPress T/U to increment +10/+1 or CR to accept");
  do
  {
    do
    {
      Serial.print(" ");
      switch (tempByte)
      {
        case 0:
          Serial.print("Year -> ");
          print2digit(year);
        break;
        
        case 1:
          Serial.print("Month -> ");
          print2digit(month);
        break;

        case 2:
          Serial.print("             ");
          Serial.write(13);
          Serial.print(" Day -> ");
          print2digit(day);
        break;

        case 3:
          Serial.print("Hours -> ");
          print2digit(hours);
        break;

        case 4:
          Serial.print("Minutes -> ");
          print2digit(minutes);
        break;

        case 5:
          Serial.print("Seconds -> ");
          print2digit(seconds);
        break;
      }

      timeStamp = millis();
      do
      {
        blinkIOSled(&timeStamp);
        inChar = Serial.read();
      }
      while ((inChar != 'u') && (inChar != 'U') && (inChar != 't') && (inChar != 'T') && (inChar != 13));
      
      if ((inChar == 'u') || (inChar == 'U'))
      // Change units
        switch (tempByte)
        {
          case 0:
            year++;
            if (year > 99) year = 0;
          break;

          case 1:
            month++;
            if (month > 12) month = 1;
          break;

          case 2:
            day++;
            if (month == 2)
            {
              if (day > (daysOfMonth[month - 1] + isLeapYear(year))) day = 1;
            }
            else
            {
              if (day > (daysOfMonth[month - 1])) day = 1;
            }
          break;

          case 3:
            hours++;
            if (hours > 23) hours = 0;
          break;

          case 4:
            minutes++;
            if (minutes > 59) minutes = 0;
          break;

          case 5:
            seconds++;
            if (seconds > 59) seconds = 0;
          break;
        }
      if ((inChar == 't') || (inChar == 'T'))
      // Change tens
        switch (tempByte)
        {
          case 0:
            year = year + 10;
            if (year > 99) year = year - (year / 10) * 10; 
          break;

          case 1:
            if (month > 10) month = month - 10;
            else if (month < 3) month = month + 10;
          break;

          case 2:
            day = day + 10;
            if (day > (daysOfMonth[month - 1] + isLeapYear(year))) day = day - (day / 10) * 10;
            if (day == 0) day = 1;
          break;

          case 3:
            hours = hours + 10;
            if (hours > 23) hours = hours - (hours / 10 ) * 10;
          break;

          case 4:
            minutes = minutes + 10;
            if (minutes > 59) minutes = minutes - (minutes / 10 ) * 10;
          break;

          case 5:
            seconds = seconds + 10;
            if (seconds > 59) seconds = seconds - (seconds / 10 ) * 10;
          break;
        }
      Serial.write(13);
    }
    while (inChar != 13);
    tempByte++;
  }
  while (tempByte < 6);  

  // Write new date/time into the RTC
  writeRTC(seconds, minutes, hours, day, month, year);
  Serial.println(" ...done      ");
  Serial.print("IOS: RTC date/time updated (");
  printDateTime(1);
  Serial.println(")");
}


// ------------------------------------------------------------------------------

// V20 Boot routines

// ------------------------------------------------------------------------------


void pulseClock(word numPulse)
// Generate <numPulse> clock pulses on the CLK clock pin.
// The steady clk level is LOW,so one clock pulse is a 0-1-0 transition
{
  word    i;
  for (i = 0; i < numPulse; i++)
  // Generate one clock pulse
  {
    // Send one impulse (0-1-0) on the CLK output
    digitalWrite(CLK, HIGH);
    digitalWrite(CLK, LOW);
  }
}

// ------------------------------------------------------------------------------

void singlePulsesResetV20()
// Reset the V20 CPU using single pulses clock
{
  digitalWrite(RES, HIGH);                      // Set RES active
  pulseClock(1000);                             // (min. RES pulse) / (typ. clock per.) = (50uS / 250nS) = 200. Here 5x
  digitalWrite(RES, LOW);                       // Set RESET_ not active
}

// ------------------------------------------------------------------------------

void FeedOneByte(byte data)
// "Feed" one byte to the V20 CPU.
//
// Wait for a Read Bus Cycle and force a byte on the Data Bus putting the SRAM in HiZ.
// It is assumed here a Memory Read Bus Cycles (no check is done on IOM_).
// For more info about the V20 bus timing see the uPD70108H datasheet.
//
// NOTE: Two kind of errors are detected here and will abort any further operation:
//
//               1 = Timeout error waiting the Read Bus Cycle, 
//               2 = Unexpected bus state
//
{
  byte  i;
  //
  // Make clock pulses until RD_ is LOW
  //
  i = MAX_RD_CLKPULSES;                         // Set the max clock pulses to find a Read Bus Cycle
  do
  {
    if (i <= 0) 
    // Bus error: Timeout error waiting the Read Bus Cycle
    {
      Serial.println("\r\n\nIOS: Read Bus Cycle timeout");
      while (1);
    }

    pulseClock(1);
    i--;
  }
  while (digitalRead(RD_) == HIGH);
  //
  // Now RD_ = LOW so we are at the T1 state of a Read Bus Cycle
  //

  pulseClock(1);                                // Now at T2 bus state
  if (digitalRead(RD_) != LOW)
  // Bus error: Unexpected bus state
  {
    Serial.println("\r\n\nIOS: Unexpected bus state");
    while (1);
  }
  digitalWrite(RAMEN_, HIGH);                   // Put SRAM in HiZ
  DDRA = 0xFF;                                  // Configure data bus D0-D7 (PA0-PA7) as output
  PORTA = data & B11110110;                     // Put the byte on data bus (D0 and D3 cleared)
  PORTA = PORTA | ((data >> 3) & B00000001);    // Adjust D0
  PORTA = PORTA | ((data << 3) & B00001000);    // Adjust D3
  pulseClock(1);                                // Go to the T3 bus state
  DDRA = 0x00;                                  // Set data bus D0-D7 as input with pull-up
  PORTA = 0xFF;
  digitalWrite(RAMEN_, LOW);                    // Enable the SRAM
}

// ------------------------------------------------------------------------------

void loadDS(word dataSegment)
// Load <dataSegment> into the DS data segment register of the V20 CPU.
// For more info about the V20 registers see the uPD70108H datasheet.
//
{
  FeedOneByte(MOV_AX_IMM_OPCODE);
  FeedOneByte(lowByte(dataSegment));
  FeedOneByte(highByte(dataSegment));
  FeedOneByte(highByte(MOV_DS_AX_OPCODE));
  FeedOneByte(lowByte(MOV_DS_AX_OPCODE));
}

// ------------------------------------------------------------------------------

void storeOneByte(byte data, word memOffset)
// Store one byte into RAM at the given memory offset using the V20 CPU.
// The effective phisycal memory address will be the sum of the <memOffset> with
// the current content of the DS register left-shited by 4.
// For more info about the V20 physical addressing mode see the uPD70108H datasheet.
//
{
  FeedOneByte(highByte(MOV_MEM_IMM_OPCODE));
  FeedOneByte(lowByte(MOV_MEM_IMM_OPCODE));
  FeedOneByte(lowByte(memOffset));
  FeedOneByte(highByte(memOffset));
  FeedOneByte(data);
}

// ------------------------------------------------------------------------------

void openFileSD(const char *  fileNameSD)
// Mount a volume on SD and open the fileNameSD file
{
  // Mount a volume on SD
  if (mountSD(&filesysSD))
  // Error mounting. Try again
  {
    errCodeSD = mountSD(&filesysSD);
    if (errCodeSD)
    // Error again. Repeat until error disappears (or the user forces a reset)
    do
    {
      printErrSD(0, errCodeSD, NULL);
      waitKey();                              // Wait a key to repeat
      mountSD(&filesysSD);                    // New double try
      errCodeSD = mountSD(&filesysSD);
    }
    while (errCodeSD);
  }

  // Open the selected file to load
  errCodeSD = openSD(fileNameSD);
  if (errCodeSD)
  // Error opening the required file. Repeat until error disappears (or the user forces a reset)
  do
  {
    printErrSD(1, errCodeSD, fileNameSD);
    waitKey();                                // Wait a key to repeat
    errCodeSD = openSD(fileNameSD);
    if (errCodeSD != 3)
    // Try to do a two mount operations followed by an open
    {
      mountSD(&filesysSD);
      mountSD(&filesysSD);
      errCodeSD = openSD(fileNameSD);
    }
  }
  while (errCodeSD);
}

// ------------------------------------------------------------------------------

word loadSDfileToRAM(word BootStrAddr)
// Load the current binary opened file on SD into RAM starting from the relative address BootStrAddr.
//
// Retuns the number of loaded bytes. If the file is 65536 bytes long the returned size is 0.
//
// An error is generated If there is an attempt to overrun the 64KB address space, and the load is halted.
// This happens if the starting address + the file lenght is greater than 65536.
//
// An error is generated if the file to load is empty, and the load is halted.
//
{
  word    loadAddress;
  byte    overRun = 0;                          // Flag set to 1 if the end of the 64KB address space is reached
  byte    emptyFile = 1;                        // Set to 1 for an empty file

  do
  // If an error occurs repeat until error disappears (or the user forces a reset)
  {
    loadAddress = BootStrAddr;                  // Set the first address (displacement) to load from
                                                // (using the current data segment register DS)
    do
    // Read a "segment" of a SD sector and load it into RAM
    {
      errCodeSD = readSD(bufferSD, &numReadBytes);    // Read current "segment" (32 bytes) of the current SD serctor
      if (numReadBytes > 0) emptyFile = 0;      // Check for an empty file
      for (iCount = 0; iCount < numReadBytes; iCount++)
      // Load the read "segment" into RAM
      {
        if (overRun)
        // Overrun load error. Already reached the end of the 64KB address space
        {
          Serial.print(F("\r\n\nIOS: Overrun error - Load aborted!"));
          while (1);
        }
        storeOneByte(bufferSD[iCount], loadAddress);  // Write current data byte into RAM
        loadAddress++;
        if (loadAddress == 0) overRun = 1;      // Reached the end of the 64KB address space
      }
    }
    while ((numReadBytes == 32) && (!errCodeSD));     // If numReadBytes < 32 -> EOF reached
    if (errCodeSD)
    {
      printErrSD(2, errCodeSD, fileNameSD);
      waitKey();                                // Wait a key to repeat
      seekSD(0);                                // Reset the sector pointer
    }
  }
  while (errCodeSD);
  if (emptyFile)
  // Empty file error
  {
    Serial.print(F("\r\n\nIOS: Empty file - Load aborted!"));
    while (1);
  }
  return loadAddress - BootStrAddr;
}


// ------------------------------------------------------------------------------

// SD Disk routines (FAT16 and FAT32 filesystems supported) using the PetitFS library.
// For more info about PetitFS see here: http://elm-chan.org/fsw/ff/00index_p.html

// ------------------------------------------------------------------------------


byte mountSD(FATFS* fatFs)
// Mount a volume on SD: 
// *  "fatFs" is a pointer to a FATFS object (PetitFS library)
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
{
  return pf_mount(fatFs);
}

// ------------------------------------------------------------------------------

byte openSD(const char* fileName)
// Open an existing file on SD:
// *  "fileName" is the pointer to the string holding the file name (8.3 format)
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
{
  return pf_open(fileName);
}

// ------------------------------------------------------------------------------

byte readSD(void* buffSD, byte* numReadBytes)
// Read one "segment" (32 bytes) starting from the current sector (512 bytes) of the opened file on SD:
// *  "BuffSD" is the pointer to the segment buffer;
// *  "numReadBytes" is the pointer to the variables that store the number of read bytes;
//     if < 32 (including = 0) an EOF was reached).
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
//
// NOTE1: Each SD sector (512 bytes) is divided into 16 segments (32 bytes each); to read a sector you need to
//        to call readSD() 16 times consecutively
//
// NOTE2: Past current sector boundary, the next sector will be pointed. So to read a whole file it is sufficient 
//        call readSD() consecutively until EOF is reached
{
  UINT  numBytes;
  byte  errcode;
  errcode = pf_read(buffSD, 32, &numBytes);
  *numReadBytes = (byte) numBytes;
  return errcode;
}

// ------------------------------------------------------------------------------

byte writeSD(void* buffSD, byte* numWrittenBytes)
// Write one "segment" (32 bytes) starting from the current sector (512 bytes) of the opened file on SD:
// *  "BuffSD" is the pointer to the segment buffer;
// *  "numWrittenBytes" is the pointer to the variables that store the number of written bytes;
//     if < 32 (including = 0) an EOF was reached.
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
//
// NOTE1: Each SD sector (512 bytes) is divided into 16 segments (32 bytes each); to write a sector you need to
//        to call writeSD() 16 times consecutively
//
// NOTE2: Past current sector boundary, the next sector will be pointed. So to write a whole file it is sufficient 
//        call writeSD() consecutively until EOF is reached
//
// NOTE3: To finalize the current write operation a writeSD(NULL, &numWrittenBytes) must be called as last action
{
  UINT  numBytes;
  byte  errcode;
  if (buffSD != NULL)
  {
    errcode = pf_write(buffSD, 32, &numBytes);
  }
  else
  {
    errcode = pf_write(0, 0, &numBytes);
  }
  *numWrittenBytes = (byte) numBytes;
  return errcode;
}

// ------------------------------------------------------------------------------

byte seekSD(word sectNum)
// Set the pointer of the current sector for the current opened file on SD:
// *  "sectNum" is the sector number to set. First sector is 0.
// The returned value is the resulting status (0 = ok, otherwise see printErrSD())
//
// NOTE: "secNum" is in the range [0..16383], and the sector addressing is continuos inside a "disk file";
//       16383 = (512 * 32) - 1, where 512 is the number of emulated tracks, 32 is the number of emulated sectors
//
{
  byte i;
  return pf_lseek(((unsigned long) sectNum) << 9);
}

// ------------------------------------------------------------------------------

void printErrSD(byte opType, byte errCode, const char* fileName)
// Print the error occurred during a SD I/O operation:
//  * "OpType" is the operation that generated the error (0 = mount, 1= open, 2 = read,
//     3 = write, 4 = seek);
//  * "errCode" is the error code from the PetitFS library (0 = no error);
//  * "fileName" is the pointer to the file name or NULL (no file name)
//
// ........................................................................
//
// Errors legend (from PetitFS library) for the implemented operations:
//
// ------------------
// mountSD():
// ------------------
// NOT_READY
//     The storage device could not be initialized due to a hard error or no medium.
// DISK_ERR
//     An error occured in the disk read function.
// NO_FILESYSTEM
//     There is no valid FAT partition on the drive.
//
// ------------------
// openSD():
// ------------------
// NO_FILE
//     Could not find the file.
// DISK_ERR
//     The function failed due to a hard error in the disk function, a wrong FAT structure or an internal error.
// NOT_ENABLED
//     The volume has not been mounted.
//
// ------------------
// readSD() and writeSD():
// ------------------
// DISK_ERR
//     The function failed due to a hard error in the disk function, a wrong FAT structure or an internal error.
// NOT_OPENED
//     The file has not been opened.
// NOT_ENABLED
//     The volume has not been mounted.
// 
// ------------------
// seekSD():
// ------------------
// DISK_ERR
//     The function failed due to an error in the disk function, a wrong FAT structure or an internal error.
// NOT_OPENED
//     The file has not been opened.
//
// ........................................................................
{
  if (errCode)
  {
    Serial.print("\r\nIOS: SD error ");
    Serial.print(errCode);
    Serial.print(" (");
    switch (errCode)
    // See PetitFS implementation for the codes
    {
      case 1: Serial.print("DISK_ERR"); break;
      case 2: Serial.print("NOT_READY"); break;
      case 3: Serial.print("NO_FILE"); break;
      case 4: Serial.print("NOT_OPENED"); break;
      case 5: Serial.print("NOT_ENABLED"); break;
      case 6: Serial.print("NO_FILESYSTEM"); break;
      default: Serial.print("UNKNOWN"); 
    }
    Serial.print(" on ");
    switch (opType)
    {
      case 0: Serial.print("MOUNT"); break;
      case 1: Serial.print("OPEN"); break;
      case 2: Serial.print("READ"); break;
      case 3: Serial.print("WRITE"); break;
      case 4: Serial.print("SEEK"); break;
      default: Serial.print("UNKNOWN");
    }
    Serial.print(" operation");
    if (fileName)
    // Not a NULL pointer, so print file name too
    {
      Serial.print(" - File: ");
      Serial.print(fileName);
    }
    Serial.println(")");
  }
}

// ------------------------------------------------------------------------------

void waitKey()
// Wait a key to continue
{
  while (Serial.available() > 0) Serial.read(); // Flush serial Rx buffer
  Serial.println(F("IOS: Check SD and press a key to repeat\r\n"));
  while(Serial.available() < 1);
}

// ------------------------------------------------------------------------------

void printOsName(byte currentDiskSet)
// Print the current Disk Set number and the OS name, if it is defined.
// The OS name is inside the file defined in DS_OSNAME
{
  Serial.print("Disk Set ");
  Serial.print(currentDiskSet);
  OsName[2] = currentDiskSet + 48;              // Set the Disk Set
  openSD(OsName);                               // Open file with the OS name
  readSD(bufferSD, &numReadBytes);              // Read the OS name
  if (numReadBytes > 0)
  // Print the OS name
  {
    Serial.print(" (");
    Serial.print((const char *)bufferSD);
    Serial.print(")");
  }
}


// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------

// Inline functions

// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------


// ------------------------------------------------------------------------------

// EXECUTE WRITE OPCODE (I/O write port address = 0x00)

// ------------------------------------------------------------------------------


void execWriteOpcode()
{
  switch (ioOpcode)
  // Execute the requested I/O WRITE Opcode. The 0xFF value is reserved as "No operation".
  {
    case  0x00:
      // USER LED:      
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                              x  x  x  x  x  x  x  0    USER Led off
      //                              x  x  x  x  x  x  x  1    USER Led on
          
      if (ioData & B00000001) digitalWrite(USER, LOW);
      else digitalWrite(USER, HIGH);
    break;
    
    case  0x01:
      // SERIAL TX:     
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    ASCII char to be sent to serial
          
      Serial.write(ioData);
    break;
    case  0x02:
      // RX IRQ FLAG:
      // Set or reset the flag to enable/disable the INTR line when a valid data is preset into the Rx serial buffer
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                              x  x  x  x  x  x  x  0    Rx IRQ off
      //                              x  x  x  x  x  x  x  1    Rx IRQ on
      //
      // NOTE: The default value after a reset/power on is 0 (Rx IRQ off)
      
      V20IntEnFlag = ioData & 1;
    break;

    case  0x03:
      // GPIOA Write (GPE Option):
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    GPIOA value (see MCP23017 datasheet)
          
      if (moduleGPIO) 
      {
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.write(GPIOA_REG);                  // Select GPIOA
        Wire.write(ioData);                     // Write value
        Wire.endTransmission();
      }
    break;
        
    case  0x04:
      // GPIOB Write (GPE Option): 
      //   
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    GPIOB value (see MCP23017 datasheet)
          
      if (moduleGPIO) 
      {
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.write(GPIOB_REG);                  // Select GPIOB
        Wire.write(ioData);                     // Write value
        Wire.endTransmission();
      }
    break;
        
    case  0x05:
      // IODIRA Write (GPE Option):
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    IODIRA value (see MCP23017 datasheet)
          
      if (moduleGPIO) 
      {
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.write(IODIRA_REG);                 // Select IODIRA
        Wire.write(ioData);                     // Write value
        Wire.endTransmission();
      }
    break;
        
    case  0x06:
      // IODIRB Write (GPE Option):
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    IODIRB value (see MCP23017 datasheet)
          
      if (moduleGPIO) 
      {
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.write(IODIRB_REG);                 // Select IODIRB
        Wire.write(ioData);                     // Write value
        Wire.endTransmission();
      }
    break;
        
    case  0x07:
      // GPPUA Write (GPE Option):
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    GPPUA value (see MCP23017 datasheet)
          
      if (moduleGPIO) 
      {
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.write(GPPUA_REG);                  // Select GPPUA
        Wire.write(ioData);                     // Write value
        Wire.endTransmission();
      }
    break;
        
    case  0x08:
      // GPPUB Write (GPIO Exp. Mod. ):
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    GPPUB value (see MCP23017 datasheet)
        
      if (moduleGPIO) 
      {
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.write(GPPUB_REG);                  // Select GPPUB
        Wire.write(ioData);                     // Write value
        Wire.endTransmission();
      }
    break;

    case  0x09:
      // DISK EMULATION
      // SELDISK - select the emulated disk number (binary). 100 disks are supported [0..99]:
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    DISK number (binary) [0..99]
      //
      //
      // Opens the "disk file" correspondig to the selected disk number, doing some checks.
      // A "disk file" is a binary file that emulates a disk using a LBA-like logical sector number.
      // Every "disk file" must have a dimension of 8388608 bytes, corresponding to 16384 LBA-like logical sectors
      //  (each sector is 512 bytes long), correspinding to 512 tracks of 32 sectors each (see SELTRACK and 
      //  SELSECT opcodes).
      // Errors are stored into "errDisk" (see ERRDISK opcode).
      //
      //
      // ...........................................................................................
      //
      // "Disk file" filename convention:
      //
      // Every "disk file" must follow the sintax "DSsNnn.DSK" where
      //
      //    "s" is the "disk set" and must be in the [0..9] range (always one numeric ASCII character)
      //    "nn" is the "disk number" and must be in the [00..99] range (always two numeric ASCII characters)
      //
      // ...........................................................................................
      //          
      //
      // NOTE 1: The maximum disks number may be lower due the limitations of the used OS (e.g. CP/M 2.2 supports
      //         a maximum of 16 disks)
      // NOTE 2: Because SELDISK opens the "disk file" used for disk emulation, before using WRITESECT or READSECT
      //         a SELDISK must be performed at first.
      
      if (ioData <= maxDiskNum)               // Valid disk number
      // Set the name of the file to open as virtual disk, and open it
      {
        diskName[2] = diskSet + 48;           // Set the current Disk Set
        diskName[4] = (ioData / 10) + 48;     // Set the disk number
        diskName[5] = ioData - ((ioData / 10) * 10) + 48;
        diskErr = openSD(diskName);           // Open the "disk file" corresponding to the given disk number
        
        
      }
      else diskErr = 16;                      // Illegal disk number
    break;

    case  0x0A:
      // DISK EMULATION
      // SELTRACK - select the emulated track number (word splitted in 2 bytes in sequence: DATA 0 and DATA 1):
      //
      //                I/O DATA 0:  D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    Track number (binary) LSB [0..255]
      //
      //                I/O DATA 1:  D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    Track number (binary) MSB [0..1]
      //
      //
      // Stores the selected track number into "trackSel" for "disk file" access.
      // A "disk file" is a binary file that emulates a disk using a LBA-like logical sector number.
      // The SELTRACK and SELSECT operations convert the legacy track/sector address into a LBA-like logical 
      //  sector number used to set the logical sector address inside the "disk file".
      // A control is performed on both current sector and track number for valid values. 
      // Errors are stored into "diskErr" (see ERRDISK opcode).
      //
      //
      // NOTE 1: Allowed track numbers are in the range [0..511] (512 tracks)
      // NOTE 2: Before a WRITESECT or READSECT operation at least a SELSECT or a SELTRAK operation
      //         must be performed
      
      if (!ioByteCnt)
      // LSB
      {
        trackSel = ioData;
      }
      else
      // MSB
      {
        trackSel = (((word) ioData) << 8) | lowByte(trackSel);
        if ((trackSel < 512) && (sectSel < 32))
        // Sector and track numbers valid
        {
          diskErr = 0;                      // No errors
        }
        else
        // Sector or track invalid number
            {
          if (sectSel < 32) diskErr = 17;     // Illegal track number
          else diskErr = 18;                  // Illegal sector number
        }
        ioOpcode = 0xFF;                      // All done. Set ioOpcode = "No operation"
      }
      ioByteCnt++;
    break;

    case  0x0B:
      // DISK EMULATION
      // SELSECT - select the emulated sector number (binary):
      //
      //                  I/O DATA:  D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    Sector number (binary) [0..31]
      //
      //
      // Stores the selected sector number into "sectSel" for "disk file" access.
      // A "disk file" is a binary file that emulates a disk using a LBA-like logical sector number.
      // The SELTRACK and SELSECT operations convert the legacy track/sector address into a LBA-like logical 
      //  sector number used to set the logical sector address inside the "disk file".
      // A control is performed on both current sector and track number for valid values. 
      // Errors are stored into "diskErr" (see ERRDISK opcode).
      //
      //
      // NOTE 1: Allowed sector numbers are in the range [0..31] (32 sectors)
      // NOTE 2: Before a WRITESECT or READSECT operation at least a SELSECT or a SELTRAK operation
      //         must be performed

      sectSel = ioData;
      if ((trackSel < 512) && (sectSel < 32))
      // Sector and track numbers valid
      {
        diskErr = 0;                        // No errors
      }
      else
      // Sector or track invalid number
      {
        if (sectSel < 32) diskErr = 17;     // Illegal track number
        else diskErr = 18;                  // Illegal sector number
      }
    break;
    case 0x0D:
        timeStamp = millis();
        blinkIOSled(&timeStamp);
        timer_interrupt_enabled = !timer_interrupt_enabled;
    break;
    case 0x0C:
      // DISK EMULATION
      // WRITESECT - write 512 data bytes sequentially into the current emulated disk/track/sector:
      //
      //                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    First Data byte
      //
      //                      |               |
      //                      |               |
      //                      |               |                 <510 Data Bytes>
      //                      |               |
      //
      //               I/O DATA 511: D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    512th Data byte (Last byte)
      //
      //
      // Writes the current sector (512 bytes) of the current track/sector, one data byte each call. 
      // All the 512 calls must be always performed sequentially to have a WRITESECT operation correctly done. 
      // If an error occurs during the WRITESECT operation, all subsequent write data will be ignored and
      //  the write finalization will not be done.
      // If an error occurs calling any DISK EMULATION opcode (SDMOUNT excluded) immediately before the WRITESECT 
      //  opcode call, all the write data will be ignored and the WRITESECT operation will not be performed.
      // Errors are stored into "diskErr" (see ERRDISK opcode).
      //
      // NOTE 1: Before a WRITESECT operation at least a SELTRACK or a SELSECT must be always performed
      // NOTE 2: Remember to open the right "disk file" at first using the SELDISK opcode
      // NOTE 3: The write finalization on SD "disk file" is executed only on the 512th data byte exchange, so be 
      //         sure that exactly 512 data bytes are exchanged.

      if (!ioByteCnt)
      // First byte of 512, so set the right file pointer to the current emulated track/sector first
      {
        if ((trackSel < 512) && (sectSel < 32) && (!diskErr))
        // Sector and track numbers valid and no previous error; set the LBA-like logical sector
        {
          diskErr = seekSD((trackSel *9) + sectSel);  // Set the starting point inside the "disk file"
                                                        //  generating a 14 bit "disk file" LBA-like 
                                                        //  logical sector address created as TTTTTTTTTSSSSS
        }
      }
          

      if (!diskErr)
      // No previous error (e.g. selecting disk, track or sector)
      {
        tempByte = ioByteCnt % 32;            // [0..31]
        bufferSD[tempByte] = ioData;          // Store current exchanged data byte in the buffer array
        if (tempByte == 31)
        // Buffer full. Write all the buffer content (32 bytes) into the "disk file"
        {
          diskErr = writeSD(bufferSD, &numWriBytes);
          if (numWriBytes < 32) diskErr = 19; // Reached an unexpected EOF
          if (ioByteCnt >= 511)
          // Finalize write operation and check result (if no previous error occurred)
          {
            if (!diskErr) diskErr = writeSD(NULL, &numWriBytes);
            ioOpcode = 0xFF;                  // All done. Set ioOpcode = "No operation"
          }
        }
      }
      ioByteCnt++;                            // Increment the counter of the exchanged data bytes
    break;
  }
  if ((ioOpcode != 0x0A) && (ioOpcode != 0x0C)) ioOpcode = 0xFF;    // All done for the single byte opcodes.
                                                                    //  Set ioOpcode = "No operation"
}


// ------------------------------------------------------------------------------

// EXECUTE READ OPCODE (I/O read port address = 0x00)

// ------------------------------------------------------------------------------


void execReadOpcode()
{
  switch (ioOpcode)
  // Execute the requested I/O READ Opcode. The 0xFF value is reserved as "No operation".
  {
    case  0x80:
      // USER KEY:      
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                              0  0  0  0  0  0  0  0    USER Key not pressed
      //                              0  0  0  0  0  0  0  1    USER Key pressed
            
      tempByte = digitalRead(USER);             // Save USER led status
      pinMode(USER, INPUT_PULLUP);              // Read USER Key
      ioData = !digitalRead(USER);
      pinMode(USER, OUTPUT); 
      digitalWrite(USER, tempByte);             // Restore USER led status
    break;

    case  0x81:
      // GPIOA Read (GPE Option):
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    GPIOA value (see MCP23017 datasheet)
      //
      // NOTE: a value 0x00 is forced if the GPE Option is not present
            
      if (moduleGPIO) 
      {
        
        // Set MCP23017 pointer to GPIOA
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.write(GPIOA_REG);
        Wire.endTransmission();
        
        // Read GPIOA
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.requestFrom(GPIOEXP_ADDR, 1);
        ioData = Wire.read();
      }
    break;

    case  0x82:
      // GPIOB Read (GPE Option):
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    GPIOB value (see MCP23017 datasheet)
      //
      // NOTE: a value 0x00 is forced if the GPE Option is not present
            
      if (moduleGPIO) 
      {
        
        // Set MCP23017 pointer to GPIOB
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.write(GPIOB_REG);
        Wire.endTransmission();
        
        // Read GPIOB
        Wire.beginTransmission(GPIOEXP_ADDR);
        Wire.requestFrom(GPIOEXP_ADDR, 1);
        ioData = Wire.read();
      }
    break;

    case  0x84:
      // DATETIME (Read date/time and temperature from the RTC. Binary values): 
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                I/O DATA 0   D7 D6 D5 D4 D3 D2 D1 D0    seconds [0..59]     (1st data byte)
      //                I/O DATA 1   D7 D6 D5 D4 D3 D2 D1 D0    minutes [0..59]
      //                I/O DATA 2   D7 D6 D5 D4 D3 D2 D1 D0    hours   [0..23]
      //                I/O DATA 3   D7 D6 D5 D4 D3 D2 D1 D0    day     [1..31]
      //                I/O DATA 4   D7 D6 D5 D4 D3 D2 D1 D0    month   [1..12]
      //                I/O DATA 5   D7 D6 D5 D4 D3 D2 D1 D0    year    [0..99]
      //                I/O DATA 6   D7 D6 D5 D4 D3 D2 D1 D0    tempC   [-128..127] (7th data byte)
      //
      // NOTE 1: If RTC is not found all read values wil be = 0
      // NOTE 2: Overread data (more then 7 bytes read) will be = 0
      // NOTE 3: The temperature (Celsius) is a byte with two complement binary format [-128..127]

      if (foundRTC)
      {
        if (ioByteCnt == 0) readRTC(&seconds, &minutes, &hours, &day, &month, &year, &tempC); // Read from RTC
        if (ioByteCnt < 7)
        // Send date/time (binary values) to Z80 bus
        {
          switch (ioByteCnt)
          {
            case 0: ioData = seconds; break;
            case 1: ioData = minutes; break;
            case 2: ioData = hours; break;
            case 3: ioData = day; break;
            case 4: ioData = month; break;
            case 5: ioData = year; break;
            case 6: ioData = tempC; break;
          }
          ioByteCnt++;
        }
        else ioOpcode = 0xFF;                   // All done. Set ioOpcode = "No operation"
      }
      else ioOpcode = 0xFF;                     // Nothing to do. Set ioOpcode = "No operation"
    break;

    case  0x85:
      // DISK EMULATION
      // ERRDISK - read the error code after a SELDISK, SELSECT, SELTRACK, WRITESECT, READSECT 
      //           or SDMOUNT operation
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    DISK error code (binary)
      //
      //
      // Error codes table:
      //
      //    error code    | description
      // ---------------------------------------------------------------------------------------------------
      //        0         |  No error
      //        1         |  DISK_ERR: the function failed due to a hard error in the disk function, 
      //                  |   a wrong FAT structure or an internal error
      //        2         |  NOT_READY: the storage device could not be initialized due to a hard error or 
      //                  |   no medium
      //        3         |  NO_FILE: could not find the file
      //        4         |  NOT_OPENED: the file has not been opened
      //        5         |  NOT_ENABLED: the volume has not been mounted
      //        6         |  NO_FILESYSTEM: there is no valid FAT partition on the drive
      //       16         |  Illegal disk number
      //       17         |  Illegal track number
      //       18         |  Illegal sector number
      //       19         |  Reached an unexpected EOF
      //
      //
      //
      //
      // NOTE 1: ERRDISK code is referred to the previous SELDISK, SELSECT, SELTRACK, WRITESECT or READSECT
      //         operation
      // NOTE 2: Error codes from 0 to 6 come from the PetitFS library implementation
      // NOTE 3: ERRDISK must not be used to read the resulting error code after a SDMOUNT operation 
      //         (see the SDMOUNT opcode)
             
      ioData = diskErr;
    break;

    case  0x86:
      // DISK EMULATION
      // READSECT - read 512 data bytes sequentially from the current emulated disk/track/sector:
      //
      //                 I/O DATA:   D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                 I/O DATA 0  D7 D6 D5 D4 D3 D2 D1 D0    First Data byte
      //
      //                      |               |
      //                      |               |
      //                      |               |                 <510 Data Bytes>
      //                      |               |
      //
      //               I/O DATA 127  D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    512th Data byte (Last byte)
      //
      //
      // Reads the current sector (512 bytes) of the current track/sector, one data byte each call. 
      // All the 512 calls must be always performed sequentially to have a READSECT operation correctly done. 
      // If an error occurs during the READSECT operation, all subsequent read data will be = 0.
      // If an error occurs calling any DISK EMULATION opcode (SDMOUNT excluded) immediately before the READSECT 
      //  opcode call, all the read data will be will be = 0 and the READSECT operation will not be performed.
      // Errors are stored into "diskErr" (see ERRDISK opcode).
      //
      // NOTE 1: Before a READSECT operation at least a SELTRACK or a SELSECT must be always performed
      // NOTE 2: Remember to open the right "disk file" at first using the SELDISK opcode

      if (!ioByteCnt)
      // First byte of 512, so set the right file pointer to the current emulated track/sector first
      {
        if ((trackSel < 512) && (sectSel < 32) && (!diskErr))
        // Sector and track numbers valid and no previous error; set the LBA-like logical sector
        {
          diskErr = seekSD((trackSel * 9) + sectSel);  // Set the starting point inside the "disk file"
                                                        //  generating a 14 bit "disk file" LBA-like 
                                                        //  logical sector address created as TTTTTTTTTSSSSS
        }
      }

            
      if (!diskErr)
      // No previous error (e.g. selecting disk, track or sector)
      {
        tempByte = ioByteCnt % 32;          // [0..31]
        if (!tempByte)
        // Read 32 bytes of the current sector on SD in the buffer (every 32 calls, starting with the first)
        {
          diskErr = readSD(bufferSD, &numReadBytes); 
          if (numReadBytes < 32) diskErr = 19;    // Reached an unexpected EOF
        }
        if (!diskErr) ioData = bufferSD[tempByte];// If no errors, exchange current data byte with the CPU
      }
      if (ioByteCnt >= 511) 
      {
        ioOpcode = 0xFF;                    // All done. Set ioOpcode = "No operation"
      }
      ioByteCnt++;                          // Increment the counter of the exchanged data bytes
    break;

    case  0x87:
      // DISK EMULATION
      // SDMOUNT - mount a volume on SD, returning an error code (binary):
      //
      //                 I/O DATA 0: D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    error code (binary)
      //
      //
      //
      // NOTE 1: This opcode is "normally" not used. Only needed if using a virtual disk from a custom program
      //         loaded with iLoad or with the Autoboot mode (e.g. ViDiT). Can be used to handle SD hot-swapping
      // NOTE 2: For error codes explanation see ERRDISK opcode
      // NOTE 3: Only for this disk opcode, the resulting error is read as a data byte without using the 
      //         ERRDISK opcode
      
      ioData = mountSD(&filesysSD);
      
    break;          

    case  0x88:
      // ATXBUFF - return the current available free space (in bytes) in the TX buffer:
      //
      //                I/O DATA:    D7 D6 D5 D4 D3 D2 D1 D0
      //                            ---------------------------------------------------------
      //                             D7 D6 D5 D4 D3 D2 D1 D0    free space in bytes (binary)
      //
      // NOTE: This opcode is intended to avoid delays in serial Tx operations, as the IOS hold the V20
      //       in a wait status if the TX buffer is full.
            
      ioData = Serial.availableForWrite() ;
    break;

  }
  if ((ioOpcode != 0x84) && (ioOpcode != 0x86)) ioOpcode = 0xFF;    // All done for the single byte opcodes. 
                                                                    //  Set ioOpcode = "No operation"
}
