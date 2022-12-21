#include <SoftwareSerial.h>       //Software Serial library
#include <Wire.h>
SoftwareSerial espSerial(3, 4);   //Pin 3 and 4 act as RX and TX. Connect them to TX and RX of ESP8266      
#define DEBUG true
String mySSID = "ROG Phone 5_1260";       // WiFi SSID
String myPWD = "sm@90700"; // WiFi Password
String myAPI = "50F4X0DRKBI5QH7J";   // API Key
String myHOST = "api.thingspeak.com";
String myPORT = "80";
String myFIELD = "field1";
String myFIELD2 = "field2";
String myFIELD3 = "field3";

#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2


OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;

void setup(){
  Serial.begin(9600); //init serial 9600
  espSerial.begin(9600);
  
  espData("AT+RST", 1000, DEBUG);                      //Reset the ESP8266 module
  espData("AT+CWMODE=1", 1000, DEBUG);                 //Set the ESP mode as station mode
  espData("AT+CWJAP=\""+ mySSID +"\",\""+ myPWD +"\"", 1000, DEBUG);   //Connect to WiFi network
}

void loop(){
  float measurement1 = getTempValue();
  float measurement2 = gettdsValue();
  float measurement3 = getpHValue();
  delay(50);
 // Serial.print("measurement1 = ");
  Serial.println(measurement1);
 //Serial.print("measurement2 = ");
  Serial.println(measurement2);
   // Serial.print("measurement3 = ");
  Serial.println(measurement3);
  
   String sendData = "GET /update?api_key="+ myAPI +"&"+ myFIELD +"="+String(measurement1)+"&"+ myFIELD2 +"="+String(measurement2)+"&"+ myFIELD3 +"="+String(measurement3);
    espData("AT+CIPMUX=1", 1000, DEBUG);       //Allow multiple connections
    espData("AT+CIPSTART=0,\"TCP\",\""+ myHOST +"\","+ myPORT, 1000, DEBUG);
    espData("AT+CIPSEND=0," +String(sendData.length()+4),1000,DEBUG);  
    espSerial.find(">"); 
    espSerial.println(sendData);
}

float getTempValue(){
  delay(10);
  sensors.requestTemperatures();
  float Celsius = sensors.getTempCByIndex(0);
  Serial.print("Temperature of water:");
  Serial.println(Celsius);
    
  return Celsius;
}

float gettdsValue(){
  delay(10);
  float volt;
  float ntu;
  volt = 0;
  for(int i=0; i<800; i++)
  {
      volt += ((float)analogRead(A0)/1023)*5;
  }
  volt = (volt/800)+0.6;
  volt = round_to_dp(volt,2);
  if(volt < 2.5){
      ntu = 3000;
  }else{
      ntu = -1120.4*square(volt)+5742.3*volt-4353.8; 
  }
  Serial.print("Turbidity of water:");
  Serial.println(ntu);
  
  return ntu;
}
float round_to_dp( float in_value, int decimal_place){
  float multiplier = powf( 10.0f, decimal_place );
  in_value = roundf( in_value * multiplier ) /multiplier;
  return in_value;
}
float getpHValue(){
  delay(10);
  float calibration_value = 27.54;
  int phval = 0; 
  unsigned long int avgval; 
  int buffer_arr[10],temp;
   for(int i=0;i<10;i++) 
   { 
   buffer_arr[i]=analogRead(A1);
   delay(30);
   }
   for(int i=0;i<9;i++)
   {
   for(int j=i+1;j<10;j++)
   {
   if(buffer_arr[i]>buffer_arr[j])
   {
   temp=buffer_arr[i];
   buffer_arr[i]=buffer_arr[j];
   buffer_arr[j]=temp;
   }
   }
   }
   avgval=0;
   for(int i=2;i<8;i++)
   avgval+=buffer_arr[i];
   float volt=(float)avgval*5.0/1024/6;
   float ph_act = -5.70 * volt + calibration_value;
   Serial.print("pH of water:");
   Serial.println(ph_act);

  return ph_act;
}


String espData(String command, const int timeout, boolean debug)
{
  Serial.print("AT Command ==> ");
  Serial.print(command);
  Serial.println("     ");
  
  String response = "";
  espSerial.println(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (espSerial.available())
    {
      char c = espSerial.read();
      response += c;
    }
  }
  if (debug)
  {
    //Serial.print(response);
  return response;
  }
}