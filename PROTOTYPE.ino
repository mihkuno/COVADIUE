#include <U8g2lib.h>
#include <Wire.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Adafruit_MLX90614.h>


// Globals
WebServer syncServer(80);
bool SOCKET_ACTIVE = true;
WebSocketsServer webSocket = WebSocketsServer(81);

// wifi
const char* ssid = "PLDTHOMEFIBRdbb60";
const char* password = "PLDTWIFIfbwm9";

// infrared
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// page buffer hardware I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// status
bool oled_status;
bool inf_status;

char* cstr(String val) {
  const byte charLength = val.length()+1;
  char charArray[charLength]; 
  val.toCharArray(charArray, charLength);
  return strdup(charArray);
}
void startDisplay(bool animation=false) {  
  
  // u8g2.setFont(u8g2_font_bpixeldouble_tr);  // original
  // u8g2.drawStr(26, 34, "C0V4D1U3"); // write something to the internal memory
  // u8g2.setFont(u8g2_font_squeezed_b7_tr);   // looks like segoe ui variable
  
  if (animation) {
    const byte c = 64;
    const byte y = 55;
    const byte a = 4;
    const byte g = 16;
    const short sl = 400;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_sonicmania_tr);
    u8g2.drawStr(35, 34, "C0V4D1U3"); // write something to the internal memory
    u8g2.drawBox(((c-(a+(g/2)))-(a+g)),y,a,a);
    u8g2.sendBuffer();
    delay(sl);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_sonicmania_tr);
    u8g2.drawStr(35, 34, "C0V4D1U3"); // write something to the internal memory
    u8g2.drawBox(((c-(a+(g/2)))-(a+g)),y,a,a);
    u8g2.drawBox(c-(a+(g/2)),y,a,a);
    u8g2.sendBuffer();
    delay(sl);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_sonicmania_tr);
    u8g2.drawStr(35, 34, "C0V4D1U3"); // write something to the internal memory
    u8g2.drawBox(((c-(a+(g/2)))-(a+g)),y,a,a);
    u8g2.drawBox(c-(a+(g/2)),y,a,a);
    u8g2.drawBox(c+(g/2),y,a,a);
    u8g2.sendBuffer();
    delay(sl);

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_sonicmania_tr);
    u8g2.drawStr(35, 34, "C0V4D1U3"); // write something to the internal memory
    u8g2.drawBox(((c-(a+(g/2)))-(a+g)),y,a,a);
    u8g2.drawBox(c-(a+(g/2)),y,a,a);
    u8g2.drawBox(c+(g/2),y,a,a);
    u8g2.drawBox((c+(g/2))+(a+g),y,a,a);
    u8g2.sendBuffer();
    delay(sl);
  }
  else {
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_sonicmania_tr);
    u8g2.drawStr(35, 34, "C0V4D1U3"); // write something to the internal memory
    u8g2.drawFrame(0, 0, 128, 64);
    u8g2.sendBuffer();
  }
}
void connectWifi() {
  // Connect to Wi-Fi
  // WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  // Print ESP32 Local IP Address
  Serial.print("IP Address: http://");
  Serial.println(WiFi.localIP());
}
void initServer() {

  syncServer.on("/", []() {
    syncServer.send(200, "text/plain", "Hello world, from esp32!");
  });  

  // one argument with multiple parameters
  syncServer.on("/capture", []() {
    syncServer.send(200, "text/plain", "welcome to capture!");
    if (syncServer.argName(0) =="metadata") { // prone to errors
      String metadata = syncServer.arg(0);
      String metaname = "value";
      String metainfo = "GET: "+metaname+" > "+metadata;
      
      Serial.println(metainfo);
      
      const byte charLength = metadata.length()+1;  // number characters in token
      const char* delimiter = ":"; // split charArray with delimiter
      const char* splitter = ">"; // split charArray with delimiter
      char charArray[charLength]; // token string in characters

      metadata.toCharArray(charArray, charLength);
      const char* s = metadata.c_str();
      byte tc; //targetCount
      for (tc=0; s[tc]; s[tc]=='>' ? tc++ : *s++);

      char* target = strtok(charArray, splitter);
      char* faces[tc];
      for (byte i = 0; i < tc; i++) {
        faces[i] = target;               // assign next token to data
        target = strtok(NULL, splitter); // clear the previous token
      }

      const byte pc = 6;   // paramCount
      u8g2.clearBuffer();
      for (byte j = 0; j < tc; j++) {
        char* data[pc];     // parameter value in token
        char* token = strtok(faces[j], delimiter);
        for (byte i = 0; i < pc; i++) {
          data[i] = token;               // assign next token to data
          token = strtok(NULL, delimiter); // clear the previous token
        }
        byte x,y,a,k;
        x = atoi(data[0]); // x value
        y = atoi(data[1]); // y value
        a = atoi(data[2]); // area
        k = atoi(data[3]); // mask 
            
        u8g2.setFont(u8g2_font_u8glib_4_tr);
        u8g2.drawFrame(x,y,a,a);

        const char*  cm = data[4];
        const char* msk = k==1? "MK" : "FC";
        u8g2.drawStr(x, y-2, cstr(String(cm)+"cm"));
        u8g2.drawStr(x+a-7, y-2, msk);   

        if (atoi(cm) < 10) {
          if (!inf_status) {
            Serial.println("Could not find a infrared, check wiring!");
          }
          else {
            float ambient = mlx.readAmbientTempC();
            float tempC = mlx.readObjectTempC();
            Serial.printf("Ambient = %f\n",ambient); 
            Serial.printf("*C\tObject = %f*C\n",tempC); 
            u8g2.drawStr(0,55,cstr(String(tempC)+" °C"));
          }
        }     
      }      
      u8g2.sendBuffer();
    }
  });

  // multiple arguments with one parameter
  syncServer.on("/covscrape", [] {
    syncServer.send(200, "text/plain", "welcome to covscrape!");
    String latestDate, latestCase, latestDead;

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_11_tr);
    
    if (syncServer.args() <= 0) {
      startDisplay(true);
    }
    
    for (uint8_t i = 0; i < syncServer.args(); i++) {
      if (syncServer.argName(i) =="cases") {
        String activeCases = syncServer.arg(i);
        String metaname =  "cases";
        String metainfo = "GET: "+metaname+" > "+activeCases;
      }
      if (syncServer.argName(i) =="death") {
        String totalDeath = syncServer.arg(i);
        String metaname =  "death";
        String metainfo = "GET: "+metaname+" > "+totalDeath;
      }
      if (syncServer.argName(i) =="recov") {
        String totalRecov = syncServer.arg(i);
        String metaname =  "value";
        String metainfo = "GET: "+metaname+" > "+totalRecov;
      }
      if (syncServer.argName(i) =="nwdat") {
        latestDate = syncServer.arg(i);
        latestDate.replace("."," ");
        u8g2.drawStr(0,15,cstr("As of "+latestDate+","));
      }
      if (syncServer.argName(i) =="nwcas") {
        latestCase = syncServer.arg(i);
        Serial.printf("nwcas: %s\n",latestCase);  
        byte x = 0;
        for (byte n=0; latestCase.length() > n; n++) {
          x = 36-(6*n);  
        }
        u8g2.drawStr(x,32,cstr(latestCase+" new cases"));
      }
      if (syncServer.argName(i) =="nwdea") { 
        latestDead = syncServer.arg(i);
        Serial.printf("nwdea: %s\n",latestDead);
        byte x = 0;
        for (byte n=0; latestDead.length() > n; n++) {
          x = 36-(6*n);  
        }
        u8g2.drawStr(x,42,cstr(latestDead+" new deaths"));
      }
      if (syncServer.argName(i) =="wkper") {
        String weekPercent = syncServer.arg(i);
        String metaname =  "value";
        String metainfo = "GET: "+metaname+" > "+weekPercent;
        u8g2.drawStr(70,60, cstr("PTC "+weekPercent));
      }
      if (syncServer.argName(i) =="activ") {
        String activeCases = syncServer.arg(i);
        String metaname =  "value";
        String metainfo = "GET: "+metaname+" > "+activeCases;
        u8g2.drawStr(0,60, cstr("ACC "+activeCases));
      }
    }
    u8g2.sendBuffer();
  });

  syncServer.onNotFound( []() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += syncServer.uri();
    message += "\nMethod: ";
    message += (syncServer.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += syncServer.args();
    message += "\n";

    for (uint8_t i = 0; i < syncServer.args(); i++) {
      message += " " + syncServer.argName(i) + ": " + syncServer.arg(i) + "\n";
    }

    syncServer.send(404, "text/plain", message);
  });
}

