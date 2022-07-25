#include <U8g2lib.h>
#include <Wire.h>
#include <esp_camera.h>
#include <esp32cam.h>
#include <WebServer.h>

#define LED 33
#define SDA 2
#define SCL 14

WebServer syncServer(80);

const char* ssid = "PLDTHOMEFIBRdbb60";
const char* password = "PLDTWIFIfbwm9";

// define frame resolution
static auto tinyRes = esp32cam::Resolution::find(128, 64);
static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(640, 480);
static auto hiRes = esp32cam::Resolution::find(800, 600);

// page buffer hardware I2C
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);



// ####################################
// OLED DISPLAY
void startDisplay() {
  Wire.begin(SDA, SCL);
  u8g2.begin();
  
  u8g2.clearBuffer();
  // u8g2.setFont(u8g2_font_bpixeldouble_tr);  // original
  // u8g2.drawStr(26, 34, "C0V4D1U3"); // write something to the internal memory
  // u8g2.setFont(u8g2_font_squeezed_b7_tr);   // looks like segoe ui variable
  u8g2.setFont(u8g2_font_sonicmania_tr);
  u8g2.drawStr(35, 34, "C0V4D1U3"); // write something to the internal memory
  u8g2.drawFrame(0, 0, 128, 64);
  u8g2.sendBuffer();
}
void listenParameters(String metastring) {
  
  const byte charLength = metastring.length()+1;  // number characters in token
  const char* delimiter = ":"; // split charArray with delimiter
  const char* splitter = ">"; // split charArray with delimiter
  char charArray[charLength]; // token string in characters

  metastring.toCharArray(charArray, charLength);
  const char* s = metastring.c_str();
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
    u8g2.drawStr(x, y-5, cm);
    u8g2.drawStr(x+a-10, y-5, msk);        
  }
  u8g2.sendBuffer();
}



// ####################################
// WiFi CONNECT
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



// ####################################
// ESP32 WEBCAMERA
// send handle to server
void serveJpg() {
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    syncServer.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
 
  syncServer.setContentLength(frame->size());
  syncServer.send(200, "image/jpeg");
  WiFiClient client = syncServer.client();
  frame->writeTo(client);
}
// connect wifi then serve to url
void startWebCam() {
  // OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
  #define PWDN_GPIO_NUM     32
  #define RESET_GPIO_NUM    -1
  #define XCLK_GPIO_NUM      0
  #define SIOD_GPIO_NUM     26
  #define SIOC_GPIO_NUM     27
  #define Y9_GPIO_NUM       35
  #define Y8_GPIO_NUM       34
  #define Y7_GPIO_NUM       39
  #define Y6_GPIO_NUM       36
  #define Y5_GPIO_NUM       21
  #define Y4_GPIO_NUM       19
  #define Y3_GPIO_NUM       18
  #define Y2_GPIO_NUM        5
  #define VSYNC_GPIO_NUM    25
  #define HREF_GPIO_NUM     23
  #define PCLK_GPIO_NUM     22
  
  // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 30000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // config for psram
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
}



// ####################################
// WEBSERVER-BASED SERIAL
void initServer() {

  syncServer.on("/", []() {
    syncServer.send(200, "text/plain", "Hello world, from esp32!");
  });  

  syncServer.on("/capture.jpg", []() {
    if (!esp32cam::Camera.changeResolution(loRes)) {
      Serial.println("SET-TY-RES FAIL");
    } 
    if (syncServer.argName(0)=="metadata") { // prone to errors
      String metadata = syncServer.arg(0);
      String metaname = syncServer.argName(0);
      String metainfo = "GET: "+metaname+" > "+metadata;
      
      Serial.println(metainfo);
      listenParameters(metadata);
    }
    serveJpg();
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


// ####################################
// MAIN SETUP & LOOP

void setup() {
  // startup led alert  (inverted logic)
  digitalWrite(LED, LOW);

  // Serial port for debugging purposes
  Serial.begin(115200);
	Serial.setTimeout(50);
  Serial.println("initializing..");

  connectWifi();
  initServer();

  startWebCam();
  startDisplay();

  // Start syncServer
  syncServer.begin(); 
}
void loop() {
  syncServer.handleClient();
}
