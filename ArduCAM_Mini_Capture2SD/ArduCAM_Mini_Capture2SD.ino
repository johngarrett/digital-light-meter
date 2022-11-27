#include <ArduCAM.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
// #include "memorysaver.h"

#define SD_CS 4
const int SPI_CS = 12;

ArduCAM myCAM( OV2640, SPI_CS );

void myCAMSaveToSDFile() {
  char str[8];
  byte buf[256];
  static int i = 0;
  static int k = 0;
  uint8_t temp = 0,temp_last=0;
  uint32_t length = 0;
  bool is_header = false;
  File outFile;
  
  //Flush the FIFO
  myCAM.flush_fifo();
  //Clear the capture done flag
  myCAM.clear_fifo_flag();
  //Start capture
  myCAM.start_capture();
  Serial.println(F("start capture"));
  while(!myCAM.get_bit(ARDUCHIP_TRIG , CAP_DONE_MASK));
  Serial.println(F("Capture Done."));  
  length = myCAM.read_fifo_length();
  Serial.print(F("The fifo length is :"));
  Serial.println(length, DEC);
  if (length >= MAX_FIFO_SIZE) { //384K
    Serial.println(F("Over size."));
    return;
  }

  if (length == 0) { //0 kb
    Serial.println(F("Size is 0."));
    return;
  }

  //Construct a file name
  k = k + 1;
  itoa(k, str, 10);
  strcat(str, ".jpg");
  //Open the new file
  outFile = SD.open(str, O_WRITE | O_CREAT | O_TRUNC);
  if(!outFile) {
    Serial.println(F("File open faild"));
    return;
  }

  myCAM.CS_LOW();
  myCAM.set_fifo_burst();

  while (length--) {
    temp_last = temp;
    temp = SPI.transfer(0x00);

    //Read JPEG data from FIFO
    if ((temp == 0xD9) && (temp_last == 0xFF)) { //If find the end ,break while,
      Serial.println(F("temp is 0xD9 and temp_last is 0xFF"));
      buf[i++] = temp;  //save the last  0XD9     
      //Write the remain bytes in the buffer
      myCAM.CS_HIGH();
      outFile.write(buf, i);    
      //Close the file
      outFile.close();
      Serial.println(F("Image save OK."));
      is_header = false;
      i = 0;
    }

    if (is_header == true) { 
      Serial.println(F("is header"));
      //Write image data to buffer if not full
      if (i < 256) {
        buf[i++] = temp;
      } else {
        //Write 256 bytes image data to file
        myCAM.CS_HIGH();
        outFile.write(buf, 256);
        i = 0;
        buf[i++] = temp;
        myCAM.CS_LOW();
        myCAM.set_fifo_burst();
      }        
    } else if ((temp == 0xD8) && (temp_last == 0xFF)) {
      is_header = true;
      buf[i++] = temp_last;
      buf[i++] = temp;   
    }
  }
}

void setup() {
  uint8_t vid, pid;
  uint8_t temp;
  Wire.begin();
  Serial.begin(115200);
  Serial.println(F("ArduCAM Start!"));
  //set the CS as an output:
  pinMode(SPI_CS,OUTPUT);
  digitalWrite(SPI_CS, HIGH);
  // initialize SPI:
  SPI.begin();
    
  //Reset the CPLD
  myCAM.write_reg(0x07, 0x80);
  delay(100);
  myCAM.write_reg(0x07, 0x00);
  delay(100);
    
  while(1) {
    //Check if the ArduCAM SPI bus is OK
    myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
    temp = myCAM.read_reg(ARDUCHIP_TEST1);
    
    if (temp != 0x55) {
      Serial.println(F("SPI interface Error!"));
      delay(1000);
      continue;
    } else {
      Serial.println(F("SPI interface OK."));
      break;
    }
  }
  //Initialize SD Card
  while(!SD.begin(SD_CS)) {
    Serial.println(F("SD Card Error!"));delay(1000);
  }
  Serial.println(F("SD Card detected."));
  while(1) {
    //Check if the camera module type is OV2640
    myCAM.wrSensorReg8_8(0xff, 0x01);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
    if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))){
      Serial.println(F("Can't find OV2640 module!"));
      delay(1000);continue;
    }
    else{
      Serial.println(F("OV2640 detected."));break;
    } 
  }
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  myCAM.OV2640_set_JPEG_size(OV2640_320x240);
  delay(1000);
}

void loop(){
  myCAMSaveToSDFile();
  delay(5000);
}