void setup() {

  Wire.begin();
  
  byte sda = 26;
  byte scl = 27;
  uint32_t freq = 100000;
  Wire1.begin(sda, scl, freq);

  // u8g2.setI2CAddress(0x3C);
  // u8g2.setBusClock(100000);
  oled_status = u8g2.begin();    // Iinitialize with the I2C addr 0x3C (128x64)
  inf_status = mlx.begin(0x5A, &Wire1);

  // Serial port for debugging purposes
  Serial.begin(115200);
	Serial.setTimeout(50);
  Serial.println("initializing..");

  connectWifi();
  initServer();
  startDisplay(); // 0x3C
  // nope! startInfrared();// 0x5A 


  webSocket.begin();
  syncServer.begin();

  // socket server event assign callback
  webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  // Figure out the type of WebSocket event
  switch(type) {

    // Client has disconnected
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;

    // New client has connected
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from ", num);
        Serial.println(ip.toString());
      
        Serial.printf("[%u] Text: %s\n", num, "SOCKET_ACTIVE");
        webSocket.sendTXT(num, "SOCKET_ACTIVE");
      }
      break;

    // Echo text message back to client
    case WStype_TEXT:
      Serial.printf("[%u] Text: %s\n", num, payload);
      Serial.println((char * )payload);

      if ((char * )payload == "CLOSE_SOCKET") {
        webSocket.sendTXT(num, "CLOSED_SOCKET");
        
        SOCKET_ACTIVE = false;
      }
      break;
  }
});

}
void loop() {
  webSocket.loop();
  syncServer.handleClient();
}
