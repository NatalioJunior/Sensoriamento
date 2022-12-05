#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>
 
#define DS18B20 7

OneWire ourWire(DS18B20);
DallasTemperature sensors(&ourWire);

char ssid[] = "";     //  your network SSID (name)
char pass[] = "";         // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

unsigned int localPort = 2390;      // local port to listen for UDP packets

IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

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

String now_time;

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
  Udp.begin(localPort);
}

void loop() {
  
  wifiUdpNtpClient();
  
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
    String queryString1 = String("?station_id=12") + String("&property_id=1") + String("&value=") + String(phValue) + String("&collected_at=") + (now_time);
    delay(2500);

    sensors.requestTemperatures();
    Serial.print("Temperatura: ");
    Serial.print(sensors.getTempCByIndex(0));
    Serial.println("*C");
    float temp = sensors.getTempCByIndex(0);
    String queryString2 = String("?station_id=12") + String("&property_id=4") + String("&value=") + String(temp) + String("&collected_at=") + (now_time);
    delay(2500);

    if (client.connect(host, 80)) {   
      // Make a HTTP request:
      // send HTTP header
      client.println("POST " + path + queryString1 + " HTTP/1.1");
      client.println("Host: " + String(host));
      client.println("Connection: close");
      client.println(); // end HTTP header
      // send HTTP body
      client.println(queryString1);
      
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
    
    if (client.connect(host, 80)) {   
      // Make a HTTP request:
      // send HTTP header
      client.println("POST " + path + queryString2 + " HTTP/1.1");
      client.println("Host: " + String(host));
      client.println("Connection: close");
      client.println(); // end HTTP header
      // send HTTP body
      client.println(queryString2);
      
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

void wifiUdpNtpClient(){
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, extract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);


    // print the hour, minute and second:
    Serial.print("The UTC time is ");       // UTC is the time at Greenwich Meridian (GMT)
    Serial.print((epoch  % 86400L) / 3600); // print the hour (86400 equals secs per day)
    Serial.print(':');
    now_time = String((epoch  % 86400L) / 3600) + ':';
    if (((epoch % 3600) / 60) < 10) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
      now_time = now_time + '0';
    }
    Serial.print((epoch  % 3600) / 60); // print the minute (3600 equals secs per minute)
    Serial.print(':');
    now_time = now_time + String((epoch  % 3600) / 60) + ':';
    if ((epoch % 60) < 10) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
      now_time = now_time + '0';
    }
    Serial.println(epoch % 60); // print the second
    now_time = now_time + String(epoch % 60);
  }
  // wait ten seconds before asking for the time again
  delay(10000);
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}
