#include <Wire.h>
#include <PubSubClient.h>

#include <WiFi.h> 
WiFiClient TCP_Client; // Objecto do tipo TCP
PubSubClient client(TCP_Client); // Objecto do tipo cliente MQTT

#define    MPU9250_ADDRESS            0x68
#define    ACC_FULL_SCALE_2_G        0x00  
#define    ACC_FULL_SCALE_4_G        0x08
#define    ACC_FULL_SCALE_8_G        0x10
#define    ACC_FULL_SCALE_16_G       0x18


int first_time;
int16_t aceleracao_antiga;
int16_t aceleracao_atual;

volatile bool intFlag=false;

volatile int count;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;


void IRAM_ATTR onTime() {
   portENTER_CRITICAL_ISR(&timerMux);
   count++;
   portEXIT_CRITICAL_ISR(&timerMux);

   intFlag=true;
  digitalWrite(13, digitalRead(13) ^ 1);
}


// This function read Nbytes bytes from I2C device at address Address. 
// Put read bytes starting at register Register in the Data array. 
void I2Cread(uint8_t Address, uint8_t Register, uint8_t Nbytes, uint8_t* Data)
{
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.endTransmission();
  
  // Read Nbytes
  Wire.requestFrom(Address, Nbytes); 
  uint8_t index=0;
  while (Wire.available())
    Data[index++]=Wire.read();
}


// Write a byte (Data) in device (Address) at register (Register)
void I2CwriteByte(uint8_t Address, uint8_t Register, uint8_t Data)
{
  // Set register address
  Wire.beginTransmission(Address);
  Wire.write(Register);
  Wire.write(Data);
  Wire.endTransmission();
}



// Initial time
long int ti;
//volatile bool intFlag=false;

// Initializations
void setup()
{
Serial.begin(115200);
Serial.println(); Serial.print("Connecting to ");Serial.println("Cabovisao-00A0");
 WiFi.mode(WIFI_STA);
 //WiFi.begin("ZON-5590", "casacoelho2020");
 WiFi.begin("Cabovisao-00A0", "tcpipemredesmicrosoft");
 while (WiFi.status() != WL_CONNECTED) {
 delay(500); Serial.print(".");
 }

 // MQTT
 client.setServer("192.168.1.15", 1883); // O Servidor MQTT, Broker, reside no pc “192.168.1.2”
 
 client.connect("esp32"); // O ESP regista-se no Broker, com o nome “esp8266”
 Serial.println("Subscribe temperatura: ");


  
  // Arduino initializations
  Wire.begin();
  

    timer = timerBegin(0, 120, true);                
   timerAttachInterrupt(timer, &onTime, true);    
    
   // Sets an alarm to sound every second
   timerAlarmWrite(timer, 1000000, true);           
   timerAlarmEnable(timer);

  
  // Set accelerometers low pass filter at 5Hz
  I2CwriteByte(MPU9250_ADDRESS,29,0x06);
  // Set gyroscope low pass filter at 5Hz
  I2CwriteByte(MPU9250_ADDRESS,26,0x06);
 
  
  // Configure gyroscope range

  // Configure accelerometers range
  I2CwriteByte(MPU9250_ADDRESS,28,ACC_FULL_SCALE_16_G);
  // Set by pass mode for the magnetometers
  I2CwriteByte(MPU9250_ADDRESS,0x37,0x02);
  

  
   pinMode(13, OUTPUT);

  
  
  // Store initial time
  ti=millis();


first_time=1;

}





// Counter
long int cpt=0;

void callback()
{ 
  intFlag=true;
  digitalWrite(13, digitalRead(13) ^ 1);
}



// Main loop, read and display data
void loop(){
  while (!intFlag);
  intFlag=false;
  
  // Display time
  Serial.print (millis()-ti,DEC);
  Serial.print ("\t");

  
 
  // ____________________________________
  // :::  accelerometer and gyroscope ::: 

  // Read accelerometer and gyroscope
  uint8_t Buf[14];
  I2Cread(MPU9250_ADDRESS,0x3B,14,Buf);
  
  // Create 16 bits values from 8 bits data
  
  // Accelerometer
  int16_t ax=-(Buf[0]<<8 | Buf[1]);
  int16_t ay=-(Buf[2]<<8 | Buf[3]);
  int16_t az=Buf[4]<<8 | Buf[5];

 
  // Display values
  
  // Accelerometer
  //Serial.print (ax,DEC); 
  //Serial.print ("\t");
  //Serial.print (ay,DEC);
  //Serial.print ("\t");
  //Serial.print (az,DEC);  
  //Serial.print ("\t");

aceleracao_atual=sqrt(pow(ax,2)+pow(ay,2)+pow(az,2));
Serial.print(aceleracao_atual);


if (abs(aceleracao_atual-aceleracao_antiga)>0 | first_time==1){
  String Ac_X;
  String Ac_Y;
  String Ac_Z;
Ac_X = String(ax);   //Converts integer to string
Ac_Y = String(ay);
Ac_Z = String(az);
//client.publish("aceleracao_x", ax.c_str()); // Publica no Broker o tópico “temperatura” com o valor “111”
client.publish("aceleracao_x", Ac_X.c_str());
client.publish("aceleracao_y", Ac_Y.c_str());
client.publish("aceleracao_z", Ac_Z.c_str());}

aceleracao_antiga=aceleracao_atual;
first_time=0;  
  // End of line
  Serial.println("");
//  delay(100);    
}
