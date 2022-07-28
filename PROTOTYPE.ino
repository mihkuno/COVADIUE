#include <U8g2lib.h>
#include <Wire.h>
#include <WebServer.h>
#include <Adafruit_MLX90614.h>


// wifi
WebServer syncServer(80);
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
    const short sl = 2500;

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
    if (syncServer.argName(0)=="metadata") { // prone to errors
      String metadata = syncServer.arg(0);
      String metaname = syncServer.argName(0);
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
        u8g2.drawStr(x, y-5, cstr(String(cm)+"cm"));
        u8g2.drawStr(x+a-10, y-5, msk);   

        // if (atoi(cm) < 20) {
        //   // read temperature
        //   const byte bias = 10;
        //   byte ambient = mlx.readAmbientTempC();
        //   String celsius = String(mlx.readObjectTempC());
        //   u8g2.drawStr(5,5,cstr(celsius));
        //   // u8g2.drawStr(10,10,cstr("bias: "+String(bias)));

        //   Serial.println(String(celsius)+"C");
        // }     
      }
      if (!inf_status) {
        Serial.println("Could not find a infrared, check wiring!");
      }
      else {
        Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC()); 
        Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
        Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF()); 
        Serial.print("*F\tObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");
      }
      
      u8g2.sendBuffer();
    }
  });

  // multiple arguments with one parameter
  syncServer.on("/covscrape", [] {
    syncServer.send(200, "text/plain", "welcome to covscrape!");

    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_t0_11_tr);
    if (syncServer.argName(0)=="cases") {
      String activeCases = syncServer.arg(0);
      String metaname =  syncServer.argName(0);
      String metainfo = "GET: "+metaname+" > "+activeCases;
    } else {startDisplay(true);}
    if (syncServer.argName(1)=="death") {
      String totalDeath = syncServer.arg(1);
      String metaname =  syncServer.argName(1);
      String metainfo = "GET: "+metaname+" > "+totalDeath;
      Serial.printf("death: %s\n",totalDeath);
    }
    if (syncServer.argName(2)=="recov") {
      String totalRecov = syncServer.arg(2);
      String metaname =  syncServer.argName(2);
      String metainfo = "GET: "+metaname+" > "+totalRecov;
      Serial.printf("recov: %s\n",totalRecov);
    }
    if (syncServer.argName(3)=="nwdat") {
      String latestDate = syncServer.arg(3);
      latestDate.replace("."," ");
      Serial.printf("nwdat: %s\n",latestDate);
      u8g2.drawStr(0,15,cstr("As of "+latestDate+","));
    } 
    if (syncServer.argName(4)=="nwcas") {
      String latestCase = syncServer.arg(4);
      Serial.printf("nwcas: %s\n",latestCase);  

      byte x = 0;
      for (byte n=0; latestCase.length() > n; n++) {
        x = 36-(6*n);  
      }
      u8g2.drawStr(x,32,cstr(latestCase+" new cases"));
    } 
    if (syncServer.argName(5)=="nwdea") { 
      String latestDead = syncServer.arg(5);
      Serial.printf("nwdea: %s\n",latestDead);

      byte x = 0;
      for (byte n=0; latestDead.length() > n; n++) {
        x = 36-(6*n);  
      }
      u8g2.drawStr(x,42,cstr(latestDead+" new deaths"));
    } 
    if (syncServer.argName(6)=="activ") {
      String activeCases = syncServer.arg(6);
      String metaname =  syncServer.argName(6);
      String metainfo = "GET: "+metaname+" > "+activeCases;
      Serial.printf("activ: %s\n",activeCases);
      u8g2.drawStr(0,60, cstr("ACC "+activeCases));
    }
    if (syncServer.argName(7)=="wkper") {
      String weekPercent = syncServer.arg(7);
      String metaname =  syncServer.argName(7);
      String metainfo = "GET: "+metaname+" > "+weekPercent;
      Serial.printf("wkper: %s\n",weekPercent);
      u8g2.drawStr(70,60, cstr("PTC "+weekPercent));
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
  // startInfrared();// 0x5A 
  startDisplay(); // 0x3C

  // Start syncServer
  syncServer.begin(); 
}
void loop() {
  syncServer.handleClient();
}
