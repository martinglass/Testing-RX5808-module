/*
  Testing RX5804 receiver module

  Turns an LED on if something went wrong. (Arduino Leonardo)
  Data in registers and RSSI value you can see on Serial monitor.

  created 03.01.2022 
  by Vlad Lesnov

  This example code is in the public domain.
*/

#define CS0_SPIDATA_PIN   4
#define CS1_SPILE_PIN     3
#define CS2_SPICLK_PIN    2
#define RSSI_OUT_PIN      0

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
  Serial.begin(9600);
}

void loop() {
  static uint8_t fsets=0;

  float value = (analogRead(RSSI_OUT_PIN)*5.0)/1023;
  if(fsets==0){
    Serial.print(String(FSETS - 1) + "\t" + String(value));
  }
  else{
    Serial.print(String(fsets - 1) + "\t" + String(value));
  }
      
   for(uint8_t i=0;i<=0x0a;i++){
      uint32_t temp = rx5808(i,READ_REG,0);
      Serial.print("\t0x");
      Serial.print(temp,HEX);  
   }
   uint32_t temp = rx5808(0x0F,READ_REG,0);
   Serial.print("\t0x");
   Serial.println(temp,HEX);  
        
  uint32_t regData=0;
  regData= (((uint32_t)N[fsets])<<7) | A[fsets];
  rx5808(0x01,WRITE_REG,regData);
  delay(40);                         //important!!!
  if(rx5808(0x01,READ_REG,0)!= regData) digitalWrite(LED_BUILTIN,HIGH);
  else                                  digitalWrite(LED_BUILTIN,LOW);
  
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
