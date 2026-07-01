// Generator mesaje CAN Logan
// pt Cluster Instrumente de bord
// v1_4
// 
// + toate pachetele CAN
// + variatie rpm, 
// - viteza nu functioneaza


#include <SPI.h>
#include <mcp_can.h>

// RPM
byte MSGx001[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}; 

// TimeOut = 10 ms
byte MSGx186[7] = {0x24, 0xC0, 0x31, 0x93, 0x1D, 0x00, 0x21}; 
//byte MSGx186[7] = {0x24, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00}; 
byte MSGx189[8] = {0x31, 0xD3, 0x1C, 0x38, 0x81, 0xCA, 0x00, 0x00};
byte MSGx18A[6] = {0x31, 0xD8, 0x00, 0x06, 0xA0, 0x04};

//byte MSGx1F6[8] = {0x00, 0x20, 0x80, 0x32, 0x80, 0x03, 0x0D, 0xFE};
byte MSGx1F6[8] = {0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// TimeOut = 20 ms
byte MSGx217[8] = {0x2D, 0x0D, 0x06, 0x00, 0x00, 0x00, 0x00, 0x60};
byte MSGx2A9[1] = {0x9C};
byte MSGx2C6[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
byte MSGx29A[8] = {0x06, 0xE0, 0x06, 0xF7, 0x03, 0x5A, 0x08, 0xB7};
byte MSGx29C[8] = {0x06, 0xF7, 0x06, 0xDB, 0xFF, 0xFF, 0xFF, 0xFF};

// TimeOut = 100 ms
byte MSGx41A[8] = {0x10, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
byte MSGx41D[4] = {0xF1, 0x21, 0x33, 0x90};
byte MSGx500[5] = {0x02, 0xAE, 0x2A, 0x93, 0x91};
byte MSGx511[7] = {0x00, 0x37, 0xB7, 0x7E, 0x44, 0xCC, 0xBD};

byte MSGx354[8] = {0x03, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};       // viteza vehicul

//byte MSGx5DA[8] = {0x48, 0x92, 0x00, 0x00, 0xC3, 0x61, 0x12, 0x20};
byte MSGx5DA[8] = {0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};       // temp motor
byte MSGx5DE[8] = {0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};       // LEDURI

byte MSGx648[8] = {0x00, 0x00, 0x00, 0x7E, 0xFC, 0x7E, 0xC8, 0x00};
byte MSGx65C[3] = {0x78, 0x99, 0x40};
byte MSGx66A[8] = {0x27, 0xFF, 0x00, 0xE4, 0xE4, 0xC0, 0x00, 0x00};

const int CS_PIN = 10;

long int CurrentTime;
long int LastTime1;
long int LastTime2;
long int LastTime3;
long int LastTime4;

MCP_CAN CAN0(CS_PIN);

long int rpm=0;
long int vss=0;
int j=0;
int stare=0;

void setup()
{
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH); delay(500); digitalWrite(4, LOW); delay(500);
  pinMode(6, OUTPUT);
  digitalWrite(6, HIGH); delay(500); digitalWrite(6, LOW); delay(500);
  pinMode(8, OUTPUT);
  digitalWrite(8, HIGH); delay(500); digitalWrite(8, LOW); delay(500);

  Serial.begin(115200);

  while (CAN_OK != CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ))
  {
    delay(100);
  }
  CAN0.setMode(MCP_NORMAL);   
  Serial.print("CAN controller initializat!");
  
}


void loop()
{
  CurrentTime = millis();
  
//  j++;
//  if (j>5)
//    {
//      if (rpm<55000) rpm++;
//      else rpm=0;
//      if (vss<2000) vss++;
//      else vss=0;
//      j=0;
//    }
    
  if (CurrentTime - LastTime1 >= 10) 
  {
    CAN0.sendMsgBuf(0x001, 0, 8, MSGx001);  // mesaj propriu
    delay(1);

    if (rpm > 6000) rpm=0;
    MSGx186[0] = (rpm*10) >> 8;
    MSGx186[1] = (rpm*10) & 0xFF;
    byte status = CAN0.sendMsgBuf(0x186, 0, 7, MSGx186);

    Serial.print("Trimitere 0x186: ");
    if (status == CAN_OK) {
        Serial.println("OK");
    } else {
        Serial.print("EROARE ");
        Serial.println(status);
    }  // rpm motor
    rpm = rpm + 5;
    delay(1);
    
//    CAN0.sendMsgBuf(0x189, 0, 8, MSGx189);
//    delay(1);
//    CAN0.sendMsgBuf(0x18A, 0, 6, MSGx18A);
//    delay(1);
    
    CAN0.sendMsgBuf(0x1F6, 0, 8, MSGx1F6);  // motor pornit
    delay(1);
    
    LastTime1 = CurrentTime; 
  } // end TimeOut = 10 ms


  if (CurrentTime - LastTime2 >= 50) 
  {
//    CAN0.sendMsgBuf(0x217, 0, 8, MSGx217);
//    delay(1);
//    CAN0.sendMsgBuf(0x2A9, 0, 1, MSGx2A9);
//    delay(1);
//    CAN0.sendMsgBuf(0x2C6, 0, 6, MSGx2C6);
//    delay(1);

    CAN0.sendMsgBuf(0x29A, 0, 8, MSGx29A);
    delay(1);
    CAN0.sendMsgBuf(0x29C, 0, 8, MSGx29C);
    delay(1);

    if (vss > 200) vss = 0;
    MSGx354[0] = (vss*100) >> 8 ;
    MSGx354[1] = (vss*100) & 0xFF;
    CAN0.sendMsgBuf(0x354, 0, 8, MSGx354);      // vss
    vss++;
    delay(1);
    
    MSGx5DE[0] = vss;
    CAN0.sendMsgBuf(0x5DE, 0, 8, MSGx5DE);      // leduri
    delay(1);

    LastTime2 = CurrentTime; 
  } // end TimeOut = 50 ms


  if (CurrentTime - LastTime3 >= 100) 
  {
//    CAN0.sendMsgBuf(0x41A, 0, 8, MSGx41A);
//    delay(1);
//    CAN0.sendMsgBuf(0x41D, 0, 4, MSGx41D);
//    delay(1);
//    CAN0.sendMsgBuf(0x500, 0, 5, MSGx500);
//    delay(1);
//    CAN0.sendMsgBuf(0x511, 0, 7, MSGx511);
//    delay(1);
//    
    CAN0.sendMsgBuf(0x5DA, 0, 8, MSGx5DA); // temp motor
    delay(1);
//    
//    CAN0.sendMsgBuf(0x648, 0, 8, MSGx648);
//    delay(1);
//    CAN0.sendMsgBuf(0x65C, 0, 3, MSGx65C);
//    delay(1);
//    CAN0.sendMsgBuf(0x66A, 0, 8, MSGx66A);
//    delay(1);
    LastTime3 = CurrentTime;
  }  // end TimeOut = 100 ms


  if (CurrentTime - LastTime4 >= 500) 
  {
    if (stare==0) 
    {
      digitalWrite(8, HIGH); // LED D8 este aprins
      stare = 1;
    }
    else
    {
      digitalWrite(8, LOW); // LED D8 este stins
      stare = 0;
    }

    Serial.print("RPM = "); 
    Serial.println(rpm);
    
    LastTime4 = CurrentTime;  
  }

} // end loop()
