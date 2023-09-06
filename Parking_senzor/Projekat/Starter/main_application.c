// STANDARD INCLUDES
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// KERNEL INCLUDES
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "timers.h"
#include "extint.h"
#include "main_application.h"

// HARDWARE SIMULATOR UTILITY FUNCTIONS  
#include "HW_access.h"

//Handles declaration
static SemaphoreHandle_t led_BS, obrada_BS1, obrada_BS2, obrada_BS3, diode_BS;
static SemaphoreHandle_t TBE_BS_0, TBE_BS_1, TBE_BS_2;
static SemaphoreHandle_t RXC_BS_0, RXC_BS_1, RXC_BS_2;
static SemaphoreHandle_t  bin_BS_on, bin_BS_off;
static TimerHandle_t per_TimerHandle1, per_TimerHandle2, per_TimerHandle_senzori, per_TimerHandle_diode;
static TaskHandle_t ledBar, serial0, serial1, serial2;
static QueueHandle_t que_S1, que_S2, que_mm1, que_mm2, red_procenti1, red_procenti2, serial_que;
static QueueHandle_t kalibracija1, kalibracija_duzina1, kalibracija2, kalibracija_duzina2;

// INTERRUPTS //
static uint32_t OnLED_ChangeInterrupt(void) {	// OPC - ON INPUT CHANGE - INTERRUPT HANDLER 

	BaseType_t xHigherPTW = pdFALSE;

	if (xSemaphoreGiveFromISR(led_BS, &xHigherPTW) != pdTRUE) {
		printf("Nije predat led interapt\n");
	}

	portYIELD_FROM_ISR((uint32_t)xHigherPTW);
}

static uint32_t prvProcessTBEInterrupt(void) {	// TBE - TRANSMISSION BUFFER EMPTY - INTERRUPT HANDLER 

	BaseType_t xHigherPTW = pdFALSE;

	if (get_TBE_status(0) != 0) {
		if (xSemaphoreGiveFromISR(TBE_BS_0, &xHigherPTW) != pdFALSE)
		{
			printf("potvrda\n");
		}
	}

	if (get_TBE_status(1) != 0) {
		if (xSemaphoreGiveFromISR(TBE_BS_1, &xHigherPTW) != pdFALSE)
		{
			printf("potvrda\n");
		}
	}
	if (get_TBE_status(2) != 0) {
		if (xSemaphoreGiveFromISR(TBE_BS_2, &xHigherPTW) != pdFALSE)
		{
			printf("potvrda\n");
		}
	}
	
	portYIELD_FROM_ISR((uint32_t)xHigherPTW);
}

static uint32_t prvProcessRXCInterrupt(void) {	// RXC - RECEPTION COMPLETE - INTERRUPT HANDLER 

	BaseType_t xHigherPTW = pdFALSE;

	if (get_RXC_status(0) != 0) {
		if (xSemaphoreGiveFromISR(RXC_BS_0, &xHigherPTW) != pdTRUE) {
			printf("get_rxc_status od 0\n");
		}
	}

	if (get_RXC_status(1) != 0) {
		if (xSemaphoreGiveFromISR(RXC_BS_1, &xHigherPTW) != pdTRUE) {
			printf("get_rxc_status od 1\n");
		}
	}

	if (get_RXC_status(2) != 0) {
		if (xSemaphoreGiveFromISR(RXC_BS_2, &xHigherPTW) != pdTRUE) {
			printf("get_rxc_status od 2\n");
		}
	}

	portYIELD_FROM_ISR((uint32_t)xHigherPTW);

}

/*TIMERS*/
static void TimerCallback_obrada1(TimerHandle_t xTimer)
{
	
	if ((xSemaphoreGive(obrada_BS1)) == pdTRUE) {
		printf("Predat obrada_BS1\n");
	}

}

static void TimerCallback_obrada2(TimerHandle_t xTimer)
{
	if ((xSemaphoreGive(obrada_BS2)) == pdTRUE) {
		printf("Predat led obrada_BS2\n");
	}
}


static void TimerCallback_obrada_senzori(TimerHandle_t xTimer)
{
	if ((xSemaphoreGive(obrada_BS3)) == pdTRUE) {
		printf("Predat led obrada_BS3\n");
	}
}

static void TimerCallback_diode(TimerHandle_t xTimer)
{
	if ((xSemaphoreGive(diode_BS)) == pdTRUE) {
		printf("Predat led obrada_BS1\n");
	}
	
}

