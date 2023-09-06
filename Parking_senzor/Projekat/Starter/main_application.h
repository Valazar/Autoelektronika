#ifndef MAIN_APP
#define MAIN_APP

#include "stdint.h"

// TASK PRIORITIES 
#define	TASK_SERIAL_SEND_PRI	    ( tskIDLE_PRIORITY + 2 )
#define TASK_SERIAL_REC_PRI			( tskIDLE_PRIORITY + 3 )
#define	SERVICE_TASK_PRI			( tskIDLE_PRIORITY + 1 )

/* SERIAL SIMULATOR CHANNEL TO USE */
#define COM_CH_0 (0)
#define COM_CH_1 (1)
#define COM_CH_2 (2)

void main_demo(void);
void diode_on(void* pvParameters);
void LEDBar_Task(void* pvParameters);
void task_ukljuci_iskljuci(void* pvParameters);
void PC_Receive_task(void* pvParameters);
void RXC_isr_0(void* pvParameters);
void RXC_isr_1(void* pvParameters);
void Serial0Send_Task(void* pvParameters);
void Serial1Send_Task(void* pvParameters);
void Serial2Send_Task(void* pvParameters);
void task_obrada_kalibracija1(void* pvParameters);
void task_obrada_kalibracija2(void* pvParameters);
void task_obrada_senzori(void* pvParameters);
static void TimerCallback_obrada1(TimerHandle_t xTimer);
static void TimerCallback_obrada2(TimerHandle_t xTimer);
static void TimerCallback_obrada_senzori(TimerHandle_t xTimer);
static void TimerCallback_diode(TimerHandle_t xTimer);
static uint32_t OnLED_ChangeInterrupt(void);
//static void TimerCallback(TimerHandle_t xTimer);

//Globalne promenljive
//deklaracija
extern uint8_t K1_1[3];
extern uint8_t K1_2[4];
extern uint8_t K2_1[3];
extern uint8_t K2_2[4];
extern uint8_t procenat1_1[2];
extern uint8_t procenat1_2[3];
extern uint8_t procenat2_1[2];
extern uint8_t procenat2_2[3];

//definicija
uint8_t K1_1[3] = { 0 };			 // KOEFICIJENTI KALIBRACIJE1 1
uint8_t K1_2[4] = { 0 };		     // KOEFICIJENTI KALIBRACIJE1 2
uint8_t K2_1[3] = { 0 };			 // KOEFICIJENTI KALIBRACIJE2 1
uint8_t K2_2[4] = { 0 };			 // KOEFICIJENTI KALIBRACIJE2 2
uint8_t procenat1_1[2] = { 0 };
uint8_t procenat1_2[3] = { 0 };
uint8_t procenat2_1[2] = { 0 };
uint8_t procenat2_2[3] = { 0 };



#endif //MAIN_APP
