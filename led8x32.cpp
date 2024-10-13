#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

#define LED_PIN D2
#define LED_COUNT 256
#define MATRIX_WIDTH 32
#define MATRIX_HEIGHT 8

Adafruit_NeoPixel matrix(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "CHANGEME";
const char* password = "CHANGEME";
const char* hostname = "led00";

AsyncWebServer server(80);

String currentText = "HELLO WORLD";
uint32_t currentColor = 0xFFFFFF;  // White
int scrollPosition = MATRIX_WIDTH;
bool textUpdated = true;
bool textCompleted = false;
int repeatCount = 0;  // 0 for none, 1 for infinity

// Add this near the top of your file, after the #include statements
const uint8_t font[] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00,  // Space
  0x3E, 0x5B, 0x4F, 0x5B, 0x3E,  // ! (unchanged)
  0x3E, 0x6B, 0x4F, 0x6B, 0x3E,  // " (unchanged)
  0x1C, 0x3E, 0x7C, 0x3E, 0x1C,  // # (unchanged)
  0x18, 0x3C, 0x7E, 0x3C, 0x18,  // $ (unchanged)
  0x1C, 0x57, 0x7D, 0x57, 0x1C,  // % (unchanged)
  0x1C, 0x5E, 0x7F, 0x5E, 0x1C,  // & (unchanged)
  0x00, 0x18, 0x3C, 0x18, 0x00,  // ' (unchanged)
  0x00, 0x7E, 0x42, 0x42, 0x00,  // ( (unchanged)
  0x00, 0x42, 0x42, 0x7E, 0x00,  // ) (unchanged)
  0x2A, 0x1C, 0x7F, 0x1C, 0x2A,  // * (unchanged)
  0x08, 0x08, 0x3E, 0x08, 0x08,  // + (unchanged)
  0x00, 0x80, 0x70, 0x30, 0x00,  // , (unchanged)
  0x08, 0x08, 0x08, 0x08, 0x08,  // - (unchanged)
  0x00, 0x00, 0x60, 0x60, 0x00,  // . (unchanged)
  0x20, 0x10, 0x08, 0x04, 0x02,  // / (unchanged)
  0x3E, 0x51, 0x49, 0x45, 0x3E,  // 0 (unchanged)
  0x00, 0x42, 0x7F, 0x40, 0x00,  // 1 (unchanged)
  0x42, 0x61, 0x51, 0x49, 0x46,  // 2 (unchanged)
  0x21, 0x41, 0x45, 0x4B, 0x31,  // 3 (unchanged)
  0x18, 0x14, 0x12, 0x7F, 0x10,  // 4 (unchanged)
  0x27, 0x45, 0x45, 0x45, 0x39,  // 5 (unchanged)
  0x3C, 0x4A, 0x49, 0x49, 0x30,  // 6 (unchanged)
  0x01, 0x71, 0x09, 0x05, 0x03,  // 7 (unchanged)
  0x36, 0x49, 0x49, 0x49, 0x36,  // 8 (unchanged)
  0x06, 0x49, 0x49, 0x29, 0x1E,  // 9 (unchanged)
  0x00, 0x36, 0x36, 0x00, 0x00,  // : (unchanged)
  0x00, 0x56, 0x36, 0x00, 0x00,  // ; (unchanged)
  0x08, 0x14, 0x22, 0x41, 0x00,  // < (unchanged)
  0x14, 0x14, 0x14, 0x14, 0x14,  // = (unchanged)
  0x00, 0x41, 0x22, 0x14, 0x08,  // > (unchanged)
  0x02, 0x01, 0x51, 0x09, 0x06,  // ? (unchanged)
  0x32, 0x49, 0x79, 0x41, 0x3E,  // @ (unchanged)
  0x7E, 0x11, 0x11, 0x11, 0x7E,  // A
  0x7F, 0x49, 0x49, 0x49, 0x36,  // B
  0x3E, 0x41, 0x41, 0x41, 0x22,  // C
  0x7F, 0x41, 0x41, 0x22, 0x1C,  // D
  0x7F, 0x49, 0x49, 0x49, 0x41,  // E
  0x7F, 0x09, 0x09, 0x09, 0x01,  // F
  0x3E, 0x41, 0x49, 0x49, 0x7A,  // G
  0x7F, 0x08, 0x08, 0x08, 0x7F,  // H
  0x00, 0x41, 0x7F, 0x41, 0x00,  // I
  0x20, 0x40, 0x41, 0x3F, 0x01,  // J
  0x7F, 0x08, 0x14, 0x22, 0x41,  // K
  0x7F, 0x40, 0x40, 0x40, 0x40,  // L
  0x7F, 0x02, 0x0C, 0x02, 0x7F,  // M
  0x7F, 0x04, 0x08, 0x10, 0x7F,  // N
  0x3E, 0x41, 0x41, 0x41, 0x3E,  // O
  0x7F, 0x09, 0x09, 0x09, 0x06,  // P
  0x3E, 0x41, 0x51, 0x21, 0x5E,  // Q
  0x7F, 0x09, 0x19, 0x29, 0x46,  // R
  0x46, 0x49, 0x49, 0x49, 0x31,  // S
  0x01, 0x01, 0x7F, 0x01, 0x01,  // T
  0x3F, 0x40, 0x40, 0x40, 0x3F,  // U
  0x1F, 0x20, 0x40, 0x20, 0x1F,  // V
  0x3F, 0x40, 0x38, 0x40, 0x3F,  // W
  0x63, 0x14, 0x08, 0x14, 0x63,  // X
  0x07, 0x08, 0x70, 0x08, 0x07,  // Y
  0x61, 0x51, 0x49, 0x45, 0x43,  // Z
  0x20, 0x54, 0x54, 0x54, 0x78, // a
  0x7F, 0x48, 0x44, 0x44, 0x38, // b
  0x38, 0x44, 0x44, 0x44, 0x20, // c
  0x38, 0x44, 0x44, 0x48, 0x7F, // d
  0x38, 0x54, 0x54, 0x54, 0x18, // e
  0x08, 0x7E, 0x09, 0x01, 0x02, // f
  0x0C, 0x52, 0x52, 0x52, 0x3E, // g
  0x7F, 0x08, 0x04, 0x04, 0x78, // h
  0x00, 0x44, 0x7D, 0x40, 0x00, // i
  0x20, 0x40, 0x44, 0x3D, 0x00, // j
  0x7F, 0x10, 0x28, 0x44, 0x00, // k
  0x00, 0x41, 0x7F, 0x40, 0x00, // l
  0x7C, 0x04, 0x18, 0x04, 0x78, // m
  0x7C, 0x08, 0x04, 0x04, 0x78, // n
  0x38, 0x44, 0x44, 0x44, 0x38, // o
  0x7C, 0x14, 0x14, 0x14, 0x08, // p
  0x08, 0x14, 0x14, 0x18, 0x7C, // q
  0x7C, 0x08, 0x04, 0x04, 0x08, // r
  0x48, 0x54, 0x54, 0x54, 0x20, // s
  0x04, 0x3F, 0x44, 0x40, 0x20, // t
  0x3C, 0x40, 0x40, 0x20, 0x7C, // u
  0x1C, 0x20, 0x40, 0x20, 0x1C, // v
  0x3C, 0x40, 0x30, 0x40, 0x3C, // w
  0x44, 0x28, 0x10, 0x28, 0x44, // x
  0x0C, 0x50, 0x50, 0x50, 0x3C, // y
  0x44, 0x64, 0x54, 0x4C, 0x44  // z
};

