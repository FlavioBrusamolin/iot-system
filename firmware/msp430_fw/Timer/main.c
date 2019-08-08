/******************************************************************************/
/* INATEL - INSTITUTO NACIONAL DE TELECOMUNICAÇÕES - EA203 EC207 EAT205       */
/* PROJETO: RELÓGIO COM TIMER                                                 */
/* AUTORA : PROFA ANA LETICIA GOMES GONCALVES                                 */
/* DEVICE: MSP430G2553 ( TEXAS INSTRUMENTS - LAUCHPAD)                        */
/* DESCRIÇÃO DO SOFTWARE: ESTE PROGRAMA CONSISTE EM CRIAR UM RELÓGIO A PARTIR */
/* DO TIMER PRESENTE NO MSP430.                                               */
/* HARDWARE:                                                                  */
/*      |           |                                                         */
/*  LED-|P1.0       |                                                         */
/*  RXD-|P1.1 (UART)|                                                         */
/*  TXD-|P1.2 (UART)|                                                         */
/*      |MSP430G2553|                                                         */
/*      |_ _ _ _ _ _|                                                         */
/******************************************************************************/ 
#include <msp430g2553.h>
#include <stdio.h>
#include "uart.h"
#include "timer.h"
/******************************************************************************/ 
/***************************VARIÁVEIS GLOBAIS**********************************/ 
extern char hora,minuto,segundo,milisegundo;    //declaradas em timer.c
extern unsigned int tempo_limite;            //declarada em timer.c
char relogio_texto[3]; 
char rx;
/******************************************************************************/ 
void main (void)
{
  WDTCTL=WDTHOLD+WDTPW;                       //Paralisa o watchdog
  DCOCTL=CALDCO_1MHZ;                       //Calibra para 1MHz
  BCSCTL1=CALBC1_1MHZ;
  //Intervalo=100ms. Se clock=1MHz, T=1us. Entao: contagem=50000 divisor=2
  //50000 x 1us/2 = 50000 x 2us = 100000us = 100ms
  ConfigTimer0(50000,2);                      //Inicia o relogio para funcionar
  ConfigUART(9600);                          //Configura a UART
  P1DIR=P1DIR|BIT0;                          //Configura P1.0 como saida
  P1OUT=P1OUT&~BIT0;
  _BIS_SR(GIE);                             //Habilita interrupcao global
  StartTimer0();
  for(;;)
  {
    sprintf(relogio_texto,"%.2d",hora); 
    UARTSend(relogio_texto);
    UARTSend (":");
    sprintf(relogio_texto,"%.2d",minuto); 
    UARTSend (relogio_texto);
    UARTSend (":");
    sprintf(relogio_texto,"%.2d",segundo); 
    UARTSend (relogio_texto);
    UARTSend ("\n\r");
    
    UARTReceive(&rx,0);                         //Verifica se recebeu algo
    if(rx!=0)   
    {
      tempo_limite=30;                         //30x100ms = 3segundos
      P1OUT=P1OUT&~BIT0;
    }
    if(tempo_limite==0)                         //Estourou o tempo para contar?
    {
      P1OUT=P1OUT|BIT0;                         //liga o led
    }
  } 
}