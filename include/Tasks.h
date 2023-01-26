
#define DS1621_ADR 0x48
#define DEFAULT_TEMP 27
#define HUMIDITY 223
#define REFRESH_RATE 5

void setWantedTemperature(int16_t wanted_temp) {
  byte th_msb = byte(wanted_temp);
  byte th_lsb = B00000000;
  byte tl_msb = byte(wanted_temp - 1);
  byte tl_lsb = B00000000;

  //Set T High
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xA1); //Set TH address
  Wire.write(th_msb); //Set TH
  Wire.write(th_lsb); //Set TH
  Wire.endTransmission();
  delay(10);

  //Set T Low
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xA2);
  Wire.write(tl_msb);
  Wire.write(tl_lsb);
  Wire.endTransmission();
}

void setupThermometer() {
  Wire.begin();
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xAC); //Set config address
  Wire.write(0); //Set continous conversion
  Wire.endTransmission();
  delay(10);

  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xEE);
  Wire.endTransmission();

  setWantedTemperature(DEFAULT_TEMP);
}

int16_t msbAndLsbToInt(uint8_t msb, uint8_t lsb) {
  int16_t raw_int = (int8_t)msb << 1 | lsb >> 7;
  raw_int = raw_int * 10/2;
  
  return raw_int;
}

int16_t getTH() {
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xA1);
  Wire.endTransmission(false);
  Wire.requestFrom(DS1621_ADR,2);
  uint8_t t_msb = Wire.read();
  uint8_t t_lsb = Wire.read();
  int16_t th = msbAndLsbToInt(t_msb, t_lsb);

  return th;
}

int16_t setWantedTemperature() {
  return getTH();
}



void updateTemperature(int turnUp) {
  int16_t newWantedTemp = setWantedTemperature();

  if (turnUp == 1) newWantedTemp = newWantedTemp + 10;
  else newWantedTemp = newWantedTemp - 10;

  newWantedTemp = newWantedTemp / 10;

  setWantedTemperature(newWantedTemp);
}

void endClientConnection(EthernetClient client) {
   ///give the web browser time to receive the data
  delay(1);
  /// close the connection:
  client.stop();
  Serial.println("client disconnected");
}

void handleHeatRequest(EthernetClient client, char turnUpHeatParameter) {
  int turnUpHeat = turnUpHeatParameter - '0';
        
  updateTemperature(turnUpHeat);
  client.println("HTTP/1.1 200 OK");
  endClientConnection(client);
}




int16_t getTemperature() {
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xAA);
  Wire.endTransmission(false);
  Wire.requestFrom(DS1621_ADR, 2);
  uint8_t t_msb = Wire.read();
  uint8_t t_lsb = Wire.read();
  int16_t temp = msbAndLsbToInt(t_msb, t_lsb);
  
  return temp;
}

String formatTemperature (int16_t temp) {
  char tempBuffer[8];
  
  if(temp < 0) {   // if temperature < 0 °C
    temp = abs(temp);  // absolute value
    sprintf(tempBuffer, "-%02u.%1u C", temp / 10, temp % 10, HUMIDITY);
  }
  else {
    if (temp >= 1000)    // if temperature >= 100.0 °C
      sprintf(tempBuffer, "%03u.%1u C", temp / 10, temp % 10, HUMIDITY);
    else
      sprintf(tempBuffer, " %02u.%1u C", temp / 10, temp % 10, HUMIDITY);
  }

  return tempBuffer;
}






void sendPage(EthernetClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");  // the connection will be closed after completion of the response
  
  client.print("Refresh: ");
  client.println(REFRESH_RATE);
  client.println();
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<h1>Temperatur</h1>");

  int16_t temperature = getTemperature();
  String formatedTemp = formatTemperature(temperature);

  client.print("<h2>Current temperature: ");
  client.print(formatedTemp);
  client.println("</h2>");

  int16_t wantedTemp = setWantedTemperature();
  String formatedWantedTemp = formatTemperature(wantedTemp);
  
  client.print("<h3>Wanted temperature: ");
  client.print(formatedWantedTemp);
  client.println("</h3>");

  client.print("<h3>Status: ");
  
  if (formatedWantedTemp < formatedTemp) {
    client.println("Idle</h3>");
    digitalWrite(2, LOW);
  } else {
    client.println("Heating</h3>");
    digitalWrite(2, HIGH);
  }
  
  client.println("<input type=\"button\" value=\"+\" onclick=\"upTheHeat()\" />");
  client.println("<input type=\"button\" value=\"-\" onclick=\"downTheHeat()\" />");
  
  client.println("<script>function upTheHeat(){console.log('upTheHeat');let xhr=makeGetRequest(1);xhr.send()}function downTheHeat(){console.log('downTheHeat');let xhr=makeGetRequest(0);xhr.send()}function makeGetRequest(heatParam){const url='http://192.168.0.238?heatGoes=' + heatParam;let xhr=new XMLHttpRequest;xhr.open('GET',url,true);return xhr}</script>");
  client.println("</html>");
}


void handleClientRequest(EthernetClient client) {
  Serial.println("new client");
  boolean currentLineIsBlank = true;
  
  while (client.connected()) {
    boolean requestIsHeatUpdate = false;
    String requestBuffer = "";

    while(client.available()) {
      char c = client.read();
      requestBuffer += c;

      Serial.print(c);
      if (requestBuffer.indexOf("heatGoes=") > -1 && !requestIsHeatUpdate) {
        requestIsHeatUpdate = true;
      } 
      else if (requestIsHeatUpdate) {
        handleHeatRequest(client, c);
        break;
      } 
      else if (c == '\n' && currentLineIsBlank){
        sendPage(client);
        endClientConnection(client);
        break;
      }

      if (c == '\n') currentLineIsBlank = true;
      else if (c != '\r') currentLineIsBlank = false;
    }
  }
}

int16_t getTL() {
  Wire.beginTransmission(DS1621_ADR);
  Wire.write(0xA2);
  Wire.endTransmission(false);
  Wire.requestFrom(DS1621_ADR,2);
  uint8_t t_msb = Wire.read();
  uint8_t t_lsb = Wire.read();
  int16_t tl = msbAndLsbToInt(t_msb, t_lsb);

  return tl;
}