// Function declarations
void setPixel(int x, int y, uint32_t color);
void drawChar(char c, int x, uint32_t color);
void drawCharColumn(char c, int x, int col, uint32_t color);
void scrollText();
uint32_t Wheel(byte WheelPos);
void saveSettings();
void loadSettings();


// Rainbow color wheel
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void setPixel(int x, int y, uint32_t color) {
  if (x >= 0 && x < MATRIX_WIDTH && y >= 0 && y < MATRIX_HEIGHT) {
    int pixelIndex;
    if (x % 2 == 0) {
      // Even columns run top to bottom
      pixelIndex = x * MATRIX_HEIGHT + y;
    } else {
      // Odd columns run bottom to top
      pixelIndex = x * MATRIX_HEIGHT + (MATRIX_HEIGHT - 1 - y);
    }
    matrix.setPixelColor(pixelIndex, color);
  }
}

void drawChar(char c, int x, uint32_t color) {
  if (c < ' ' || c > 'z') return;  // Only handle printable characters (from space ' ' to 'z')

  int charIndex;
  if (c >= 'a' && c <= 'z') {
    charIndex = c - 'a' + 59;  // Map lowercase letters to indices 26-51
  } else {
    charIndex = c - ' ';  // Map other characters as before
  }

  for (int col = 0; col < 5; col++) {
    uint8_t line = pgm_read_byte(&font[charIndex * 5 + col]);
    for (int row = 0; row < 8; row++) {
      if (line & 0x1) {
        setPixel(x + col, row, color);
      }
      line >>= 1;
    }
  }
}

