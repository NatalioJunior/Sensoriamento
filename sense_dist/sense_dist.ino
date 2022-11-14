//import the library in the sketch
#include <SharpIR.h>

#define PI 3.1415926535897932384626433832795

//Create a new instance of the library
//Call the sensor "sensor"
//The model of the sensor is "GP2YA41SK0F"
//The sensor output pin is attached to the pin A0
SharpIR sensor( SharpIR::GP2Y0A21YK0F, A0 );

void setup()
{
  Serial.begin( 9600 ); //Enable the serial comunication
}

void loop()
{
  int distance = sensor.getDistance(); //Calculate the distance in centimeters and store the value in a variable
  float volume = ((60-distance) * 25 * PI)/1000;

  Serial.print("Distancia: ");
  Serial.print( distance); //Print the value to the serial monitor
  Serial.println("cm");
  Serial.print("Volume: ");
  Serial.print(volume); //Print the value to the serial monitor
  Serial.println("L");
  Serial.println("");
  delay(500);
}
