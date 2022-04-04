/*
  Testing RX5808 receiver module

  Data in registers and RSSI value you can see on Serial monitor.

  created 03.01.2022 
  by Vlad Lesnov
  https://github.com/VladLesnov/Testing-RX5808-module
  This example code is in the public domain.

  Updated by Martin Glass on 31.03.2022
*/

/*
#define CS0_SPIDATA_PIN   4
#define CS1_SPILE_PIN     3
#define CS2_SPICLK_PIN    2
#define RSSI_OUT_PIN      0

// STM32 Black Pill
#define CS0_SPIDATA_PIN   PB4
#define CS1_SPILE_PIN     PB7
#define CS2_SPICLK_PIN    PB3
#define RSSI_OUT_PIN      PA0

// Arduino Nano
#define CS0_SPIDATA_PIN   11
#define CS1_SPILE_PIN     10
#define CS2_SPICLK_PIN    13
#define RSSI_OUT_PIN      A0
*/

// Arduino Nano
#define CS0_SPIDATA_PIN   11
#define CS1_SPILE_PIN     10
#define CS2_SPICLK_PIN    13
#define RSSI_OUT_PIN      A0

#define VREF 5.0 //Arduino
//#define VREF 3.3 //STM32 

#define TSPI 10 //uS
#define READ_REG  0
#define WRITE_REG 1
#define IF 480  //MHZ
#define FRQLOW 5704

#define FSETS 16
//FSTEP=16MHZ IF=480MHZ dF=18MHZ NDF4010
uint16_t FRQ[FSETS] = {FRQLOW,  5720,  5736,  5752,  5768,  5784,  5800,  5816,  5832,  5848,  5864,  5880,  5896,  5912,  5928,  5944}; //MHZ
uint16_t N[FSETS]=    {81,    81,    82,    82,    82,    82,    83,    83,    83,    83,    84,    84,    84,    84,    85,    85};
uint8_t  A[FSETS]=    {20,    28,     4,    12,    20,    28,     4,    12,    20,    28,     4,    12,    20,    28,     4,    12};
uint32_t REG01[FSETS]={0x2894,0x289C,0x2904,0x290C,0x2914,0x291C,0x2984,0x298C,0x2994,0x299C,0x2A04,0x2A0C,0x2A14,0x2A1C,0x2A84,0x2A8C};



// raddr{0x00-0x0A,0x0B~0x0E:Reserved,0x0F}; rw{write cycle - 1,read cycle - 0};
int32_t rx5808(uint8_t raddr,uint8_t rw,uint32_t data);
int32_t Reg01RX5808(uint16_t freq); //freq - MHZ

void setup() {

  pinMode(CS2_SPICLK_PIN, OUTPUT);
  pinMode(CS1_SPILE_PIN, OUTPUT);
  pinMode(CS0_SPIDATA_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  digitalWrite(CS1_SPILE_PIN,HIGH);
  Serial.begin(115200);
}

void loop() {
  static uint8_t fsets=0;

  Serial.print("fsets: "+String(fsets) + "\t");

  uint32_t regData=0;
  regData= Reg01RX5808(FRQ[fsets]);
  rx5808(0x01,WRITE_REG,regData);
  delay(40);                         //important!!!
  if(rx5808(0x01,READ_REG,0)!= regData) digitalWrite(LED_BUILTIN,HIGH);
  else                                  digitalWrite(LED_BUILTIN,LOW);

//Print the current Frequency
//  Serial.print(Reg01RX5808(FRQ[fsets]),HEX);
  Serial.print("FREQ: " + String(FRQ[fsets]) + " ");


// Read & Printout current RSSI Value
  int v=analogRead(RSSI_OUT_PIN);
  float value = (v*VREF)/1023;
    Serial.print("RSSI: " + String(v) + " (" + String(value) + "V)      Reg");

// Read & Printout Each Register loop (register 0-A & F)
   for(uint8_t i=0;i<=0x0a;i++){                
      uint32_t temp = rx5808(i,READ_REG,0);
      Serial.print("\t"+String(i)+": 0x");
      Serial.print(temp,HEX);  // Print Reg value
   }
   uint32_t temp = rx5808(0x0F,READ_REG,0);
   Serial.print("\tSTATUS: 0x");
   Serial.println(temp,HEX);  // Print Status
//End of Register loop
        
  ++fsets;
  if(fsets>=FSETS) fsets=0;
  delay(1000);
}

int32_t rx5808(uint8_t raddr,uint8_t rw,uint32_t data){
  uint32_t temp=(((uint32_t)data)<<5) | ((rw&0x01)<<4) | (raddr&0x0F);
  digitalWrite(CS1_SPILE_PIN,LOW);
  
  //send reg addres
  pinMode(CS0_SPIDATA_PIN, OUTPUT);
  for(uint8_t i=0;i<5;i++){
    
    if(bitRead(temp,i)) digitalWrite(CS0_SPIDATA_PIN,HIGH);
    else                digitalWrite(CS0_SPIDATA_PIN,LOW);
    delayMicroseconds(TSPI);
    digitalWrite(CS2_SPICLK_PIN,HIGH);
    delayMicroseconds(TSPI);
    digitalWrite(CS2_SPICLK_PIN,LOW);
    delayMicroseconds(TSPI);
  }
  //send  data
  if(rw){
    for(uint8_t i=5;i<25;i++){
      if(bitRead(temp,i)) digitalWrite(CS0_SPIDATA_PIN,HIGH);
      else                digitalWrite(CS0_SPIDATA_PIN,LOW);
      delayMicroseconds(TSPI);
      digitalWrite(CS2_SPICLK_PIN,HIGH);
      delayMicroseconds(TSPI);
      digitalWrite(CS2_SPICLK_PIN,LOW);
      delayMicroseconds(TSPI);
    }    
  }
  else{ //receive data
    pinMode(CS0_SPIDATA_PIN, INPUT_PULLUP);
    for(uint8_t i=5;i<25;i++){
      delayMicroseconds(TSPI);
      if(digitalRead(CS0_SPIDATA_PIN)) bitWrite(temp,i,1);
      else                             bitWrite(temp,i,0);       
      digitalWrite(CS2_SPICLK_PIN,HIGH);
      delayMicroseconds(TSPI);
      digitalWrite(CS2_SPICLK_PIN,LOW);
      delayMicroseconds(TSPI);
    }        
  }
  
  digitalWrite(CS1_SPILE_PIN,HIGH);
  delayMicroseconds(TSPI);
  
  return (temp>>5);
}


int32_t Reg01RX5808(uint16_t freq){ //freq - MHZ
    if(freq<FRQLOW) return -1;
    uint32_t n=(freq-IF)/2/32;
    uint8_t  a=((freq-IF)/2)%32;
    return ((n<<7) | a);  
}
