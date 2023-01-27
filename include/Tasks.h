/// DS1621_ADR: This macro defines the address of the DS1621 temperature sensor.
/// DEFAULT_TEMP: This macro defines the default temperature value.
/// HUMIDITY: This macro defines the humidity value.
/// REFRESH_RATE: This macro defines the refresh rate for the temperature sensor.
#define DS1621_ADR 0x48
#define DEFAULT_TEMP 27
#define HUMIDITY 223
#define REFRESH_RATE 5


/// SetTemp() is a function that sets the wanted temperature of a device.
/// It takes an integer argument, wanted_temp, which is the desired temperature in Celsius.
/// The function then converts the temperature to two bytes, th_msb and th_lsb, and assigns them to the device's high temperature register.
/// It also assigns tl_msb and tl_lsb to the device's low temperature register, which is set one degree lower than the desired temperature.
void SetTemp(int16_t wanted_temp)
{
  byte th_msb = byte(wanted_temp);
  byte th_lsb = B00000000;
  byte tl_msb = byte(wanted_temp - 1);
  byte tl_lsb = B00000000;


  /// This code is used to set the temperature threshold of a DS1621 temperature sensor.
  /// The Wire library is used to communicate with the sensor over I2C.
  /// The first line begins a transmission to the DS1621 sensor, using its address (DS1621_ADR).
  /// The next three lines write the temperature threshold values (th_msb and th_lsb) to the sensor.
  /// Finally, the transmission is ended and a 10 millisecond delay is added for good measure.
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xA1);
  /
      Wire.write(th_msb);
  Wire.write(th_lsb);
  Wire.endTransmission();
  delay(10);


  /// This code is used to communicate with a DS1621 temperature sensor.
  /// The first line begins a transmission to the address of the DS1621, followed by three lines of data written to the sensor.
  /// The first line is the command 0xA2 which sets the temperature low limit register.
  /// The second and third lines are the most significant byte (tl_msb) and least significant byte (tl_lsb) of the temperature limit value, respectively.
  /// Finally, the transmission ended.
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xA2);
  Wire.write(tl_msb);
  Wire.write(tl_lsb);
  Wire.endTransmission();
}


/// This function sets up the thermometer to use the DS1621 chip.
/// It begins by setting up a communication with the chip using Wire.begin() and Wire.beginTransmission().
/// It then sends a command to set the config address and set continuous conversion.
/// After that, it sends another command to set EE, followed by setting the wanted temperature to a default temperature using SetTemp().
/// Finally, it delays for 10 milliseconds.
void setupThermometer()
{
  Wire.begin();
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xAC); // Set config address
  Wire.write(0);    // Set continuous conversion
  Wire.endTransmission();
  delay(10);


  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xEE);
  Wire.endTransmission();


  SetTemp(DEFAULT_TEMP);
}


/// This function takes in two 8-bit unsigned integers, msb and lsb, and returns a 16-bit signed integer.
/// It first shifts the most significant byte (msb) to the left by one bit and combines it with the least significant byte (lsb) shifted to the right by 7 bits.
/// The resulting 16-bit number is then multiplied by 10 and divided by 2 before being returned.
int16_t msbAndLsbToInt(uint8_t msb, uint8_t lsb)
{
  int16_t raw_int = (int8_t)msb << 1 | lsb >> 7;
  raw_int = raw_int * 10 / 2;


  return raw_int;
}


/// This function is used to get the temperature high (TH) value from a DS1621 temperature sensor.
/// It begins by sending a transmission to the DS1621 address with the 0xA1 command.
/// It then requests two bytes of data from the address, which it reads into two variables, t_msb and t_lsb.
/// The msbAndLsbToInt() function is then used to convert these two bytes into an int16_t value, which is returned as the TH value.
int16_t getTH()
{
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xA1);
  Wire.endTransmission(false);
  Wire.requestFrom(DS1621_ADR, 2);
  uint8_t t_msb = Wire.read();
  uint8_t t_lsb = Wire.read();
  int16_t th = msbAndLsbToInt(t_msb, t_lsb);


  return th;
}


/// This function sets the wanted temperature.
/// It returns an integer value of type int16_t, which is obtained from the getTH() function.
int16_t SetTemp()
{
  return getTH();
}


