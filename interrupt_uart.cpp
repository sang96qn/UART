#include <msp430g2553.h>
#define LED_RED BIT0
#define LED_BLUE BIT6
#define RXD BIT1
#define TXD BIT2
#define RX_BUF UCA0RXBUF
//#define LED_RED_CONTROL 1
//#define LED_BLUE_CONTROL 2
/*-------UART----------------*/
unsigned long SMCLK_F=1000000;                 // frequency of Sub-System Master Clock in Hz
unsigned long BAUDRATE=9600;                   // may be ... 1200, 2400, 4800, 9600, 19200, ...
/*-------END_UART_DEFINE------*/
//*****************************************************************************
// Transfer  UART
//*****************************************************************************
void UART_Init();		                        //Khoi tao UART
void UART_Write_Char(char byte);	   			//VDK gui di 1 ki tu
void UART_Write_String(char* str);		  		//VDK gui chuoi ki tu
void UART_Write_Int(unsigned long num);		   	//VDK Goi so kieu int
void UART_Write_Reg(char *name, int n);			//In ra gia tri bit cua thanh ghi
void UART_Write_Float(float x,unsigned char coma);      //coma<=4
//*****************************************************************************
// Receive  UART
//*****************************************************************************
char UART_Read_Char();							//VDK nhan 1 ki tu
void UART_Read_String(char *str);				//VDK nhan chuoi ky tu
//*****************************************************************************
void config_GPIO();
//**************INTERRUPT_UART_USCIAB0RX******************
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)    	// xay ra ngat RX, tuc la khi MSP430G2553 nhan vao ky tu (receive), se xay ra chuong trinh ngat
{
  if(RX_BUF == '1'){
	P1OUT ^= LED_RED;
  }
  else if(RX_BUF == '2'){
	P1OUT ^= LED_BLUE;
  }
}
//*************************************************************
int main()
{
  config_GPIO();
  UART_Init();
  __bis_SR_register(LPM0_bits + GIE); // Power Save Mode
}
void config_GPIO()
{
  WDTCTL = WDTPW + WDTHOLD; // Stop Watchdog Timer
  P1DIR |= LED_RED +LED_BLUE;
  P1OUT &=~ LED_RED + LED_BLUE;
}

/*----------------------- UART LIBRARY MSP430G2553------------------------------*/
/*
 * Cac ky tu dac biet
 * 	0 Ky tu rong
 * 	1 Bat dau header
 * 	2 Bat dau van ban
 * 	3 Ket thuc van ban
 * 	4 Ket thuc truyen
 * 	5 Truy van
 * 	7 Tab ngang
 * 	8 Xoa nguoc
 * 	9 Tab ngang
 * 	10 Xuong dong
 * 	11 Tab doc
 */
void UART_Init(){							   // cai dat cau hinh de su dung chuc nang UART
	unsigned int tempfactor;
	P1SEL |= RXD + TXD;          // P1.1 = RXD, P1.2=TXD
	P1SEL2 |= RXD + TXD;          // P1.1 = RXD, P1.2=TXD

	UCA0CTL0 =0x00;		       		//Tat Parity ,LSB first,8-bit data,one stop bits
	UCA0CTL1 |= UCSWRST + UCSSEL_2; // Chon nguon Clock l� DCO SMCLK
 	tempfactor = SMCLK_F/BAUDRATE;
	UCA0BR0 = (unsigned char) tempfactor&0x00FF;
	tempfactor >>= 8;
	UCA0BR1 = (unsigned char) (tempfactor&0x00FF);
	UCA0CTL1 &=~ UCSWRST; 			// **Initialize USCI state machine** - Reset module de hoat dong
	IE2 |= UCA0RXIE; 				// Enable USCI_A0 RX interrupt
	__bis_SR_register(GIE); 		// Interrupts enabled
}

void UART_Write_Char(char byte){
	while (!(IFG2 & UCA0TXIFG)); 	//	Doi gui xong ky tu truoc - cho den khi bo dem transmit rong - co ngat UCA0TXIFG duoc set len 1 khi thanh ghi UCA0TXBUF rong
	UCA0TXBUF = byte; 				// 	Ky tu can gui duoc gan vo bo dem transmit va se duoc gui di
}


void UART_Write_String(char* str){
	while(*str){ 					// Het chuoi ky tu thi dung gui
		UART_Write_Char(*str++);}	// Gui tung ky tu trong chuoi bang ham UART_Write_Char()
}


void UART_Write_Int(unsigned long num){
     unsigned char buffer[16];
     unsigned char i,j;

     if(num == 0){
    	 UART_Write_Char('0');
          return;}

     for(i = 15; i > 0 && num > 0; i--){
         buffer[i] = (num%10)+'0';
         num /= 10;}

     for(j = i+1; j <= 15; j++) {
    	 UART_Write_Char(buffer[j]);}
}

void UART_Write_Reg(char *name, int n){
	int size = 8;
    int i;

    UART_Write_String((char *)"- ");
    UART_Write_String(name);
    UART_Write_String((char *)": ");

    int mask = 1 << (size - 1);

    for(i = 0; i < size; i++) {
         if((n & (mask >> i)) != 0) {
        	 UART_Write_Char('1');
         } else {
        	 UART_Write_Char('0');
         }
    }
    UART_Write_String(" (");
	UART_Write_Int(n);
	UART_Write_String(")\n\r");
}

void UART_Write_Float(float x, unsigned char coma){
	unsigned long temp;
	if(coma>4)coma=4;
	// de trong 1 ki tu o dau cho hien thi dau
	if(x<0){
		UART_Write_Char('-');			//In so am
		x*=-1;}

	temp = (unsigned long)x;			// Chuyan thanh so nguyen.

	UART_Write_Int(temp);

	x=((float)x-temp);					//*mysqr(10,coma);
	if(coma!=0)UART_Write_Char('.');	// In ra dau "."
	while(coma>0){
		x*=10;
		coma--;}

	temp=(unsigned long)(x+0.5);		//Lam tron
	UART_Write_Int(temp);
}

char UART_Read_Char(){
	while (!(IFG2 & UCA0RXIFG)); 		// Doi den khi co ky tu gui den - cho den khi bo dem receiver tran - co ngat UCA0RXBUF duoc set len 1 khi thanh ghi UCA0RXBUF tran
	return UCA0RXBUF; 					// 	tra ve ky tu da gui den tu bo dem receiver
}

void UART_Read_String(char *str){
	*str = UART_Read_Char();
	while(*str!='\0'){
		str++;
		*str = UART_Read_Char();}
}
/*--------------------------END_UART_LIB--------------------*/