// MAIN - SYSTEM STARTUP POINT 
void main_demo(void) {

	if (init_serial_uplink(COM_CH_0) != 0) {
		printf("Neuspesna inicijalizacija kanala 1\n"); // inicijalizacija serijske na kanalu 0
	}
	if (init_serial_downlink(COM_CH_0) != 0) {
		printf("Neuspesna inicijalizacija kanala 0\n");
	}
	if (init_serial_uplink(COM_CH_1) != 0) {
		printf("Neuspesna inicijalizacija kanala \n"); // inicijalizacija serijske na kanalu 1
	}
	if (init_serial_downlink(COM_CH_1) != 0) {
		printf("Neuspesna inicijalizacija kanala 1\n");
	}
	if (init_serial_uplink(COM_CH_2) != 0) {
		printf("Neuspesna inicijalizacija kanala 2\n"); // inicijalizacija serijske na kanalu 2
	}
	if (init_serial_downlink(COM_CH_2) != 0) {
		printf("Neuspesna inicijalizacija kanala 2\n");
	}
	if (init_LED_comm() != 0) {							// inicijalizacija ledbara -a 
		printf("Neuspesna inicijalizacija kanala ledbar-a\n");
	}


	// INTERRUPT HANDLERS
	vPortSetInterruptHandler(portINTERRUPT_SRL_TBE, prvProcessTBEInterrupt);	// SERIAL TRANSMITT INTERRUPT HANDLER 
	vPortSetInterruptHandler(portINTERRUPT_SRL_RXC, prvProcessRXCInterrupt);	// SERIAL RECEPTION INTERRUPT HANDLER 
	vPortSetInterruptHandler(portINTERRUPT_SRL_OIC, OnLED_ChangeInterrupt);		// ON INPUT CHANGE INTERRUPT HANDLER 

	// BINARY SEMAPHORES
	//LED_INT_BinarySemaphore = xSemaphoreCreateBinary();	// CREATE LED INTERRUPT SEMAPHORE 
	TBE_BS_0 = xSemaphoreCreateBinary();		// CREATE TBE SEMAPHORE - SERIAL TRANSMIT COMM 
	RXC_BS_0 = xSemaphoreCreateBinary();		// CREATE RXC SEMAPHORE - SERIAL RECEIVE COMM
	TBE_BS_1 = xSemaphoreCreateBinary();		// CREATE TBE SEMAPHORE - SERIAL TRANSMIT COMM 
	RXC_BS_1 = xSemaphoreCreateBinary();		// CREATE RXC SEMAPHORE - SERIAL RECEIVE COMM
	TBE_BS_2 = xSemaphoreCreateBinary();		// CREATE TBE SEMAPHORE - SERIAL TRANSMIT COMM 
	RXC_BS_2 = xSemaphoreCreateBinary();		// CREATE RXC SEMAPHORE - SERIAL RECEIVE COMM

	obrada_BS1 = xSemaphoreCreateBinary();		//SEMAFORI ZA OBRADU PODATAKA 1, 2, 3
	obrada_BS2 = xSemaphoreCreateBinary();
	obrada_BS3 = xSemaphoreCreateBinary();
	bin_BS_on = xSemaphoreCreateBinary();       // SEMAFOR ZA UKLJUCIVANJE KOMUNIKACIJE
	bin_BS_off = xSemaphoreCreateBinary();		// SEMAFOR ZA GASENJE KOMUNIKACIJE
	diode_BS = xSemaphoreCreateBinary();		// SEMAFOR ZA UKLJUCIVANJE TRECEG STUPCA DIODA
	led_BS = xSemaphoreCreateBinary();			// SEMAFOR ZA PREKIDAC

	/*QUEUES*/

	kalibracija1 = xQueueCreate(3, sizeof(uint8_t[26])); //red za skladistenje primljene rijeci
	if (kalibracija1 == NULL) {
		printf("Greska pri kreiranju reda kalibracija1\n");
	}
	kalibracija_duzina1 = xQueueCreate(3, sizeof(uint8_t)); //red za skladistenje duzine primljene rijeci
	if (kalibracija_duzina1 == NULL) {
		printf("Greska pri kreiranju reda kalibracija_duzina1\n");
	}
	kalibracija2 = xQueueCreate(3, sizeof(uint8_t[26])); //red za skladistenje primljene rijeci
	if (kalibracija2 == NULL) {
		printf("Greska pri kreiranju reda kalibracija2\n");
	}
	kalibracija_duzina2 = xQueueCreate(3, sizeof(uint8_t)); //red za skladistenje duzine primljene rijeci
	if (kalibracija_duzina2 == NULL) {
		printf("Greska pri kreiranju reda kalibracija_duzina2\n");
	}
	que_S1 = xQueueCreate(10, sizeof(uint8_t[3]));// xQueueCreate(3, sizeof(uint32_t));
	if (que_S1 == NULL) {
		printf("Greska pri kreiranju reda que_S1\n");
	}
	que_S2 = xQueueCreate(10, sizeof(uint8_t[3])); //xQueueCreate(3, sizeof(uint32_t));
	if (que_S2 == NULL) {
		printf("Greška pri kreiranju reda que_S2\n");
	}
	que_mm1 = xQueueCreate(5, sizeof(uint8_t[4])); // Kreiraj red za mm1
	if (que_mm1 == NULL) {
		printf("Greska pri kreiranju reda que_S1\n");
	}
	que_mm2 = xQueueCreate(5, sizeof(uint8_t[4])); // Kreiraj red za mm1
	if (que_mm2 == NULL) {
		printf("Greska pri kreiranju reda que_S2\n");
	}
	red_procenti1 = xQueueCreate(3, sizeof(uint8_t)); //red za skladistenje duzine primljene rijeci
	if (kalibracija_duzina1 == NULL) {
		printf("Greska pri kreiranju reda red_procenti1\n");
	}
	red_procenti2 = xQueueCreate(3, sizeof(uint8_t)); //red za skladistenje duzine primljene rijeci
	if (kalibracija_duzina1 == NULL) {
		printf("Greska pri kreiranju reda red_procenti2\n");
	}
	serial_que = xQueueCreate(4, sizeof(char[20])); // red za skladistenje poruke 
	if (serial_que == NULL) {
		printf("Greska pri kreiranju reda serial_que\n");
	}


	/* TIMER*/ 
	per_TimerHandle1 = xTimerCreate("Timer za obradu1", pdMS_TO_TICKS(200), pdTRUE, NULL, TimerCallback_obrada1); //callback 200ms
	if (per_TimerHandle1 == NULL) {
		printf("Greška pri kreiranju timera\n");
	}
	if (xTimerStart(per_TimerHandle1, 0) == pdFAIL) {
		printf("Neuspjesan start tajmera.\n");
	}
	per_TimerHandle2 = xTimerCreate("Timer za obradu2", pdMS_TO_TICKS(200), pdTRUE, NULL, TimerCallback_obrada2); //callback 200ms
	if (per_TimerHandle2 == NULL) {
		printf("Greška pri kreiranju timera\n");
	}
	if (xTimerStart(per_TimerHandle2, 0) == pdFAIL) {
		printf("Neuspjesan start tajmera.\n");
	}

	per_TimerHandle_senzori = xTimerCreate("Timer za obradu senzora", pdMS_TO_TICKS(500), pdTRUE, NULL, TimerCallback_obrada_senzori); //callback 200ms
	if (per_TimerHandle_senzori == NULL) {
		printf("Greška pri kreiranju timera\n");
	}
	if (xTimerStart(per_TimerHandle_senzori, 0) == pdFAIL) {
		printf("Neuspjesan start tajmera.\n");
	}
	per_TimerHandle_diode = xTimerCreate("Timer za treci stubac led", pdMS_TO_TICKS(500), pdTRUE, NULL, TimerCallback_diode); //callback 200ms
	if (per_TimerHandle_diode == NULL) {
		printf("Greška pri kreiranju timera\n");
	}
	if (xTimerStart(per_TimerHandle_diode, 0) == pdFAIL) {
		printf("Neuspjesan start tajmera.\n");
	}


	// TASKS 
	BaseType_t status;

	
	status = xTaskCreate(Serial0Send_Task, "ST0", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAL_SEND_PRI, &serial0);	// SERIAL TRANSMITTER TASK 
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(Serial1Send_Task, "ST1", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAL_SEND_PRI, &serial1);	// SERIAL TRANSMITTER TASK 
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}
	status = xTaskCreate(Serial2Send_Task, "ST2", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAL_SEND_PRI, &serial2);	// SERIAL TRANSMITTER TASK 
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}
	status = xTaskCreate(RXC_isr_0, "SR0", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAL_REC_PRI, NULL);	// SERIAL RECEIVER TASK
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(RXC_isr_1, "SR1", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAL_REC_PRI, NULL);	// SERIAL RECEIVER TASK 
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(PC_Receive_task, "pc rec", configMINIMAL_STACK_SIZE, NULL, TASK_SERIAL_REC_PRI, NULL);	// SERIAL RECEIVER TASK 
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(task_ukljuci_iskljuci, "on/off task", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);	//ON OFF TASK
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}	

	status = xTaskCreate(LEDBar_Task, "led bar task", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);	//LED BAR TASK ZA PREKIDAC
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(task_obrada_kalibracija1, "task za obradu1", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);	// TASK ZA OBRADU 1
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(task_obrada_kalibracija2, "task za obradu2", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);	// TASK ZA OBRADU 2
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	status = xTaskCreate(task_obrada_senzori, "task za obradu senzora", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, NULL);	// TASK ZA OBRADU SENZORA
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}
	status = xTaskCreate(diode_on, "task za ukljucivanje treceg stupca", configMINIMAL_STACK_SIZE, NULL, SERVICE_TASK_PRI, &ledBar);	// TASK ZA UKLJUCIVANJE TRECEG STUPCA DIODA
	if (status != pdPASS) {
		printf("Greska prilikom kreiranja\n");
	}

	// START SCHEDULER
	vTaskStartScheduler();
	for (;;) {}
}


