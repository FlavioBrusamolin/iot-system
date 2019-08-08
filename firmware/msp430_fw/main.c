#include "msp430g2553.h"

#include <inttypes.h>

#define OFF 0
#define ON  1

#define TEMP_INCH INCH_4

// P1
#define TEMP      BIT4
#define BUZZER    BIT0
#define VENTOINHA BIT6
#define LIGA      BIT3

// P2
#define LEDFRIO   BIT0
#define LEDMEDIO  BIT1
#define LEDQUENTE BIT2

#define FRIO   204 // 25C 170 30C 204
#define MEDIO  306 // 40C 272 45C 306

uint8_t estado = OFF;

uint16_t adc;

unsigned int  timer_cont = 0;

unsigned char tx_buf[32];
unsigned char tx_index;

#define abs(x) x < 0 ? -x : x;

uint8_t intlen(int16_t const value)
{
  //return log10(abs(value)) + (value < 0 ? 2 : 1);
  int16_t const absolute = abs(value);
  if (absolute < 10) return (value < 0 ? 2 : 1);
  if (absolute < 100) return (value < 0 ? 3 : 2);
  if (absolute < 1000) return (value < 0 ? 4 : 3);
  if (absolute < 10000) return (value < 0 ? 5 : 4);
  return (absolute < 0 ? 6 : 5);
}

char* itoa(int16_t value, char* const str)
{
  uint8_t length = intlen(value);
 
  str[length] = '\0';
  while (length > 0)
  {
    str[--length] = value % 10 + '0';
    value /= 10;
  }

  return str;
}

int8_t temperature(uint16_t adc)
{
    return adc * 20 / 136;
}

void config_adc(uint16_t pin, uint16_t inch)
{
  ADC10AE0 = pin;       // sets the pin as the ADC10 input
  ADC10CTL1 =
      inch              // configures the pin
    | SHS_0             // sampleandhold controlled by the ADC10SC bit
    | ADC10DIV_7        // selects clock division by 8
    | ADC10SSEL_0       // selects internal clock ADC10SC
    | CONSEQ_0;         // selects 0 mode
  ADC10CTL0 =
      SREF_1            // selects Vref+ as VR+ and GND as VR-
    | ADC10SHT_0        // sampleandhold time of 4 clock cycles
    | ADC10ON           // enables the ADC as an interrupt after the conversion
    | ADC10IE           // IE
    | REFON;            // Reference generator on
  ADC10CTL0 &= ~REF2_5V;
}

void println(char const * const str)
{
    while (tx_buf[0] != '\0');
    for (tx_index = 0; str[tx_index]; tx_index++)
    {
        tx_buf[tx_index] = str[tx_index];
    }
    tx_buf[tx_index++] = '\n';
    tx_buf[tx_index] = '\0';
    
    tx_index = 0;
    IFG2 |= UCA0TXIFG;
    //UC0IE |= UCA0TXIE;
}

void config_uart(uint16_t baudrate)
{
    const uint32_t SMCLOCK = 1000000ull;
    P1SEL  |= BIT1 | BIT2;
    P1SEL2 |= BIT1 | BIT2;
    
    /*DCOCTL  = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL  = CALDCO_1MHZ;*/
    
    UCA0CTL1 |=  UCSSEL_2;                      // submain clock (SMCLK) as source for UART
    UCA0BR0   =  (SMCLOCK / baudrate) % 256;    // 1 MHZ 9600
    UCA0BR1   =  (SMCLOCK / baudrate) / 256;    // 1 MHZ 9600
    UCA0MCTL  =  UCBRS2 + UCBRS0;               // Modulation UCBRSx = 5
    UCA0CTL1 &= ~UCSWRST;                       // Initialize USCI
    UC0IE    |=  UCA0TXIE | UCA0RXIE;           // IE
}

void config_timer(uint32_t interval, uint8_t divider)
{
    TACCR0 = interval - 1;
    TACTL |=
         TASSEL_2
        | divider
        | TAIE;      // Enables general TimerA0 interrupt
    TACCTL0 |= CCIE; // Enables CCR0 interrupt. Enables TAIFG interrupt request.
}