/// This function updates the wanted temperature by either increasing or decreasing it by 10 degrees.
/// It takes in an integer parameter 'turnUp' which is either 1 or 0. If it is 1, the temperature will be increased by 10 degrees, and if it is 0, the temperature will be decreased by 10 degrees.
/// The newWantedTemp variable stores the new wanted temperature which is calculated by calling the SetTemp() function.
/// The newWantedTemp variable is then divided by 10 and set as the new wanted temperature using the SetTemp() function again.
void UpdateTemp(int turnUp)
{
  int16_t newWantedTemp = SetTemp();


  if (turnUp == 1)
    newWantedTemp = newWantedTemp + 10;
  else
    newWantedTemp = newWantedTemp - 10;


  newWantedTemp = newWantedTemp / 10;


  SetTemp(newWantedTemp);
}


/// This function is used to end a client connection over Ethernet.
/// It takes an EthernetClient object as an argument and first delays for one millisecond.
/// Then, it calls the stop() method on the client object to disconnect it from the server.
/// Finally, it prints a message to the serial monitor indicating that the client has been disconnected.
void endClientConnection(EthernetClient client)
{
  delay(1);
  client.stop();
  Serial.println("client disconnected");
}


/// This function handles a heat request from an Ethernet client. It takes in an Ethernet client and a char parameter as arguments.
/// The char parameter is converted to an int and used to update the temperature. The client is then sent an HTTP/1.1 200 OK response,
/// followed by the end of the client connection.
void handleHeatRequest(EthernetClient client, char turnUpHeatParameter)
{
  int turnUpHeat = turnUpHeatParameter - '0';


  UpdateTemp(turnUpHeat);
  client.println("HTTP/1.1 200 OK");
  endClientConnection(client);
}


/// This function retrieves the temperature from a DS1621 temperature sensor using the I2C protocol.
/// The DS1621_ADR constant is used to identify the address of the device.
/// The Wire library is used to begin a transmission, write 0xAA to the device, end the transmission without releasing the bus, and request two bytes from the device.
/// The two bytes are read and converted into an int16_t value using msbAndLsbToInt() before being returned.
int16_t GetTemp()
{
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xAA);
  Wire.endTransmission(false);
  Wire.requestFrom(DS1621_ADR, 2);
  uint8_t t_msb = Wire.read();
  uint8_t t_lsb = Wire.read();
  int16_t temp = msbAndLsbToInt(t_msb, t_lsb);


  return temp;
}


/// This function takes in an integer representing a temperature in Celsius and returns a string with the temperature formatted as a string.
/// If the temperature is less than 0, the absolute value of the temperature is taken and the string is formatted with a negative sign.
/// If the temperature is greater than or equal to 1000, it is formatted with three digits before the decimal point. Otherwise, it is formatted with two digits before the decimal point and one digit after.
String formatTemperature(int16_t temp)
{
  char tempBuffer[8];


  if (temp < 0)
  {                   // if temperature < 0 °C
    temp = abs(temp); // absolute value
    sprintf(tempBuffer, "-%02u.%1u C", temp / 10, temp % 10, HUMIDITY);
  }
  else
  {
    if (temp >= 1000) // if temperature >= 100.0 °C
      sprintf(tempBuffer, "%03u.%1u C", temp / 10, temp % 10, HUMIDITY);
    else
      sprintf(tempBuffer, " %02u.%1u C", temp / 10, temp % 10, HUMIDITY);
  }


  return tempBuffer;
}