void drawCharColumn(char c, int x, int col, uint32_t color) {
  if (c < ' ' || c > 'z' || col < 0 || col >= 5) return;

  int charIndex;
  if (c >= 'a' && c <= 'z') {
    charIndex = c - 'a' + 59;  // Map lowercase letters to indices 26-51
  } else {
    charIndex = c - ' ';  // Map other characters as before
  }

  uint8_t line = pgm_read_byte(&font[charIndex * 5 + col]);
  for (int row = 0; row < 8; row++) {
    if (line & 0x1) {
      setPixel(x, row, color);
    }
    line >>= 1;
  }
}

void scrollText() {
  if (textCompleted && repeatCount == 0) return;

  matrix.clear();
  int x = scrollPosition;
  int letterSpacing = 6;

  // Draw the text
  for (unsigned int i = 0; i < currentText.length(); i++) {
    char c = currentText[i];
    if (x + 5 > 0 && x < MATRIX_WIDTH) {  // Only draw visible characters
      if (currentColor == 0xFFFFFFFF) {  // Rainbow mode
        for (int col = 0; col < 5; col++) {
          uint32_t color = Wheel(((x + col) * 256 / MATRIX_WIDTH) & 255);
          drawCharColumn(c, x + col, col, color);
        }
      } else {
        drawChar(c, x, currentColor);
      }
    }
    x += letterSpacing;
  }

  matrix.show();
  scrollPosition--;

  // Check if the entire text has scrolled off
  if (scrollPosition < -(int(currentText.length() * letterSpacing))) {
    if (repeatCount == 0) {
      textCompleted = true;
    } else {
      scrollPosition = MATRIX_WIDTH;  // Reset for repeat
    }
  }
}

void saveSettings() {
  EEPROM.write(0, repeatCount);
  // Save color (4 bytes)
  EEPROM.write(1, (currentColor >> 24) & 0xFF);
  EEPROM.write(2, (currentColor >> 16) & 0xFF);
  EEPROM.write(3, (currentColor >> 8) & 0xFF);
  EEPROM.write(4, currentColor & 0xFF);
  // Save text
  for (unsigned int i = 0; i < currentText.length(); i++) {
    EEPROM.write(i + 5, currentText[i]);
  }
  EEPROM.write(currentText.length() + 5, '\0');  // Null terminator
  EEPROM.commit();
}

