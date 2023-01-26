
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <Tasks.h>
#include <Arduino_FreeRTOS.h>

void WaitforClient( void *pvParameters);
//The code is divided into two parts, 
//the first part is the code that handles the temperature sensor 
//the second part is the code that handles the web server.

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};

IPAddress ip(192,168,0,238);

EthernetServer server(80);

void setup() {

  Serial.begin(9600);
  while (!Serial) {};
  Serial.println("Ethernet WebServer Example");
  pinMode(2, OUTPUT);

  Ethernet.begin(mac,ip);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF) 
    Serial.println("Ethernet cable is not connected.");

  setupThermometer();

  // start the server
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  xTaskCreate(WaitforClient,"WaitforClient",1024,NULL,1,NULL);
  vTaskStartScheduler();
  
}

void loop() {
}

void WaitforClient(void *pvParameters){
  while(1)
  {
  EthernetClient client = server.available();
  
  if (client) 
    handleClientRequest(client);
  };

}
