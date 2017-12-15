//*****************************************************************************
//
// MSP432 main.c template - Empty main
//
//****************************************************************************

#include "msp.h"
#include "driverlib.h"
#include <gpio.h>
#include <debug.h>
#include <interrupt.h>
#include <hw_memmap.h>
#include <HAL_I2C.h>
#include <HAL_TMP006.h>
#include <grlib.h>
#include <HAL_OPT3001.h>
#include "Crystalfontz128x128_ST7735.h"
#include "HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

//Define the hw address range for Port Registers
#define PORT_BASE 0X40004c00 //MSP432 PAG. 101

//Define the hw address range base for UART Registers
#define USCI_A0BASE 0x40001000 //MSP432 PAG. 97

//Define the hw address range for Cortex-M4 Module to control NVIC
#define SYSTEM_CONTROL_SPACE_BASE 0xE000E000 //MSP432 PAG. 110

//Assign the offsets for each pin
unsigned int P1SEL0_OFF = 0X0A;
unsigned int P1SEL1_OFF = 0X0C;
unsigned int P1DIR_OFF = 0X04;

//Assign offsets Clock System Registers (Tech Ref. Pag. 320)
volatile unsigned int* CSKEY = (volatile unsigned int*) (CS_BASE + 0X00);
volatile unsigned int* CSCTL0 = (volatile unsigned int*) (CS_BASE + 0X04);
volatile unsigned int* CSCTL1 = (volatile unsigned int*) (CS_BASE + 0X08);

unsigned int DCORSEL_12MHZ = 0x3;

//Assign offsets UART Registers
volatile unsigned short int* UCA0_CTLW0 =
		(volatile unsigned short int*) (USCI_A0BASE); //Control Word 0
volatile unsigned short int* UCA0_BRW =
		(volatile unsigned short int*) (USCI_A0BASE + 0x06); //Baud Rate Control
volatile unsigned short int* UCA0_MCTLW =
		(volatile unsigned short int*) (USCI_A0BASE + 0X08); // Modulation Control
volatile unsigned short int * UCA0_TXBUF =
		(volatile unsigned short int *) ( USCI_A0BASE + 0x0E); //Transmit Buffer
volatile unsigned short int * UCA0_RXBUF =
		(volatile unsigned short int *) ( USCI_A0BASE + 0x0C); //Receive Buffer
volatile unsigned short int * UCA0_IE =
		(volatile unsigned short int *) ( USCI_A0BASE + 0x1A); //Interrup Enable
volatile unsigned short int * UCA0_IFG =
		(volatile unsigned short int *) ( USCI_A0BASE + 0x1C); //Interrupt Flag

//Assign offsets NVIC Registers
volatile unsigned int * NVIC_ISER0 =
		(volatile unsigned int *) ( SYSTEM_CONTROL_SPACE_BASE + 0x100);
volatile unsigned int * NVIC_ICPR0 =
		(volatile unsigned int *) ( SYSTEM_CONTROL_SPACE_BASE + 0x280);
volatile unsigned int * NVIC_IABR0 =
		(volatile unsigned int *) ( SYSTEM_CONTROL_SPACE_BASE + 0x300);

//Assign offsets Timer32 Registers
volatile unsigned int * T32LOAD1 = (volatile unsigned int *) ( TIMER32_BASE
		+ 0x0);
volatile unsigned int * T32CONTROL1 = (volatile unsigned int *) ( TIMER32_BASE
		+ 0x8);
volatile unsigned int * T32INTCLR1 = (volatile unsigned int *) ( TIMER32_BASE
		+ 0xC);

int g_Counter = 0;
int g_TimerExp = 0;
char str[10];
int air = 0;
int light = 1;
double confTime;
int counter = 0;
int firstTime = 0;
char tempArr[2];
double temp;
unsigned long int lum;
int j = 0;
int config;

char msgAirOn[] = "\n\nAr condicionado ligado\n";
char msgAirOff[] = "\nAr condicionado desligado\n";
char lampAcesa[] = "\nLampada acesa\n";
char lampApagada[] = "\nLampada apagada\n";
char msgArLigado[] = "Ar ligado";
char msgArDesligado[] = "Ar desligado";
char msgLuzAcesa[] = "Luz Acesa";
char msgLuzApagada[] = "Luz Apagada";

/* Graphic library context */
Graphics_Context g_sContext;

/*Configure Clock SMCLK  => Tech. Ref. Pag. 320*/
void configureClockSystem(uint32_t dcorsel) {

	//Write CSKEY = xxxx_695Ah to unlock CS registers
	*CSKEY = 0x695A;

	//Clean DCORSEL
	*CSCTL0 &= ~70000;

	//Select 12 MHz
	*CSCTL0 |= (dcorsel << 16);

}

/*UART0 Reset => Tech. Ref. Pag. 746*/
void resetUART() {
	//UCSWRST = 1
	*UCA0_CTLW0 |= 0x1;

}

