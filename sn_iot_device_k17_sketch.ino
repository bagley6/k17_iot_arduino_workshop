#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ThingerWifi.h>
#include <Servo.h>

#define ssid  "Grant_Corte2"
#define password "aviva_Grant"

ThingerWifi thing("granthulbert", "esp8266_12e_01", "&hPf3tv03KHT");
Servo myservo;  // create servo object to control a servo
#define ANGLEFLAT 81
#define ANGLELEFT 66
#define ANGLERIGHT 96
#define EIOT_CLOUD_ADDRESS     "hulbertfamily.com"
#define EIOT_CLOUD_PORT        80

int currentServo = ANGLEFLAT;

void slowServo( int newAngle ) {
  myservo.attach(D1);
  delay(100);
  if( newAngle > currentServo ) {
    for( int i=currentServo; i<=newAngle; i++  ) {
      myservo.write( i );
      delay(100);
    }
  } else {
    for( int i=currentServo; i>=newAngle; i-- ) {
      myservo.write( i );
      delay(100);
    }
  }
  myservo.detach();
  currentServo = newAngle;
}

void connectToWiFi() {
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(A0, INPUT); // analog moisture sensor
  pinMode(D2, INPUT); // digital moisture sensor
  pinMode(D3, INPUT);
  pinMode(D0, OUTPUT);
  myservo.attach(D1);  // attaches the servo on pin D1 to the servo object
  delay(100);
  myservo.write(81);  // 81 degrees is midpoint
  delay(100);
  myservo.detach();

  connectToWiFi();

//  thing.add_wifi("NETGEAR38", "fuzzyship377");
  thing.add_wifi("Grant_Corte2", "aviva_Grant");
//  thing.add_wifi("WHITEBOAT", "yitbos22");
// resource input example (i.e. turning on/off a light, a relay, configuring a parameter, etc)
  thing["led"] << [](pson& in){
    digitalWrite(BUILTIN_LED, in ? LOW : HIGH);
  };

  thing["buzzer"] << [](pson& in){
    digitalWrite(D0, in ? HIGH : LOW);
  };

    // digital moisture
    thing["digitalMoisture"] >> [](pson & out){
        out = (unsigned int) digitalRead(D2);
    };

    // LDR resource
    thing["LDR"] >> [](pson & out){
        out = (unsigned int) analogRead(A0);
    };

    // D3 digital input (hoping analog)
    thing["D3"] >> [](pson & out){
        out = (unsigned int) analogRead(D3);
    };

  thing["servo"] << [](pson& in){
    String url = "/robots.txt";
    WiFiClient client;
    if (!client.connect(EIOT_CLOUD_ADDRESS, EIOT_CLOUD_PORT)) {
      Serial.println("connection failed");
      return;
    }
    Serial.println(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + EIOT_CLOUD_ADDRESS + "\r\n" + "Connection: close\r\n\r\n");
    client.print(String("GET ") + url + " HTTP/1.1\r\n" + "Host: " + EIOT_CLOUD_ADDRESS + "\r\n" + "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

    client.print(String("POST ") + url + " HTTP/1.1\r\n" +
      "Host: " + String(EIOT_CLOUD_ADDRESS) + "\r\n" + 
      "Connection: close\r\n" + 
      "Content-Length: 0\r\n" + 
      "\r\n");
    delay(100);
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }

//    myservo.attach(D1);
    if( in ) {
        slowServo(ANGLELEFT);
//      myservo.write(66);
//      delay(100);
    } else {
        slowServo(ANGLERIGHT);
//      myservo.write(96);
//      delay(100);
    }
//    myservo.detach();
//    myservo.attach(D1);
//    myservo.write( (long)in["degrees"] );
//    delay(100);
//    myservo.detach();
  };

  // resource output example (i.e. reading a sensor value)
  thing["millis"] >> [](pson& out){ out = millis(); };

  // resource input/output example (i.e. passing input values and do some calculations)
  thing["in_out"] = [](pson& in, pson& out){
      out["sum"] = (long)in["value1"] + (long)in["value2"];
      out["mult"] = (long)in["value1"] * (long)in["value2"];
  };
}

void loop() {
  thing.handle();
//  Serial.print( analogRead(A0) );
}