/// This function sends a page to an Ethernet client.
/// It prints the HTTP/1.1 200 OK status code, the content type, and the connection status.
/// It then prints the refresh rate and HTML tags.
/// It gets the current temperature and formats it, then sets the wanted temperature and formats it as well.
/// It then checks if the wanted temperature is lower than the current temperature and sets a digital pin accordingly.
/// Finally, it prints two buttons with JavaScript functions to increase or decrease heat, followed by closing HTML tags.
void sendPage(EthernetClient client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");


  client.print("Refresh: ");
  client.println(REFRESH_RATE);
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<style>");
  client.println("body{ background-image: url('https://images.unsplash.com/photo-1526666923127-b2970f64b422?ixlib=rb-4.0.3&ixid=MnwxMjA3fDB8MHxwaG90by1wYWdlfHx8fGVufDB8fHx8&auto=format&fit=crop&w=2072&q=80'); background-repeat: no-repeat; background-size: cover;}");
 client.println("h1{ color: white; text-align: center;}");
 client.println("h2{ color: white; text-align: center;}");
 client.println("h3{ color: white; text-align: center;}");
 client.println("input{ font-size: 32px; color: white;background-color: transparent; align-items: center; display: block; margin: 0 auto;text-align: center; padding: 10px; }");
 client.println("</style>");
 client.println("<html>");
 client.println("<h1>SmartHome Thermometer</h1>");


 int16_t temperature = GetTemp();
 String formatedTemp = formatTemperature(temperature);


 client.print("<h2 style='align-items: center; justify-content: center;'>Current temperature: ");
 client.print(formatedTemp);
 client.println("</h2>");


 int16_t wantedTemp = SetTemp();
 String formatedWantedTemp = formatTemperature(wantedTemp);
 client.print("<h3 style='align-items: center; justify-content: center;'>Wanted temperature: ");
 client.print(formatedWantedTemp);
 client.println("</h3>");


 client.print("<h3 style='align-items: center; justify-content: center;'>Status: ");
  if (formatedWantedTemp < formatedTemp) {
   client.println("Idle</h3>");
   digitalWrite(2, LOW);
 } else {
   client.println("Heating</h3>");
   digitalWrite(2, HIGH);
 }
  client.println("<input type=\"button\" value=\"Increase temperature\" onclick=\"upTheHeat()\" />");
 client.println("<input type=\"button\" value=\"Decrease temperature \" onclick=\"downTheHeat()\" />");
  client.println("<script>function upTheHeat(){console.log('upTheHeat');let xhr=makeGetRequest(1);xhr.send()}function downTheHeat(){console.log('downTheHeat');let xhr=makeGetRequest(0);xhr.send()}function makeGetRequest(heatParam){const url='http://192.168.0.238?heatGoes=' + heatParam;let xhr=new XMLHttpRequest;xhr.open('GET',url,true);return xhr}</script>");
 client.println("</html>");


}


/// This function handles a client request. It takes an EthernetClient object as a parameter.
/// It will loop while the client is connected and while the client has available data.
/// The function will read each character of the request, store it in a string buffer, and print it to the serial monitor.
/// If the request contains "heatGoes=" then it will call handleHeatRequest() with the client and character as parameters.
/// If no heat request is found, then if it encounters a new line character and the current line is blank, it will call sendPage() with the client as a parameter and then endClientConnection().
/// Otherwise, if it encounters a new line character, it sets currentLineIsBlank to true; otherwise, if it encounters any other character besides carriage return, currentLineIsBlank is set to false.
void handleClientRequest(EthernetClient client)
{
  Serial.println("new client");
  boolean currentLineIsBlank = true;


  while (client.connected())
  {
    boolean requestIsHeatUpdate = false;
    String requestBuffer = "";


    while (client.available())
    {
      char c = client.read();
      requestBuffer += c;


      Serial.print(c);
      if (requestBuffer.indexOf("heatGoes=") > -1 && !requestIsHeatUpdate)
      {
        requestIsHeatUpdate = true;
      }
      else if (requestIsHeatUpdate)
      {
        handleHeatRequest(client, c);
        break;
      }
      else if (c == '\n' && currentLineIsBlank)
      {
        sendPage(client);
        endClientConnection(client);
        break;
      }


      if (c == '\n')
        currentLineIsBlank = true;
      else if (c != '\r')
        currentLineIsBlank = false;
    }
  }
}


/// This function is used to get the temperature low (tl) value from a DS1621 temperature sensor.
/// It begins by sending a transmission to the address of the sensor, and then writing 0xA2 to it.
/// The transmission then ended without releasing the bus. The function then requests two bytes from the sensor and stores them in two separate variables.
/// Finally, it uses a helper function called msbAndLsbToInt() to combine these two bytes into one int16_t value, which is returned by the function.
int16_t getTL()
{
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xA2);
  Wire.endTransmission(false);
  Wire.requestFrom(DS1621_ADR, 2);
  uint8_t t_msb = Wire.read();
  uint8_t t_lsb = Wire.read();
  int16_t tl = msbAndLsbToInt(t_msb, t_lsb);


  return tl;
}