/*Configure Baud Rate => Tech. Ref. Pag. 742*/
void configureBaudRateTo12MHz() {

	*UCA0_BRW = 39;

	*UCA0_MCTLW = 0x1 +  //UCOS16
			(1 << 4) +  //BRF
			(0 << 8);  //BRS

	//Set UCSSEL to 10b
	*UCA0_CTLW0 &= ~(0x3 << 6);
	*UCA0_CTLW0 |= (0x2 << 6);
}

/**/
void removeReset() {
	*UCA0_CTLW0 &= ~0x1;
}

int uart_puts(const char *str) {
	int status = -1;

	status = 0;

	while (*str != '\0') {
		/* Wait for the transmit buffer to be ready */
		while ((*UCA0_IFG & 0x2) == 0x0)
			;

		/* Transmit data */
		UCA0TXBUF = *str;

		/* If there is a line-feed, add a carriage return */
		if (*str == '\n') {
			/* Wait for the transmit buffer to be ready */
			while ((*UCA0_IFG & 0x2) == 0x0)
				;
			UCA0TXBUF = '\r';
		}

		str++;
	}

	return status;
}

void waitFor(unsigned int secs) {
	unsigned int retTime = time(0) + secs;   // Get finishing time.
	while (time(0) < retTime)
		;               // Loop until it arrives.
}

void generateReports(void) {
	uart_puts("\nRELATORIO DE TODOS OS EVENTOS REGISTRADOS:\n");
	uint8_t* addr_ptr = (uint8_t*) 0x20000;
	int i;
	for (i = 0; i < 127000; i++) {
		if (*(uint8_t*) (i + addr_ptr) == 'A') {
			uart_puts(lampAcesa);
		} else if (*(uint8_t*) (i + addr_ptr) == 'P') {
			uart_puts(lampApagada);
		} else if (*(uint8_t*) (i + addr_ptr) == 'L') {
			uart_puts(msgAirOn);
		} else if (*(uint8_t*) (i + addr_ptr) == 'D') {
			uart_puts(msgAirOff);
		}
	}
}

void saveEvent(void* value, int i) {
	//para clock de 12 MHz -> 0 wait states (manual MSP432P401R, seção 5.8)
	FlashCtl_setWaitState(FLASH_BANK0, 1);
	FlashCtl_setWaitState(FLASH_BANK1, 1);

	FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK0, FLASH_SECTOR4);
	FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR0);

	FlashCtl_enableWordProgramming(FLASH_IMMEDIATE_WRITE_MODE);
	FlashCtl_unprotectSector(FLASH_MAIN_MEMORY_SPACE_BANK1, FLASH_SECTOR0);

	uint8_t* addr_ptr = (uint8_t*) 0x20000;

	if (i <= 127000) {
		FlashCtl_programMemory(value, (void*) (i + addr_ptr), strlen(value));
	} else {
		Graphics_drawStringCentered(&g_sContext,
				(int8_t *) "Limite de memória atingido!", 12, 64, 100,
				OPAQUE_TEXT);
	}

	j = i + 1;
}

void startTimer() {
	*T32LOAD1 = 360000000;

	//prescaler em 16 -> registrador de controle
	//
	*T32CONTROL1 = (0x1 << 6) +	//periodic mode
			(0x1 << 5) + 	//interrupt enabled
			(0x0 << 2) + 	//prescaler em 1
			(0x1 << 1) + 	//32 bits
			(0x0 << 0);	//wrapping mode (reinicia quando chega a zero)

	//configura interrupção do timer
	//habilita interrupção do timer no controlador de interrupção
	//TIMER32_1 -> 25
	*NVIC_ISER0 |= 0x2000000;

	//habilita timer
	*T32CONTROL1 |= (0x1 << 7);

	char events[2] = "";

	while (1) {
		if (g_TimerExp == 1) {
			if (strlen(tempArr) != 0) {
				config = atoi(tempArr);
			}

			lum = OPT3001_getLux();
			temp = TMP006_getTemp();

			if (config != 0 && temp > config && air == 0) {
				events[0] = 'L';
				air = 1;
				uart_puts(msgAirOn);

				GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);

				Graphics_drawStringCentered(&g_sContext, (int8_t *) msgArLigado,
						12, 64, 100, OPAQUE_TEXT);
			} else if (config != 0 && temp < config && air == 1) {
				events[0] = 'D';
				air = 0;
				uart_puts(msgAirOff);

				GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

				Graphics_drawStringCentered(&g_sContext,
						(int8_t *) msgArDesligado, 12, 64, 100, OPAQUE_TEXT);

			}

			if (lum <= 50 && light == 1) {
				if (events[0] == 0) {
					events[0] = 'P';
				} else {
					events[1] = 'P';
				}

				light = 0;
				uart_puts(lampApagada);

				if (air == 1) {
					GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
				}

				GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
				waitFor(5);
				GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);

				if (air == 1) {
					GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
				}

				Graphics_drawStringCentered(&g_sContext,
						(int8_t *) msgLuzApagada, 12, 64, 100, OPAQUE_TEXT);

			} else if (lum > 50 && light == 0) {
				if (events[0] == 0) {
					events[0] = 'A';
				} else {
					events[1] = 'A';
				}
				light = 1;
				uart_puts(lampAcesa);

				if (air == 1) {
					GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
				}

				GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
				waitFor(5);
				GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);

				if (air == 1) {
					GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
				}

				Graphics_drawStringCentered(&g_sContext, (int8_t *) msgLuzAcesa,
						12, 64, 100, OPAQUE_TEXT);

			}

			if (events[0] > 0 || events[1] > 0) {
				saveEvent(events, j);

				if (events[0] > 0 && events[1] > 0) {
					j++;
				}

				events[0] = 0;
				events[1] = 0;
			}

			g_Counter++;
			g_TimerExp = 0; // flag reset
		}
	}
}

