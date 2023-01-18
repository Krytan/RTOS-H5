#include <Arduino.h>
#include <Arduino_FreeRTOS.h>

void Task1( void *pvParameters );

void Task2( void *pvParameters );

void Task3( void *pvParameters );

void setup() {

  // initialize serial communication at 9600 bits per second:
  pinMode(8, OUTPUT);
  pinMode(2, INPUT);
  Serial.begin(9600);  
  xTaskCreate(Task3,"task3",128,NULL,1,NULL);

vTaskStartScheduler();

}

// Har ikke bruge for loopen
void loop()
{

}

void Task2(void *pvParameters)  {

    Serial.println("Task2");

    digitalWrite(8, HIGH);   

    vTaskDelay( 500 / portTICK_PERIOD_MS ); 

    digitalWrite(8, LOW);    

    vTaskDelay( 500 / portTICK_PERIOD_MS ); 

}

void Task1(void *pvParameters)  

{
      Serial.println("Task1");
       digitalWrite(8, HIGH);
        vTaskDelay( 333 / portTICK_PERIOD_MS ); 
        // Turn off the LED and delay for 777ms
        digitalWrite(8, LOW);
        vTaskDelay( 777 / portTICK_PERIOD_MS ); 

}

void Task3(void *pvParameters)  {

  for(;;)
  {
    if (digitalRead(2) == HIGH)
    {
      xTaskCreate(Task1,"task1",128,NULL,1,NULL);
    }else
    {
      xTaskCreate(Task2,"task2",128,NULL,1,NULL);
    };
    
  }

}