void loadSettings() {
  repeatCount = EEPROM.read(0);
  // Load color
  currentColor = EEPROM.read(1) << 24 | EEPROM.read(2) << 16 | EEPROM.read(3) << 8 | EEPROM.read(4);
  // Load text
  currentText = "";
  for (int i = 5; i < 105; i++) {  // Assuming max text length of 100 characters
    char c = EEPROM.read(i);
    if (c == '\0') break;
    currentText += c;
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>LED Matrix Control</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      background-color: #1e1e1e;
      color: #ffffff;
      margin: 0;
      padding: 20px;
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
      background-color: #2d2d2d;
      border-radius: 10px;
      padding: 20px;
      box-shadow: 0 0 10px rgba(0,0,0,0.5);
    }
    h1 {
      color: #4CAF50;
      text-align: center;
      margin-bottom: 30px;
    }
    .input-row {
      display: flex;
      justify-content: space-between;
      margin-bottom: 15px;
      padding: 10px;
      background-color: #3d3d3d;
      border-radius: 5px;
    }
    .label {
      font-weight: bold;
    }
    input[type="text"], input[type="color"] {
      background-color: #4d4d4d;
      border: none;
      padding: 5px;
      color: #ffffff;
      border-radius: 3px;
    }
    button {
      background-color: #4CAF50;
      border: none;
      color: white;
      padding: 10px 20px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      margin: 4px 2px;
      cursor: pointer;
      border-radius: 5px;
    }
    .checkbox-row {
      display: flex;
      align-items: center;
      margin-bottom: 15px;
    }
    .checkbox-row input[type="checkbox"] {
      margin-right: 10px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>LED Matrix Control</h1>
    <div class="input-row">
      <span class="label">Text:</span>
      <input type="text" id="text" placeholder="Enter text" onkeypress="handleKeyPress(event)">
    </div>
    <div class="input-row">
      <span class="label">Color:</span>
      <input type="color" id="color" value="#ffffff">
    </div>
    <div class="checkbox-row">
      <input type="checkbox" id="repeat" name="repeat">
      <label for="repeat">Repeat Infinitely</label>
    </div>
    <div class="input-row">
      <button onclick="setText()">Set Text</button>
      <button onclick="setRainbow()">Rainbow</button>
    </div>
  </div>

  <script>
    function setText() {
      var text = document.getElementById('text').value;
      var color = document.getElementById('color').value;
      var repeat = document.getElementById('repeat').checked ? 1 : 0;
      setDisplaySettings(text, color, repeat);
    }

    function setRainbow() {
      var text = document.getElementById('text').value;
      if (text.trim() === "") {
        text = "RAINBOW";
      }
      var repeat = document.getElementById('repeat').checked ? 1 : 0;
      setDisplaySettings(text, "#FFFFFFFF", repeat);
    }

    function setDisplaySettings(text, color, repeat) {
      fetch('/api/display', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `text=${encodeURIComponent(text)}&color=${encodeURIComponent(color)}&repeat=${repeat}`
      })
        .then(response => response.json())
        .then(data => console.log(data));
    }

    function handleKeyPress(event) {
      if (event.keyCode === 13) {
        event.preventDefault();
        setText();
      }
    }
  </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(50);
  matrix.clear();
  matrix.show();
  Serial.println("LED matrix initialized");

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set up mDNS responder
  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
    Serial.print("Device can be reached at http://");
    Serial.print(hostname);
    Serial.println(".local");
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  // Initialize EEPROM
  EEPROM.begin(512);
  loadSettings();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/api/display", HTTP_GET, [](AsyncWebServerRequest *request){
    String response = "{\"text\":\"" + currentText + "\",\"color\":\"" + String(currentColor, HEX) + "\",\"repeat\":" + String(repeatCount) + "}";
    request->send(200, "application/json", response);
  });

  server.on("/api/display", HTTP_POST, [](AsyncWebServerRequest *request){
    if (request->hasParam("text", true) && request->hasParam("color", true) && request->hasParam("repeat", true)) {
      currentText = request->getParam("text", true)->value();
      String colorHex = request->getParam("color", true)->value();
      colorHex.replace("#", "");
      if (colorHex == "FFFFFFFF") {
        currentColor = 0xFFFFFFFF;  // Special flag for rainbow mode
      } else {
        currentColor = strtol(colorHex.c_str(), NULL, 16);
      }
      repeatCount = request->getParam("repeat", true)->value().toInt();
      scrollPosition = MATRIX_WIDTH;
      textCompleted = false;
      saveSettings();
      request->send(201, "application/json", "{\"message\":\"Display settings created\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"Missing parameters\"}");
    }
  });

  server.on("/api/display", HTTP_PUT, [](AsyncWebServerRequest *request){
    bool updated = false;
    if (request->hasParam("text", true)) {
      currentText = request->getParam("text", true)->value();
      updated = true;
    }
    if (request->hasParam("color", true)) {
      String colorHex = request->getParam("color", true)->value();
      colorHex.replace("#", "");
      if (colorHex == "FFFFFFFF") {
        currentColor = 0xFFFFFFFF;  // Special flag for rainbow mode
      } else {
        currentColor = strtol(colorHex.c_str(), NULL, 16);
      }
      updated = true;
    }
    if (request->hasParam("repeat", true)) {
      repeatCount = request->getParam("repeat", true)->value().toInt();
      updated = true;
    }
    if (updated) {
      scrollPosition = MATRIX_WIDTH;
      textCompleted = false;
      saveSettings();
      request->send(200, "application/json", "{\"message\":\"Display settings updated\"}");
    } else {
      request->send(400, "application/json", "{\"error\":\"No parameters to update\"}");
    }
  });

  server.on("/api/display", HTTP_DELETE, [](AsyncWebServerRequest *request){
    currentText = "";
    currentColor = 0xFFFFFF;
    repeatCount = 0;
    scrollPosition = MATRIX_WIDTH;
    textCompleted = true;
    saveSettings();
    request->send(200, "application/json", "{\"message\":\"Display cleared\"}");
  });

  server.begin();
  Serial.println("HTTP server started");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  MDNS.update();  // Handle mDNS updates
  scrollText();
  delay(50);  // Adjust for scroll speed
}