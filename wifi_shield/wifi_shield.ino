#include <WiFi.h>
#include <SPI.h>

char ssid[] = "";     //  your network SSID (name)
char pass[] = "";         // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

WiFiClient client;

char host[] = "smarteye-api.herokuapp.com";
String path="/measurements";

float calibration = 21.34 - 0.09; //change this value to calibrate
const int analogInPin = A0;
int sensorValue = 0;
unsigned long int avgValue;
float b;
int buf[10],temp;

boolean request = false;

void setup() {

  // initialize serial:

  Serial.begin(9600);

  // attempt to connect using WPA2 encryption:

  Serial.println("Attempting to connect to WPA network...");

  status = WiFi.begin(ssid, pass);

  // if you're not connected, stop here:

  if ( status != WL_CONNECTED) {

    Serial.println("Couldn't get a wifi connection");

    while(true);

  }

  // if you are connected, print out info about the connection:

  else {

    Serial.println("Connected to network");

  }
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0;i<10;i++) {
    buf[i]=analogRead(analogInPin);
    delay(30);
  }
  
  for(int i=0;i<9;i++) {
    for(int j=i+1;j<10;j++) {
      if(buf[i]>buf[j]) {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++) 
    avgValue+=buf[i];

  if (!request){
    float pHVol=(float)avgValue*5.0/1024/6;
    float phValue = -5.70 * pHVol + calibration;
    Serial.print("pH = ");
    Serial.println(phValue);
    Serial.print("volt = ");
    Serial.println(pHVol);
    String queryString = String("?station_id=12") + String("&property_id=1") + String("&value=") + String(phValue) + String("&collected_at=10/08/2022");

    if (client.connect(host, 80)) {   
      // Make a HTTP request:
      // send HTTP header
      client.println("POST " + path + queryString + " HTTP/1.1");
      client.println("Host: " + String(host));
      client.println("Connection: close");
      client.println(); // end HTTP header
      // send HTTP body
      //client.println(queryString);
      
      while(client.available()) {
        // read an incoming byte from the server and print them to serial monitor:
        char c = client.read();
        Serial.print(c);
      }

      if(!client.connected()) {
        // if the server's disconnected, stop the client:
        Serial.println("disconnected");
        client.stop();
      }
      client.println();
    }
    request = true;
  }
  delay(500);
}