void main(void) {

	GPIO_setAsOutputPin(
	GPIO_PORT_P2,
	GPIO_PIN1 | GPIO_PIN2);

	GPIO_setOutputLowOnPin(
	GPIO_PORT_P2,
	GPIO_PIN1 | GPIO_PIN2);

	WDTCTL = WDTPW | WDTHOLD;           // Stop watchdog timer

	//Set the P1.2 and P1.3 as primary function for UART => RX/TX (Tech Ref. Pag. 727)
	*((volatile unsigned char*) (PORT_BASE + P1DIR_OFF)) &= ~0x0E;
	*((volatile unsigned char*) (PORT_BASE + P1SEL0_OFF)) |= 0x0E;
	*((volatile unsigned char*) (PORT_BASE + P1SEL1_OFF)) &= ~0x0E;

	//Set the frequency to 12MHz
	configureClockSystem(DCORSEL_12MHZ);

	//UART0 Reset
	resetUART();

	//Configure Baud Rate to 12MHz
	configureBaudRateTo12MHz();

	removeReset();

	//Enable interruption UART Rx
	*UCA0_IE = 0x1;

	//Enable UART interruption in the controller NVIC
	*NVIC_ISER0 |= 0x10000;

	/* Initializes display */
	Crystalfontz128x128_Init();

	/* Set default screen orientation */
	Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);

	/* Initializes graphics context */
	Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);
	Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
	Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
	GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
	Graphics_clearDisplay(&g_sContext);
	Graphics_drawStringCentered(&g_sContext, "Environment Monitor:",
	AUTO_STRING_LENGTH, 64, 50,
	OPAQUE_TEXT);

	/* Configures P2.6 to PM_TA0.3 for using Timer PWM to control LCD backlight */
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN6,
	GPIO_PRIMARY_MODULE_FUNCTION);

	uart_puts(" \nConfigure a temperatura maxima do ambiente.\n");

	//Enable primary function P6.4 and P6.5 via header to the signals SDA and SCL of the BoosterPAck
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN4,
	GPIO_PRIMARY_MODULE_FUNCTION);
	GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN5,
	GPIO_PRIMARY_MODULE_FUNCTION);

	//Initialization I2C => SMCLK of 12MHz
	eUSCI_I2C_MasterConfig i2c_config;
	i2c_config.selectClockSource = EUSCI_B_I2C_CLOCKSOURCE_SMCLK;
	i2c_config.i2cClk = 12000000;
	i2c_config.dataRate = EUSCI_B_I2C_SET_DATA_RATE_100KBPS;
	i2c_config.byteCounterThreshold = 0;
	i2c_config.autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP;

	I2C_disableModule(EUSCI_B1_BASE);
	I2C_initMaster(EUSCI_B1_BASE, &i2c_config);
	I2C_enableModule(EUSCI_B1_BASE);

	OPT3001_init();
	TMP006_init();

	startTimer();

}

void EUSCIA0_IRQHandler(void) {
	//Get NVIC interrupt status and clear
	*NVIC_ICPR0 = *NVIC_IABR0;
	char data;

	//If the buffer of Rx has data do
	if (*UCA0_IFG & 0x1 == 0x1) {
		data = *UCA0_RXBUF;

		if (data != 'r') {
			*UCA0_TXBUF = data;

			if (firstTime == 0) {
				tempArr[0] = data;
				firstTime = 1;
			} else {
				tempArr[1] = data;
				firstTime = 0;
			}
		}
		if (data == 'r') {
			generateReports();
		}
	}
}

void T32_INT1_IRQHandler(void) {
	//limpa irq do timer no controlador do timer
	*T32INTCLR1 = 0x0;		//qualquer coisa

	//Não é conveniente enviar o contador via UART aqui
	//dentro, pois isso depende de:
	// 1) transformar o contador int em uma string
	// 2) se a string tiver mais do que 1 byte, enviar
	// cada um dos bytes em loop
	//Essas operações (particularmente a 2)) tomam muito tempo,
	//e o handler de IRQ deveria ser o mais rápido possível.

	//avisa main para enviar contador pela serial e incrementar
	g_TimerExp = 1;

}

