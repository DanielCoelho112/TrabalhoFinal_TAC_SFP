#include <Wire.h>
#include <PubSubClient.h>

#include <WiFi.h> 
WiFiClient TCP_Client; // Objecto do tipo TCP
PubSubClient client(TCP_Client); // Objecto do tipo cliente MQTT


//All this values were extracted from the mp9250 datasheet
//url: https://invensense.tdk.com/wp-content/uploads/2015/02/PS-MPU-9250A-01-v1.1.pdf


//PINS:
// ESP32 TO GY91
//3V3 TO VIN
//GND TO GND
//D21 TO SDA
//D22 TO SCL



#define    MPU9250_ADDRESS            0x68
#define    ACC_FULL_SCALE_2_G        0x00  
#define    ACC_FULL_SCALE_4_G        0x08
#define    ACC_FULL_SCALE_8_G        0x10
#define    ACC_FULL_SCALE_16_G       0x18




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

  // baudrate of serial 
Serial.begin(115200);
Serial.println(); Serial.print("Connecting to ");Serial.println("ZON-5590");
 WiFi.mode(WIFI_STA);
 
 // establishment wifi connection
 WiFi.begin("ZON-5590", "casacoelho2020");
 //WiFi.begin("Cabovisao-00A0", "tcpipemredesmicrosoft");
 while (WiFi.status() != WL_CONNECTED) {
 delay(500); Serial.print(".");
 }

 // MQTT in Raspberry  (CHECK THIS IP)
 client.setServer("192.168.1.10", 1883); // O Servidor MQTT, Broker, reside no pc “192.168.1.2”
 
 client.connect("esp32"); // O ESP regista-se no Broker, com o nome “esp8266”
 Serial.println("Publishing Accelerations: ");


  
  // Arduino initializations
  Wire.begin();
  

// defining timer
    timer = timerBegin(0, 120, true);                
   timerAttachInterrupt(timer, &onTime, true);    
    
   // Sets an alarm to sound every second
   timerAlarmWrite(timer, 1000000, true);           
   timerAlarmEnable(timer);

  
  // Set accelerometers low pass filter at 5Hz, Saw this in a forum 
  I2CwriteByte(MPU9250_ADDRESS,29,0x06);

 
  

  // Configure accelerometers range  
  I2CwriteByte(MPU9250_ADDRESS,28,ACC_FULL_SCALE_16_G); // 16 was chosen for trial and error, it was the most stable

 
   pinMode(13, OUTPUT);
 
  // Store initial time
  ti=millis();



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

  
 
  // ------------------------------------
  // accelerometer

  // Read accelerometer
  uint8_t Buf[14];
  I2Cread(MPU9250_ADDRESS,0x3B,14,Buf);
  
  // Create 16 bits values from 8 bits data

  // Extract the 3 accelerations from the Buf
  
  // Accelerometer
  int16_t ax=-(Buf[0]<<8 | Buf[1]);
  int16_t ay=-(Buf[2]<<8 | Buf[3]);
  int16_t az=Buf[4]<<8 | Buf[5];


 // Sending accelerations in MQTT topics

  
  String Ac_X;
  String Ac_Y;
  String Ac_Z;
Ac_X = String(ax);   //Converts integer to string
Ac_Y = String(ay);
Ac_Z = String(az);
//client.publish("aceleracao_x", ax.c_str()); // Publica no Broker o tópico “temperatura” com o valor “111”
client.publish("aceleracao_x", Ac_X.c_str());
client.publish("aceleracao_y", Ac_Y.c_str());
client.publish("aceleracao_z", Ac_Z.c_str());

Serial.print(Ac_X.c_str());
Serial.print(" ");
Serial.print(Ac_Y.c_str());
Serial.print(" ");
Serial.print(Ac_Z.c_str());
  
  // End of line
  Serial.println("");
//  delay(100);    
}
