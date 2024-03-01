#include <Digital scale.h>
#use rs232(baud = 9600, xmit = PIN_C6, rcv = PIN_C7)
#use delay(clock = 4M)

#define DT pin_A1
#define SCK pin_A0

#define LCD_RS_PIN PIN_D0            
#define LCD_RW_PIN PIN_D1
#define LCD_ENABLE_PIN PIN_D2
#define LCD_DATA4 PIN_D3
#define LCD_DATA5 PIN_D4
#define LCD_DATA6 PIN_D5
#define LCD_DATA7 PIN_D6
#include <lcd.c>

char price[10];     

#INT_RDA 
void RDA_isr(void)
{
   static int cnt= 0;
   int ch;
   ch = getc();
   
   if (ch == '\n')
   {
      price[cnt] = '\0';
      cnt = 0;
      return;
   }
 
   price[cnt++] = ch;
   if (cnt == 10)  cnt--;
}
int led[2] = {PIN_C4, PIN_C5};



unsigned int32 readCount() 
{
  unsigned int32 return_val;
  unsigned int8 i;
  
  output_bit(DT, 1);
  output_bit(SCK, 0);
  return_val = 0;

  while (input(DT));
  for (i = 0; i < 24; i++) 
  {
    output_bit(SCK, 1);
    return_val = return_val << 1;
    output_bit(SCK, 0);
    
    if (input(DT)) 
    {
      return_val++;
    }
  }
  output_bit(SCK, 1);
  return_val = return_val ^ 0x800000;
  output_bit(SCK, 0);
  return return_val;
}

int32 cal_Average() {
  unsigned int32 sum = 0;
  for (int k = 0; k < 10; k++) {
    sum = sum + readCount();
  }
  sum = sum / 10;
  return sum;
}





void main() 
{
  lcd_init();
  lcd_putc ('\f');
  lcd_gotoxy(1,1);
  lcd_putc("OMRON");
  
  enable_interrupts(INT_RDA);
  enable_interrupts(GLOBAL);
  
  unsigned int32 value = 0, init_val = 0;
  float hx711_val = 0;
  
  init_val = cal_Average();
  int i = 0;
  
  while (TRUE) 
  {
    value = cal_Average();
    
    output_high(led[i]);
    delay_ms(50);
    output_low(led[i]);
    i = i + 1;
    if (i == 2) i=0;
    
    if (init_val >= value)   
    {
      float result1 = (init_val - value);
      hx711_val = result1;     
    } 
    else 
    {
      float result2 = (value - init_val);
      hx711_val = result2;
    } 
   
   float cali_gain = 900.65;   

   
   printf("%f@   ", hx711_val / (float)cali_gain);   
   lcd_putc ('\f');
   lcd_gotoxy(1,1);
   lcd_putc ("Weight: ");
   lcd_gotoxy(8,1);
   printf(lcd_putc, "%f ", hx711_val / (float)cali_gain);
   lcd_gotoxy(16,1);
   lcd_putc ("g");
   
   lcd_gotoxy(1,2);
   lcd_putc ("Price: ");
   
   lcd_gotoxy(8,2);
   printf(lcd_putc, "%s", price);
   
   delay_ms(50);
  }
}