void Serial0Send_Task(void* pvParameters)
{
	uint8_t brojac = 0;

	unsigned char trigger1[] = "S1\n";
	int niz_len = 3;

	for (;;)
	{
		for (size_t i = 0; i < sizeof(trigger1) / sizeof(unsigned char); i++)
		{
			if (send_serial_character(COM_CH_0, trigger1[i]) != pdFALSE) {
				printf("Greska\n");
			}

			vTaskDelay(pdMS_TO_TICKS(100));
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void Serial1Send_Task(void* pvParameters)
{
	uint8_t brojac = 0;

	unsigned char trigger1[] = "S2\n";
	int niz_len = 3;

	for (;;)
	{
		for (size_t i = 0; i < sizeof(trigger1) / sizeof(unsigned char); i++)
		{
			if (send_serial_character(COM_CH_1, trigger1[i]) != pdFALSE) {
				printf("Greska\n");
			}

			vTaskDelay(pdMS_TO_TICKS(100));
		}

		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void PC_Receive_task(void* pvParameters) {

	uint8_t	i = 0;
	uint8_t rec_buffer[7];
	uint8_t niz[7] = "REVERSE";
	uint8_t niz1[5] = "PARK";
	uint8_t niz2[5] = "DRIVE";
	uint8_t niz3[7] = "NEUTRAL";

	for (;;) {

		if (xSemaphoreTake(RXC_BS_2, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}
		if (get_serial_character(2, &rec_buffer[i]) != pdFALSE) {
			printf("Greska\n");
		}

		i++;

		if (i == (uint8_t)4) {
			if ((rec_buffer[0] == niz1[0]) && (rec_buffer[1] == niz1[1]) && (rec_buffer[2] == niz1[2]) && (rec_buffer[3] == niz1[3])) {
				printf("PARK \n");
				if (xSemaphoreGive(bin_BS_off) == pdFALSE) {
					printf("Greska\n");
				}
				i = (uint8_t)0;
			}
		}
		if (i == (uint8_t)5) {
			if ((rec_buffer[0] == niz2[0]) && (rec_buffer[1] == niz2[1]) && (rec_buffer[2] == niz2[2]) && (rec_buffer[3] == niz2[3]) && (rec_buffer[4] == niz2[4])) {
				printf("DRIVE \n");
				if (xSemaphoreGive(bin_BS_off) == pdFALSE) {
					printf("Greska\n");
				}
				i = (uint8_t)0;
			}
		}
		if (i == (uint8_t)7) {
			if ((rec_buffer[0] == niz[0]) && (rec_buffer[1] == niz[1]) && (rec_buffer[2] == niz[2]) && (rec_buffer[3] == niz[3]) && (rec_buffer[4] == niz[4]) && (rec_buffer[5] == niz[5]) && (rec_buffer[6] == niz[6])) {
				printf("REVERSE \n");
				if (xSemaphoreGive(bin_BS_on) == pdFALSE) {
					printf("Greska\n");
				}
				i = (uint8_t)0;
			}
			else if ((rec_buffer[0] == niz3[0]) && (rec_buffer[1] == niz3[1]) && (rec_buffer[2] == niz3[2]) && (rec_buffer[3] == niz3[3]) && (rec_buffer[4] == niz3[4]) && (rec_buffer[5] == niz3[5]) && (rec_buffer[6] == niz3[6])) {
				printf("NEUTRAL \n");
				if (xSemaphoreGive(bin_BS_off) == pdFALSE) {
					printf("Greska\n");
				}
				i = (uint8_t)0;
			}
			else {
				printf("Pogresna komanda\n");
				i = (uint8_t)0;
			}
		}

	}
}

void task_ukljuci_iskljuci(void* pvParameters)
{
	for (;;)
	{	
		if ((xSemaphoreTake(bin_BS_off, portMAX_DELAY) == pdTRUE)) {

			if (set_LED_BAR(1, 0x00) != 0) {
				printf("Greska\n");
			}
			vTaskSuspend(serial1);
			vTaskSuspend(serial0);
			vTaskSuspend(ledBar);
			vTaskSuspend(serial2);
		}
		if ( (xSemaphoreTake(bin_BS_on, portMAX_DELAY) == pdTRUE)) {

			if (set_LED_BAR(1, 0x01) != 0) {
				printf("Greska\n");
			}
			vTaskResume(serial1);
			vTaskResume(serial0);
			vTaskResume(ledBar);
			vTaskResume(serial2);
		}
	}
}

void LEDBar_Task(void* pvParameters) {

	uint8_t d = 0;;

	for (;;) {

		if (xSemaphoreTake(led_BS, portMAX_DELAY) != pdTRUE) {
			printf("Greska kod tastera\n");
		}
		if (get_LED_BAR(0, &d) != 0) {
			printf("Greska\n");
		}
		if (d == (uint8_t)0x01) {
			printf("UPALJEN\n");
			if (xSemaphoreGive(bin_BS_on) == pdFALSE) {
				printf("Greska\n");
			}
		}
	    if (d == (uint8_t)0x00) {
			printf("UGASEN\n");
			if (xSemaphoreGive(bin_BS_off) == pdFALSE) {
				printf("Greska\n");
			}
		}
	}
} 

void RXC_isr_0(void* pvParameters) {

	uint8_t cc = 0;
	uint8_t r_buffer[25] = { 0 };
	uint8_t duzina_rijeci = 0;
	uint8_t r_point = 0;
	BaseType_t queueSendResult;

	for (;;) {
		if (xSemaphoreTake(RXC_BS_0, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}
		if (get_serial_character(COM_CH_0, &cc) != pdFALSE) {
			printf("Greska\n");
		}

		if (cc == (uint8_t)0xff) {   // fF oznacava POCETAK poruke
			r_point = (uint8_t)0;
		}
		else if (cc == (uint8_t)0x0d) { // 0d za svaki KRAJ poruke
			if (r_point > (uint8_t)0) {
				queueSendResult = xQueueSend(que_S1, &r_buffer, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_S1\n");
				}
			}
		}
		else if (cc == (uint8_t)75) {
			r_point = (uint8_t)0;
		}
		else if (cc == (uint8_t)10) {
			duzina_rijeci = r_point;
			queueSendResult = xQueueSend(kalibracija1, r_buffer, portMAX_DELAY);
			if (queueSendResult != pdPASS) {
				printf("Greska pri slanju u red kalibracija1\n");
			}
			queueSendResult = xQueueSend(kalibracija_duzina1, &duzina_rijeci, portMAX_DELAY);
			if (queueSendResult != pdPASS) {
				printf("Greska pri slanju u red kalibracija_duzina1\n");
			}
		}
		else if (r_point < (uint8_t)25) {
			r_buffer[r_point++] = cc;
			//printf("KANAL 0: primio karakter: %u\n", cc);
		}
		else {
			//Ovaj else blok hvata sve ostale slu?ajeve
			printf("Neprepoznat karakter: %u\n", cc);
		}
	}
}

void RXC_isr_1(void* pvParameters) {

	uint8_t cc = 0;
	uint8_t r_buffer[25] = { 0 };
	uint8_t duzina_rijeci = 0;
	uint8_t r_point = 0;
	BaseType_t queueSendResult;

	for (;;) {
		if (xSemaphoreTake(RXC_BS_1, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}
		if (get_serial_character(COM_CH_1, &cc) != pdFALSE) {
			printf("Greska\n");
		}

		if (cc == (uint8_t)0xff) {   // fF oznacava POCETAK poruke
			r_point = (uint8_t)0;
		}
		else if (cc == (uint8_t)0x0d) { // 0d za svaki KRAJ poruke
			if (r_point > (uint8_t)0) {
				queueSendResult = xQueueSend(que_S2, &r_buffer, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_S2\n");
				}
			}
		}
		else if (cc == (uint8_t)75) {
			r_point = (uint8_t)0;
		}
		else if (cc == (uint8_t)10) {
			duzina_rijeci = r_point;
			queueSendResult = xQueueSend(kalibracija2, r_buffer, portMAX_DELAY);
			if (queueSendResult != pdPASS) {
				printf("Greska pri slanju u red kalibracija2\n");
			}
			queueSendResult = xQueueSend(kalibracija_duzina2, &duzina_rijeci, portMAX_DELAY);
			if (queueSendResult != pdPASS) {
				printf("Greska pri slanju u red kalibracija_duzina2\n");
			}
		}
		else if (r_point < (uint8_t)25) {
			r_buffer[r_point++] = cc;
			//printf("KANAL 1: primio karakter: %u\n", cc);
		}
		else {
			//Ovaj else blok hvata sve ostale slu?ajeve
			printf("Neprepoznat karakter: %u\n", cc);
		}
	}
}

void task_obrada_kalibracija1(void* pvParameters) {

	uint8_t duzina1 = 0;
	uint8_t kal1[26] = { 0 };
	uint8_t niz[] = { 48, 49, 50, 51, 52, 53, 54, 55, 56, 57 };

	BaseType_t queueReceiveResult;

	for (;;) {

		if (xSemaphoreTake(obrada_BS1, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		//for (uint8_t set1Count = 0; set1Count < 2; set1Count++) {

		queueReceiveResult = xQueueReceive(kalibracija_duzina1, &duzina1, portMAX_DELAY); // za prijem duzine rijeci kalibracije na kanalu 0
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red kalibracija_duzina1\n");
		}
		queueReceiveResult = xQueueReceive(kalibracija1, &kal1, portMAX_DELAY); // za prijem kalibracije na kanalu 0
		printf("\n");
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red kalibracija1\n");
		}


		for (uint8_t i = 0; i < duzina1; i++) {
			if ((kal1[i] == (uint8_t)95) && (i == (uint8_t)10) && (duzina1 < (uint8_t)21)) {//nalazi milimetre kalibracije 1
				K1_1[0] = kal1[i + (uint8_t)1];
				if (kal1[i + (uint8_t)2] != (uint8_t)109) {
					K1_1[1] = kal1[i + (uint8_t)2];
				}
				if (kal1[i + (uint8_t)3] != (uint8_t)109) {
					K1_1[2] = kal1[i + (uint8_t)3];
				}
			}
			if ((kal1[i] == (uint8_t)95) && (i > (uint8_t)11) && (duzina1 < (uint8_t)21)) { //nalazi procente kalibracije 1
				procenat1_1[0] = kal1[i + (uint8_t)1];
				if (kal1[i + (uint8_t)2] != (uint8_t)37) {
					procenat1_1[1] = kal1[i + (uint8_t)2];
				}
			}
			if ((kal1[i] == (uint8_t)95) && (i == (uint8_t)10) && (duzina1 >= (uint8_t)21)) {//nalazi milimetre kalibracije 2
				K1_2[0] = kal1[i + (uint8_t)1];
				if (kal1[i + (uint8_t)2] != (uint8_t)109) {
					K1_2[1] = kal1[i + (uint8_t)2];
				}
				if (kal1[i + (uint8_t)3] != (uint8_t)109) {
					K1_2[2] = kal1[i + (uint8_t)3];
				}
				if (kal1[i + (uint8_t)4] != (uint8_t)109) {
					K1_2[3] = kal1[i + (uint8_t)4];
				}
			}
			if ((kal1[i] == (uint8_t)95) && (i > (uint8_t)11) && (duzina1 >= (uint8_t)21)) { //nalazi procente kalibracije 2
				procenat1_2[0] = kal1[i + (uint8_t)1];
				if (kal1[i + (uint8_t)2] != (uint8_t)37) {
					procenat1_2[1] = kal1[i + (uint8_t)2];
				}
				if (kal1[i + (uint8_t)3] != (uint8_t)37) {
					procenat1_2[2] = kal1[i + (uint8_t)3];
				}
			}

		}
		for (uint8_t i = 0; i < (uint8_t)10; i++) { //KONVERZIJE IZ HEX U DEC
			if (K1_1[0] == niz[i]) {
				K1_1[0] = i;
			}
			if (K1_1[1] == niz[i]) {
				K1_1[1] = i;
			}
			if (K1_1[2] == niz[i]) {
				K1_1[2] = i;
			}
			if (K1_2[0] == niz[i]) {
				K1_2[0] = i;
			}
			if (K1_2[1] == niz[i]) {
				K1_2[1] = i;
			}
			if (K1_2[2] == niz[i]) {
				K1_2[2] = i;
			}
			if (K1_2[3] == niz[i]) {
				K1_2[3] = i;
			}
			if (procenat1_1[0] == niz[i]) {
				procenat1_1[0] = i;
			}
			if (procenat1_1[1] == niz[i]) {
				procenat1_1[1] = i;
			}
			if (procenat1_2[0] == niz[i]) {
				procenat1_2[0] = i;
			}
			if (procenat1_2[1] == niz[i]) {
				procenat1_2[1] = i;
			}
			if (procenat1_2[2] == niz[i]) {
				procenat1_2[2] = i;
			}


		}
	}
}

void task_obrada_kalibracija2(void* pvParameters) {

	uint8_t duzina2 = 0;
	uint8_t kal2[26] = { 0 };
	uint8_t niz[] = { 48, 49, 50, 51, 52, 53, 54, 55, 56, 57 };

	uint8_t length = 0;
	BaseType_t queueReceiveResult;

	for (;;) {

		if (xSemaphoreTake(obrada_BS2, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		queueReceiveResult = xQueueReceive(kalibracija_duzina2, &duzina2, portMAX_DELAY); //isto kao za kanal 0 ali ovaj put za kanal 1
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red kalibracija_duzina2\n");
		}
		queueReceiveResult = xQueueReceive(kalibracija2, &kal2, portMAX_DELAY);
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red kalibracija2\n");
		}

		for (uint8_t i = 0; i < duzina2; i++) {
			if ((kal2[i] == (uint8_t)95) && (i == (uint8_t)10) && (duzina2 < (uint8_t)21)) {//nalazi milimetre kalibracije 1
				K2_1[0] = kal2[i + (uint8_t)1];
				if (kal2[i + (uint8_t)2] != (uint8_t)109) {
					K2_1[1] = kal2[i + (uint8_t)2];
				}
				if (kal2[i + (uint8_t)3] != (uint8_t)109) {
					K2_1[2] = kal2[i + (uint8_t)3];
				}
			}
			if ((kal2[i] == (uint8_t)95) && (i > (uint8_t)10) && (duzina2 < (uint8_t)21)) { //nalazi procente kalibracije 1
				procenat2_1[0] = kal2[i + (uint8_t)1];
				if (kal2[i + (uint8_t)2] != (uint8_t)37) {
					procenat2_1[1] = kal2[i + (uint8_t)2];
				}
			}
			if ((kal2[i] == (uint8_t)95) && (i == (uint8_t)10) && (duzina2 >= (uint8_t)21)) {//nalazi milimetre kalibracije 2
				K2_2[0] = kal2[i + (uint8_t)1];
				if (kal2[i + (uint8_t)2] != (uint8_t)109) {
					K2_2[1] = kal2[i + (uint8_t)2];
				}
				if (kal2[i + (uint8_t)3] != (uint8_t)109) {
					K2_2[2] = kal2[i + (uint8_t)3];
				}
				if (kal2[i + (uint8_t)4] != (uint8_t)109) {
					K2_2[3] = kal2[i + (uint8_t)4];
				}
			}
			if ((kal2[i] == (uint8_t)95) && (i > (uint8_t)10) && (duzina2 >= (uint8_t)21)) { //nalazi procente kalibracije 2
				procenat2_2[0] = kal2[i + (uint8_t)1];
				if (kal2[i + (uint8_t)2] != (uint8_t)37) {
					procenat2_2[1] = kal2[i + (uint8_t)2];
				}
				if (kal2[i + (uint8_t)3] != (uint8_t)37) {
					procenat2_2[2] = kal2[i + (uint8_t)3];
				}
			}
		}
		for (uint8_t i = 0; i < (uint8_t)10; i++) { //KONVERZIJE IZ HEX U DEC
			if (K2_1[0] == niz[i]) {
				K2_1[0] = i;
			}
			if (K2_1[1] == niz[i]) {
				K2_1[1] = i;
			}
			if (K2_1[2] == niz[i]) {
				K2_1[2] = i;
			}
			if (K2_2[0] == niz[i]) {
				K2_2[0] = i;
			}
			if (K2_2[1] == niz[i]) {
				K2_2[1] = i;
			}
			if (K2_2[2] == niz[i]) {
				K2_2[2] = i;
			}
			if (K2_2[3] == niz[i]) {
				K2_2[3] = i;
			}
			if (procenat2_1[0] == niz[i]) {
				procenat2_1[0] = i;
			}
			if (procenat2_1[1] == niz[i]) {
				procenat2_1[1] = i;
			}
			if (procenat2_2[0] == niz[i]) {
				procenat2_2[0] = i;
			}
			if (procenat2_2[1] == niz[i]) {
				procenat2_2[1] = i;
			}
			if (procenat2_2[2] == niz[i]) {
				procenat2_2[2] = i;
			}

		}
	}
}

void task_obrada_senzori(void* pvParams) {

	uint8_t niz[] = { 48, 49, 50, 51, 52, 53, 54, 55, 56, 57 };
	uint8_t ls1[3] = { 0, 0 , 0 };
	uint8_t ls2[3] = { 0, 0 , 0 };

	uint8_t mm1[3] = { 0, 0 , 0 };
	uint8_t mm2[3] = { 0, 0 , 0 };

	uint8_t ls[4], ds[4] = { 0 };      // konverzija za ispis

	uint8_t calibratedPercent1, calibratedPercent2 = 0; //promjenjive za kalibraciju izmjerenih vrijednosti sa senzora
	float slope1, slope2 = 0.0f;
	float intercept1, intercept2 = 0.0f;
	float percent1, percent2, percent3, percent4 = 0.0f;
	float sensorValue1, sensorValue2 = 0.0f;
	float sensor1, sensor2, sensor3, sensor4 = 0.0f;
	float intermediateResult1, intermediateResult2 = 0.0f; // promjenjive za medju korak zbog kastovanja
	
	BaseType_t queueSendResult;
	BaseType_t queueReceiveResult;

	for (;;) {
		if (xSemaphoreTake(obrada_BS3, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		queueReceiveResult = xQueueReceive(que_S1, &ls1, portMAX_DELAY); // za primanje podataka iz redova za senzore sa oba kanala
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red que_S1\n");
		}
		queueReceiveResult = xQueueReceive(que_S2, &ls2, portMAX_DELAY);
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red que_S2\n");
		}

		mm1[0] = (ls1[0] / (uint8_t)16) * (uint8_t)10 + ls1[0] % (uint8_t)16;
		mm2[0] = (ls2[0] / (uint8_t)16) * (uint8_t)10 + ls2[0] % (uint8_t)16;

		ls[0] = (uint8_t)mm1[0] / (uint8_t)10 + (uint8_t)48;
		ls[1] = mm1[0] % 10u + (uint8_t)48;
		ls[2] = ls1[1];
		ls[3] = ls1[2];
		ds[0] = (uint8_t)mm2[0] / (uint8_t)10 + (uint8_t)48;
		ds[1] = mm2[0] % 10u + (uint8_t)48;
		ds[2] = ls2[1];
		ds[3] = ls2[2];

		for (uint8_t i = 0; i < (uint8_t)10; i++) { //KONVERZIJE IZ HEX U DEC
			if (ls1[1] == niz[i]) {
				mm1[1] = i;
			}
			if (ls1[2] == niz[i]) {
				mm1[2] = i;
			}
			if (ls2[1] == niz[i]) {
				mm2[1] = i;
			}
			if (ls2[2] == niz[i]) {
				mm2[2] = i;
			}

		}

	    sensorValue1 = (100.0f * (float)mm1[0] + 10.0f * (float)mm1[1] + (float)mm1[2]); // KANAL 0 OBRADA PODATAKA
		percent1 = (10.0f * (float)procenat1_1[0] + (float)procenat1_1[1]);
		percent2 = (100.0f * (float)procenat1_2[0]);
		sensor1 = (100.0f * (float)K1_1[0] + 10.0f * (float)K1_1[1] + (float)K1_1[2]);
		sensor2 = (1000.0f * (float)K1_2[0] + 100.0f * (float)K1_2[1] + 10.0f * (float)K1_2[2] + (float)K1_2[3]);

		slope1 = (percent2 - percent1) / (sensor2 - sensor1);
		intercept1 = percent1 - slope1 * sensor1;
		intermediateResult1 = slope1 * sensorValue1 + intercept1;
		calibratedPercent1 = (uint8_t)intermediateResult1;

		sensorValue2 = (100.0f * (float)mm2[0] + 10.0f * (float)mm2[1] + (float)mm2[2]); // KANAL 1  OBRADA PODATAKA 
		percent3 = (10.0f * (float)procenat2_1[0] + (float)procenat2_1[1]);
		percent4 = (100.0f * (float)procenat2_2[0]);
		sensor3 = (100.0f * (float)K2_1[0] + 10.0f * (float)K2_1[1] + (float)K2_1[2]);
		sensor4 = (1000.0f * (float)K2_2[0] + 100.0f * (float)K2_2[1] + 10.0f * (float)K2_2[2] + (float)K2_2[3]);

		slope2 = (percent4 - percent3) / (sensor4 - sensor3);
		intercept2 = percent3 - slope2 * sensor3;
		intermediateResult2 = slope2 * sensorValue2 + intercept2;
		calibratedPercent2 = (uint8_t)intermediateResult2;
		
		queueSendResult = xQueueSend(red_procenti1, &calibratedPercent1, portMAX_DELAY);
		if (queueSendResult != pdPASS) {
			printf("Greska pri slanju u red red_procenti1\n");
		}
		queueSendResult = xQueueSend(red_procenti2, &calibratedPercent2, portMAX_DELAY);
		if (queueSendResult != pdPASS) {
			printf("Greska pri slanju u red red_procenti2\n");
		}
		queueSendResult = xQueueSend(que_mm1, ls, portMAX_DELAY);
		if (queueSendResult != pdPASS) {
			printf("Greska pri slanju u red que_mm1\n");
		}
		queueSendResult = xQueueSend(que_mm2, ds, portMAX_DELAY);
		if (queueSendResult != pdPASS) {
			printf("Greska pri slanju u red que_mm2\n");
		}
		printf("PROCENAT1 %d\n", calibratedPercent1);
		printf("PROCENAT2 %d\n", calibratedPercent2);
	}
}

void diode_on(void* pvParameters) { //ukljucivanje dioda treceg stupca na osnovu udaljenosti

	uint8_t Percent1 = 0;
	uint8_t	Percent2 = 0;
	uint8_t poruka1 = 1;
	uint8_t poruka2 = 2;
	uint8_t poruka3 = 3;

	BaseType_t queueReceiveResult;
	BaseType_t queueSendResult;

	for (;;) {

		if (xSemaphoreTake(diode_BS, portMAX_DELAY) != pdTRUE) {
			printf("Greska kod preuzimanja semafora\n");
		}

		queueReceiveResult = xQueueReceive(red_procenti1, &Percent1, portMAX_DELAY); // za primanje podataka iz redova za senzore sa oba kanala
		//printf("PROCENTI1 %d\n", Percent1);
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red red_procenti1\n");
		}
		queueReceiveResult = xQueueReceive(red_procenti2, &Percent2, portMAX_DELAY);
		//printf("PROCENTI2 %d\n", Percent2);
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red red_procenti2\n");
		}

		if (Percent1 < Percent2) {
			if (Percent1 > (uint8_t)100) {
				if (set_LED_BAR(2, 0x00) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka1, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent1 <= (uint8_t)100) && (Percent1 > (uint8_t)90)) {
				if (set_LED_BAR(2, 0x01) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}

			}
			else if ((Percent1 <= (uint8_t)90) && (Percent1 > (uint8_t)80)) {
				if (set_LED_BAR(2, 0x03) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent1 <= (uint8_t)80) && (Percent1 > (uint8_t)70)) {
				if (set_LED_BAR(2, 0x07) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent1 <= (uint8_t)70) && (Percent1 > (uint8_t)60)) {
				if (set_LED_BAR(2, 0x0f) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent1 <= (uint8_t)60) && (Percent1 > (uint8_t)50)) {
				if (set_LED_BAR(2, 0x1f) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent1 <= (uint8_t)50) && (Percent1 > (uint8_t)40)) {
				if (set_LED_BAR(2, 0x3f) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent1 <= (uint8_t)40) && (Percent1 > (uint8_t)30)) {
				if (set_LED_BAR(2, 0x7f) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if (Percent1 <= (uint8_t)30) {
				if (set_LED_BAR(2, 0xff) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka2, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else {
				printf("Nedozvoljeno stanje");
			}

		}
		else if (Percent1 > Percent2) {
			if (Percent2 > (uint8_t)100) {
				if (set_LED_BAR(2, 0x00) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka1, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent2 <= (uint8_t)100) && (Percent2 > (uint8_t)90)) {
				if (set_LED_BAR(2, 0x01) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent2 <= (uint8_t)90) && (Percent2 > (uint8_t)80)) {
				if (set_LED_BAR(2, 0x03) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent2 <= (uint8_t)80) && (Percent2 > (uint8_t)70)) {
				if (set_LED_BAR(2, 0x07) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent2 <= (uint8_t)70) && (Percent2 > (uint8_t)60)) {
				if (set_LED_BAR(2, 0x0f) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent2 <= (uint8_t)60) && (Percent2 > (uint8_t)50)) {
				if (set_LED_BAR(2, 0x1f) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent2 <= (uint8_t)50) && (Percent2 > (uint8_t)40)) {
				if (set_LED_BAR(2, 0x3f) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if ((Percent2 <= (uint8_t)40) && (Percent2 > (uint8_t)30)) {
				if (set_LED_BAR(2, 0x7f) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka3, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else if (Percent2 <= (uint8_t)30) {
				if (set_LED_BAR(2, 0xff) != 0) {
					printf("Greska\n");
				}
				queueSendResult = xQueueSend(serial_que, &poruka2, portMAX_DELAY);
				if (queueSendResult != pdPASS) {
					printf("Greska pri slanju u red que_mm2\n");
				}
			}
			else {
				printf("Nedozvoljeno stanje");
			}
		}
		else {
			printf("Nedozvoljeno stanje");
		}

	}
}

void Serial2Send_Task(void* pvParameters) {

	uint8_t poruka = 0;
	uint8_t ispis_mm1[4] = { 0, 0 , 0 , 0 };
	uint8_t ispis_mm2[4] = { 0, 0 , 0 , 0 };

	char poruka1[] = " NEMA_DETEKCIJE. ";
	char poruka2[] = " KONTAKT_DETEKCIJA. ";
	char poruka3[] = "Mjerenje senzora S1 u mm: ";
	char poruka4[] = " a S2 u mm: ";
	char poruka5[] = " DETEKCIJA. ";

	BaseType_t queueReceiveResult;

	for (;;) {


		for (uint8_t i = 0; i < 26u; i++) {
			if (send_serial_character((uint8_t)COM_CH_2, (uint8_t)poruka3[i]) != pdFALSE) {
				printf("Greska\n");
			}
			vTaskDelay(pdMS_TO_TICKS(90));
		}

		if (xSemaphoreTake(TBE_BS_2, portMAX_DELAY) != pdTRUE) {
			printf("Greska\n");
		}

		queueReceiveResult = xQueueReceive(que_mm1, &ispis_mm1, portMAX_DELAY); // za primanje podataka iz redova za senzore
		/*printf("ispis_mm1: ");
		for (size_t  i = 0; i < sizeof(ispis_mm1) / sizeof(ispis_mm1[0]); i++) {
			printf("%d ", ispis_mm1[i]);
		}
		printf("\n");*/
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red que_mm1\n");
		}
		queueReceiveResult = xQueueReceive(que_mm2, &ispis_mm2, portMAX_DELAY);
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri prijemu u red que_mm2\n");
		}
		/*printf("ispis_mm2: ");
		for (size_t  i = 0; i < sizeof(ispis_mm2) / sizeof(ispis_mm2[0]); i++) {
			printf("%d ", ispis_mm2[i]);
		}
		printf("\n");*/
		for (uint8_t i = 0; i < 4u; i++) {
			if (send_serial_character((uint8_t)COM_CH_2, ispis_mm1[i]) != pdFALSE) {
				printf("Greska\n");
			}
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		for (uint8_t i = 0; i < 12u; i++) {
			if (send_serial_character((uint8_t)COM_CH_2, (uint8_t)poruka4[i]) != pdFALSE) {
				printf("Greska\n");
			}
			vTaskDelay(pdMS_TO_TICKS(70));
		}


		for (uint8_t i = 0; i < 4u; i++) {
			if (send_serial_character((uint8_t)COM_CH_2, ispis_mm2[i]) != pdFALSE) {
				printf("Greska\n");
			}
			vTaskDelay(pdMS_TO_TICKS(100));
		}

		queueReceiveResult = xQueueReceive(serial_que, &poruka, portMAX_DELAY);
		if (queueReceiveResult != pdPASS) {
			printf("Greska pri slanju u red que_mm2\n");
		}

		if (poruka == (uint8_t)1) {
			for (uint8_t i = 0; i < 17u; i++) {
				if (send_serial_character((uint8_t)COM_CH_2, (uint8_t)poruka1[i]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(70));
			}
		}
		if (poruka == (uint8_t)2) {
			for (uint8_t i = 0; i < 20u; i++) {
				if (send_serial_character((uint8_t)COM_CH_2, (uint8_t)poruka2[i]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(70));
			}
		}
		if (poruka == (uint8_t)3) {
			for (uint8_t i = 0; i < 12u; i++) {
				if (send_serial_character((uint8_t)COM_CH_2, (uint8_t)poruka5[i]) != pdFALSE) {
					printf("Greska\n");
				}
				vTaskDelay(pdMS_TO_TICKS(100));
			}
		}
	}
}