void StartTimer(void)
{
  TA0CTL=TA0CTL|MC_1;     //inicia contagem modo crescente
}

void StopTimer(void)
{
  TA0CTL=TA0CTL&(~MC_3);  //zera o campo MC para parar o timer
}

void led_control(unsigned int adc)
{
  if (adc <= FRIO)
  {
    P2OUT |= (LEDFRIO);
    P2OUT &= ~(LEDMEDIO | LEDQUENTE);
    StopTimer();
  }
  else if (adc > FRIO && adc <= MEDIO)
  {
    P2OUT |= (LEDMEDIO);
    P2OUT &= ~(LEDFRIO | LEDQUENTE);
    StopTimer();
  }
  else
  {
    P2OUT |= (LEDQUENTE);
    P2OUT &= ~(LEDFRIO | LEDMEDIO);
    StartTimer();
  }
}

int main( void )
{
  WDTCTL = WDTPW + WDTHOLD;
  
  P2DIR |= LEDFRIO | LEDMEDIO | LEDQUENTE;
  P2OUT &= ~(LEDFRIO | LEDMEDIO | LEDQUENTE);
  
  P1DIR |= BUZZER | VENTOINHA;
  P1OUT &= ~(BUZZER | VENTOINHA);
  P1REN |= LIGA;
  
  tx_index = 0;
  tx_buf[0] = '\0';
  
  config_uart(8526);
  config_adc(TEMP, TEMP_INCH);
  config_timer(10e3, ID_0);
  
  P1IE |= LIGA;
  P1IES |= LIGA;
  P1IFG &= ~LIGA;
  _BIS_SR(GIE);
  
  while(1) 
  {
    switch(estado) 
    {
      case OFF:
        P1OUT &= ~(BUZZER | VENTOINHA | TEMP);
        P2OUT &= ~(LEDFRIO | LEDMEDIO | LEDQUENTE);
        println("OFF");
        __delay_cycles(1e6);
        break;
      case ON:
        P1OUT &= ~BUZZER;
        ADC10CTL0 |= ENC | ADC10SC;
        P1OUT |= TEMP;
        __delay_cycles(1e6);
        char stradc[8];
        itoa(temperature(adc), stradc);
        println(stradc);
        break;
    }
  }
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  if ((P1IFG & LIGA) == LIGA)
  {
    if(estado == ON)
      estado = OFF;
    else
      estado = ON;
    P1IFG&= ~LIGA;
  }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
  char rx_buf = UCA0RXBUF;
  /*if (rx_buf == 'S' || rx_buf == 's')
  {
    estado = ON;
  }
  if (rx_buf == 'N' || rx_buf == 'n')
  {
    estado = OFF;
  }*/
  if (rx_buf == '1' && estado == ON)
  {
    // println("Ventoinha ligada");
    P1OUT |= VENTOINHA;
  }
  if (rx_buf == '0' && estado == ON)
  {
    // println("Ventoinha desligada");
    P1OUT &= ~VENTOINHA; 
  }
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCIA0TX_ISR(void)
{
  if (tx_buf[tx_index])
  {
    UCA0TXBUF = tx_buf[tx_index++];
  }
  else
  {
    tx_index = 0;
    tx_buf[0] = '\0';
    IFG2 &= ~UCA0TXIFG;
  }
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
  if ((ADC10CTL1 & TEMP_INCH) == TEMP_INCH)
  {
    adc = ADC10MEM;
    led_control(adc);
  }
  ADC10CTL0 &= ~ADC10IFG;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
  if (estado != ON)
  {
    timer_cont = 0;
    TACTL &= ~TAIFG;
    TACCTL0 &= ~CCIFG;
    return;
  }
  
  timer_cont++;
  if (timer_cont >= 200) // 2s
  {
    P1OUT |= BUZZER;
    __delay_cycles(1e6);
    timer_cont = 0;
  }
  
  TACTL &= ~TAIFG;
  TACCTL0 &= ~CCIFG;
}