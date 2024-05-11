#include <SPI.h>
#include "Ap_29demo.h"
#include "imagedata.h"
#include "tandan.h"
//IO settings
int BUSY_Pin = D1;
int RES_Pin = D4;
int DC_Pin = D2;
int CS_Pin = D8;
//HSCLK---D5
//HMOSI--D7

unsigned char HRES, VRES_byte1, VRES_byte2;

#define EPD_W21_CS_0 digitalWrite(CS_Pin, LOW)
#define EPD_W21_CS_1 digitalWrite(CS_Pin, HIGH)
#define EPD_W21_DC_0 digitalWrite(DC_Pin, LOW)
#define EPD_W21_DC_1 digitalWrite(DC_Pin, HIGH)
#define EPD_W21_RST_0 digitalWrite(RES_Pin, LOW)
#define EPD_W21_RST_1 digitalWrite(RES_Pin, HIGH)
#define isEPD_W21_BUSY digitalRead(BUSY_Pin)
////////FUNCTION//////
void SPI_Write(unsigned char value);
void EPD_W21_WriteDATA(unsigned char command);
void EPD_W21_WriteCMD(unsigned char command);
//EPD
void EPD_W21_Init(void);
void EPD_init(void);
void EPD_sleep(void);
void EPD_refresh(void);
void lcd_chkstatus(void);
void PIC_display_Clean(void);
void PIC_display(const unsigned char* picData);
void Sys_run(void) {
  ESP.wdtFeed();  //Feed dog to prevent system reset
}
void LED_run(void) {
  digitalWrite(LED_BUILTIN, LOW);  // Turn the LED on (Note that LOW is the voltage level
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED on (Note that LOW is the voltage level
  delay(500);
}

void setup() {
  Serial.begin(115200);
  pinMode(BUSY_Pin, INPUT);
  pinMode(RES_Pin, OUTPUT);
  pinMode(DC_Pin, OUTPUT);
  pinMode(CS_Pin, OUTPUT);
  Serial.println("\nthis is Epaper start to write context");
  //SPI
  SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
  SPI.begin();
  /************Normal picture display*******************/
  EPD_init();           //EPD init
  PIC_display_Clean();  //EPD Clear
  // PIC_display_Clean_Bar();
  // PIC_display_Clean_white();
  // return;
  // PIC_display(gImage_BW1,gImage_R1);//EPD_picture1
  // delay(8000);  //3s
  // // PIC_display(epd_bitmap_td_bw);  //黑白图片显示
  PIC_display_My(epd_bitmap_td_bw, epd_bitmap_td_rw);  //画坦荡
  // // PIC_display(gImage_tandan,gImage_tandan);
  // delay(500);   //3s
  // EPD_sleep();  //EPD_sleep,Sleep instruction is necessary, please do not delete!!!
  delay(8000);  //3s
}
//Tips//
/*When the electronic paper is refreshed in full screen, the picture flicker is a normal phenomenon, and the main function is to clear the display afterimage in the previous picture.
  When the local refresh is performed, the screen does not flash.*/
/*When you need to transplant the driver, you only need to change the corresponding IO. The BUSY pin is the input mode and the others are the output mode. */

void loop() {
 
}

//////////////////////SPI///////////////////////////////////

void SPI_Write(unsigned char value) {
  SPI.transfer(value);
}

void EPD_W21_WriteCMD(unsigned char command) {
  EPD_W21_CS_0;
  EPD_W21_DC_0;  // command write
  SPI_Write(command);
  EPD_W21_CS_1;
}
void EPD_W21_WriteDATA(unsigned char command) {
  EPD_W21_CS_0;
  EPD_W21_DC_1;  // command write
  SPI_Write(command);
  EPD_W21_CS_1;
}


/////////////////EPD settings Functions/////////////////////
void EPD_W21_Init(void) {
  EPD_W21_RST_0;  // Module reset
  delay(20);      //At least 10ms delay
  EPD_W21_RST_1;
  delay(20);  //At least 10ms delay
}
void EPD_init(void) {
  HRES = 0x98;        //152
  VRES_byte1 = 0x01;  //296 == 128Hex
  VRES_byte2 = 0x28;

  EPD_W21_Init();           //Electronic paper IC reset
  EPD_W21_WriteCMD(0x04);   //Power on
  lcd_chkstatus();          //waiting for the electronic paper IC to release the idle signal
  EPD_W21_WriteCMD(0x00);   //panel setting
  EPD_W21_WriteDATA(0x0f);  //LUT from OTP
  EPD_W21_WriteDATA(0x89);
  EPD_W21_WriteCMD(0x01);  //Power Steeing (PWR)
  EPD_W21_WriteDATA(0x03);
  EPD_W21_WriteDATA(0x00);  //
  EPD_W21_WriteDATA(0x21);
  EPD_W21_WriteDATA(0x21);
  EPD_W21_WriteDATA(0x01);
  EPD_W21_WriteCMD(0x41);
  EPD_W21_WriteDATA(0x07);
  EPD_W21_WriteCMD(0x61);  //resolution setting
  EPD_W21_WriteDATA(HRES);
  EPD_W21_WriteDATA(VRES_byte1);
  EPD_W21_WriteDATA(VRES_byte2);
  EPD_W21_WriteCMD(0X50);   //VCOM AND DATA INTERVAL SETTING
  EPD_W21_WriteDATA(0x97);  //(0x77);    //WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7
}
void EPD_refresh(void) {
  EPD_W21_WriteCMD(0x12);  //DISPLAY REFRESH
  delay(10);               //!!!The delay here is necessary, 200uS at least!!!
  lcd_chkstatus();
}
void EPD_sleep(void) {
  EPD_W21_WriteCMD(0X02);  //power off
  lcd_chkstatus();         //waiting for the electronic paper IC to release the idle signal
  //Part2 Increase the time delay
  delay(1000);             //Power off time delay, this is  necessary!!!
  EPD_W21_WriteCMD(0X07);  //deep sleep
  EPD_W21_WriteDATA(0xA5);
}
void PIC_display(const unsigned char* picData_old) {
  unsigned int i;
  //Write Data
  EPD_W21_WriteCMD(0x10);  //Transfer old data
  for (i = 0; i < 5624; i++) {
    // unsigned char xx = ~pgm_read_byte(&picData_old[i]);
    // Serial.println(xx);
    EPD_W21_WriteDATA(pgm_read_byte(&picData_old[i]));
  }
  EPD_W21_WriteCMD(0x13);  //红白模式下，00代表不写，ff代表红色填充
  for (i = 0; i < 5624; i++) {
    if (i < 1875) {
      EPD_W21_WriteDATA(0x00);  //第一段白色
    } else if (i < 3748) {
      EPD_W21_WriteDATA(0X00);  //第二段
    } else {
      EPD_W21_WriteDATA(0x00);  //第三段红色
    }
  }


  EPD_W21_WriteCMD(0x12);  //DISPLAY REFRESH
  delay(20);               //!!!The delay here is necessary, 200uS at least!!!
  lcd_chkstatus();
}

void PIC_display_My(const unsigned char* picData_old, const unsigned char* picData_new) {
  unsigned int i;
  //Write Data
  EPD_W21_WriteCMD(0x10);  //Transfer old data
  Serial.println("Start To Write Black code");
  for (i = 0; i < 5624; i++) {
    EPD_W21_WriteDATA(pgm_read_byte(&picData_old[i]));
  }
  EPD_W21_WriteCMD(0x13);  //Transfer new data
  Serial.println("Finsh To Write Black code");
  for (i = 0; i < 5624; i++) {
    unsigned char data;
    data = ~pgm_read_byte(&picData_new[i]);
    // Serial.println(data);
    EPD_W21_WriteDATA(data);
  }
  Serial.println("Finsh To Write Red code");
  //Refresh
  EPD_W21_WriteCMD(0x12);  //DISPLAY REFRESH
  delay(20);               //!!!The delay here is necessary, 200uS at least!!!
  lcd_chkstatus();         //waiting for the electronic paper IC to release the idle signal
}

void PIC_display(const unsigned char* picData_old, const unsigned char* picData_new) {
  unsigned int i;
  //Write Data
  EPD_W21_WriteCMD(0x10);  //Transfer old data
  Serial.println("Start To Write Black code");
  for (i = 0; i < 5624; i++) {
    EPD_W21_WriteDATA(~pgm_read_byte(&picData_old[i]));
  }
  EPD_W21_WriteCMD(0x13);  //Transfer new data
  Serial.println("Finsh To Write Black code");
  for (i = 0; i < 5624; i++) {
    EPD_W21_WriteDATA(~pgm_read_byte(&picData_new[i]));
  }
  Serial.println("Finsh To Write Red code");
  //Refresh
  EPD_W21_WriteCMD(0x12);  //DISPLAY REFRESH
  delay(20);               //!!!The delay here is necessary, 200uS at least!!!
  lcd_chkstatus();         //waiting for the electronic paper IC to release the idle signal
}


void PIC_display_Clean(void) {
  unsigned int i;
  EPD_W21_WriteCMD(0x10);  //Transfer old data
  for (i = 0; i < 5624; i++) {
    EPD_W21_WriteDATA(0xff);
  }

  EPD_W21_WriteCMD(0x13);  //Transfer new data
  for (i = 0; i < 5624; i++) {
    EPD_W21_WriteDATA(0x00);
  }
  //Refresh
  EPD_W21_WriteCMD(0x12);  //DISPLAY REFRESH
  delay(10);               //!!!The delay here is necessary, 200uS at least!!!
  lcd_chkstatus();         //waiting for the electronic paper IC to release the idle signal
}

void PIC_display_Clean_Bar(void) {
  int row = 296;
  int col = 19;
  unsigned int i,j;
  EPD_W21_WriteCMD(0x10);  //黑白模式下，ff代表写入白色，00代表写入黑色
  for(i=0;i<row;i++) {
    for(j=0;j<col;j++) {
      if(i%2==1) {
        EPD_W21_WriteDATA(0Xff); 
      }else{
        EPD_W21_WriteDATA(0xff); 
      }
    }
  }
  EPD_W21_WriteCMD(0x13);  //红白模式下，00代表不写，ff代表红色填充
  for(i=0;i<row;i++) {
    int k=1;
    for(j=0;j<col;j++) {
      
      if(i%32>=16) {
        if(k%4>=1)
        {
          EPD_W21_WriteDATA(0Xff); 
        }else{
          EPD_W21_WriteDATA(0X00); 
        }
        k++;
      }else{
        EPD_W21_WriteDATA(0x00); 
      }
    }
  }
  EPD_W21_WriteCMD(0x12);  //DISPLAY REFRESH
  delay(10);               //!!!The delay here is necessary, 200uS at least!!!
  lcd_chkstatus();         //waiting for the electronic paper IC to release the idle signal

}

void PIC_display_Clean_white(void) {
  unsigned int i;
  EPD_W21_WriteCMD(0x10);  //黑白模式下，ff代表写入白色，00代表写入黑色
  for (i = 0; i < 5624; i++) {
    if (i < 1875) {
      EPD_W21_WriteDATA(0X55);  //第一段白色
    } else if (i < 3748) {
      // EPD_W21_WriteDATA(0X00);
      EPD_W21_WriteDATA(0X00);  //第二段黑色

    } else {
      EPD_W21_WriteDATA(0X00);  //第二段黑色
      // break;
    }
  }

  EPD_W21_WriteCMD(0x13);  //红白模式下，00代表不写，ff代表红色填充
  for (i = 0; i < 5624; i++) {
    if (i < 1875) {
      EPD_W21_WriteDATA(0xaa);  //第一段白色
    } else if (i < 3748) {
      EPD_W21_WriteDATA(0X00);  //第二段
      // break;
    } else {
      EPD_W21_WriteDATA(0x55);  //第三段红色
    }
  }
  //Refresh
  EPD_W21_WriteCMD(0x12);  //DISPLAY REFRESH
  delay(10);               //!!!The delay here is necessary, 200uS at least!!!
  lcd_chkstatus();         //waiting for the electronic paper IC to release the idle signal
}
void lcd_chkstatus(void) {
  while (1) {  //=0 BUSY
    if (isEPD_W21_BUSY == 1) break;
    ESP.wdtFeed();  //Feed dog to prevent system reset
  }
}
