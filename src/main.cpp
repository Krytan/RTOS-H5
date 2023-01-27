/// This code includes the necessary libraries for a program written for an Arduino board.
/// The SPI.h library provides an interface for the Serial Peripheral Interface (SPI) bus, which is used to communicate with peripheral devices.
/// The Ethernet.h library provides functions for communicating over Ethernet networks. The Wire.h library provides functions for communicating over I2C buses.
/// The Tasks.h library provides functions for creating and managing tasks in a FreeRTOS environment.
/// Finally, the Arduino_FreeRTOS.h library provides an API to access FreeRTOS features on Arduino boards.
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <Tasks.h>
#include <Arduino_FreeRTOS.h>


//The code is divided into two parts,
//the first part is the code that handles the temperature sensor
//the second part is the code that handles the web server.
void WaitforClient( void *pvParameters);
/// This code creates an array of bytes called mac with the values 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, and 0xEE. The values in the array represent a MAC address.
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEE
};


/// iP Address for arduino
IPAddress ip(192,168,0,238);




/// listen for incoming connections on port 80.
EthernetServer server(80);




/// This code is part of the setup() function for an Ethernet WebServer example.
/// It sets up a serial connection with a baud rate of 9600, sets pin 2 as an output, and initializes the Ethernet connection with the given mac address and IP address.
/// It then checks to make sure that the Ethernet shield is connected and that the Ethernet cable is connected.
/// It calls a setupThermometer() function, starts the server, prints out the local IP address, and creates a task called WaitforClient.
void setup() {


  Serial.begin(9600);
  while (!Serial) {};
  Serial.println("Smarthome webserver:");
  pinMode(2, OUTPUT);


  Ethernet.begin(mac,ip);


  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found. Pls check your hardware");
    while (true) {
      delay(1);
    }
  }
  if (Ethernet.linkStatus() == LinkOFF)
    Serial.println("No internet found");


  setupThermometer();


  // start the server
  server.begin();
  Serial.print("server is at: ");
  Serial.println(Ethernet.localIP());
  xTaskCreate(WaitforClient,"WaitforClient",1024,NULL,1,NULL);
  vTaskStartScheduler();
 
}




/// This code is part of a program that creates a server to handle client requests.
/// The void loop() function is an empty loop that will be used to run the program.
/// The WaitforClient() function is a loop that will continuously check for any available clients and then call the handleClientRequest() function to handle the request.
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
