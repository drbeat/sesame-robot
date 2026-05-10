#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>         // NVS-backed credential storage
#include "board_config.h"       // Board-specific pins — selected via -DBOARD_* at compile time
#include "face-bitmaps.h"
#include "movement-sequences.h"
#include "captive-portal.h"
#include "wifi-config.h"        // WiFi provisioning UI
#include <ArduinoJson.h>        // Required for robust JSON parsing

// --- Access Point Configuration ---
// This is the network the Robot will create
#define AP_SSID  "Sesame-Controller"
#define AP_PASS  "12345678" // Must be at least 8 characters

// --- Station Mode Configuration (Optional) ---
// Set these to connect to your home/office WiFi network
// Leave NETWORK_SSID empty to disable station mode
#define NETWORK_SSID ""  // Your WiFi network name
#define NETWORK_PASS ""  // Your WiFi password
bool enableNetworkMode = false;  // Loaded from NVS in setup()

// Screen dimensions, OLED address, I2C pins, and servo pin arrays are all
// defined in board_config.h and selected at compile time via -DBOARD_*.
// See board_config.h for the full list of supported targets.


// DNS Server for Captive Portal
DNSServer dnsServer;
const byte DNS_PORT = 53;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
WebServer server(80);

// Global state for animations
String currentCommand = "";
String currentFaceName = "default";
const unsigned char* const* currentFaceFrames = nullptr;
uint8_t currentFaceFrameCount = 0;
uint8_t currentFaceFrameIndex = 0;
unsigned long lastFaceFrameMs = 0;
int faceFps = 8;
FaceAnimMode currentFaceMode = FACE_ANIM_LOOP;
int8_t faceFrameDirection = 1;
bool faceAnimFinished = false;
int currentFaceFps = 0;
bool idleActive = false;
bool idleBlinkActive = false;
unsigned long nextIdleBlinkMs = 0;
uint8_t idleBlinkRepeatsLeft = 0;

// WiFi Info Scrolling
unsigned long lastInputTime = 0;
bool firstInputReceived = false;
bool showingWifiInfo = false;
int wifiScrollPos = 0;
unsigned long lastWifiScrollMs = 0;
String wifiInfoText = "";

// Network Mode
bool networkConnected = false;
IPAddress networkIP;
String deviceHostname = "sesame-robot";

// Servo pin array — populated from SERVO_PINS defined in board_config.h.
// To add a new board, add its entry in board_config.h and pass -DBOARD_<NAME>
// to the compiler; no changes are needed here.
Servo servos[8];
const int servoPins[8] = SERVO_PINS;

// Subtrim values for each servo (offset in degrees)
int8_t servoSubtrim[8] = {0, 0, 0, 0, 0, 0, 0, 0};


// Animation constants
int frameDelay = 100;
int walkCycles = 10;
int motorCurrentDelay = 20; // ms delay between motor movements to prevent over-current

struct FaceEntry {
  const char* name;
  const unsigned char* const* frames;
  uint8_t maxFrames;
};

static const uint8_t MAX_FACE_FRAMES = 6;

#define MAKE_FACE_FRAMES(name) \
  const unsigned char* const face_##name##_frames[] = { \
    epd_bitmap_##name, epd_bitmap_##name##_1, epd_bitmap_##name##_2, \
    epd_bitmap_##name##_3, epd_bitmap_##name##_4, epd_bitmap_##name##_5 \
  };

#define X(name) MAKE_FACE_FRAMES(name)
FACE_LIST
#undef X
#undef MAKE_FACE_FRAMES

const FaceEntry faceEntries[] = {
#define X(name) { #name, face_##name##_frames, MAX_FACE_FRAMES },
  FACE_LIST
#undef X
  { "default", face_defualt_frames, MAX_FACE_FRAMES }
};

struct FaceFpsEntry {
  const char* name;
  uint8_t fps;
};

const FaceFpsEntry faceFpsEntries[] = {
  { "walk", 1 },
  { "rest", 1 },
  { "swim", 1 },
  { "dance", 1 },
  { "wave", 1 },
  { "point", 5 },
  { "stand", 1 },
  { "cute", 1 },
  { "pushup", 1 },
  { "freaky", 1 },
  { "bow", 1 },
  { "worm", 1 },
  { "shake", 1 },
  { "shrug", 1 },
  { "dead", 2 },
  { "crab", 1 },
  { "idle", 1 },
  { "idle_blink", 7 },
  { "default", 1 },
  // Conversational faces (manually controlled by Python - no auto-animation)
  { "happy", 1 },
  { "talk_happy", 1 },
  { "sad", 1 },
  { "talk_sad", 1 },
  { "angry", 1 },
  { "talk_angry", 1 },
  { "surprised", 1 },
  { "talk_surprised", 1 },
  { "sleepy", 1 },
  { "talk_sleepy", 1 },
  { "love", 1 },
  { "talk_love", 1 },
  { "excited", 1 },
  { "talk_excited", 1 },
  { "confused", 1 },
  { "talk_confused", 1 },
  { "thinking", 1 },
  { "talk_thinking", 1 },
};


// Prototypes
void setServoAngle(uint8_t channel, int angle);
void updateFaceBitmap(const unsigned char* bitmap);
void setFace(const String& faceName);
void setFaceMode(FaceAnimMode mode);
void setFaceWithMode(const String& faceName, FaceAnimMode mode);
void updateAnimatedFace();
void delayWithFace(unsigned long ms);
void enterIdle();
void exitIdle();
void updateIdleBlink();
int getFaceFpsForName(const String& faceName);
bool pressingCheck(String cmd, int ms);
void handleGetSettings();
void handleSetSettings();
void handleGetStatus();
void handleApiCommand();
void updateWifiInfoScroll();
void updateWifiInfoText();
void recordInput();

void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleCommandWeb() {
  // We send 200 OK immediately so the web browser doesn't hang waiting for animation to finish
  if (server.hasArg("pose")) {
    currentCommand = server.arg("pose");
    recordInput();
    exitIdle();
    server.send(200, "text/plain", "OK"); 
  } 
  else if (server.hasArg("go")) {
    currentCommand = server.arg("go");
    recordInput();
    exitIdle();
    server.send(200, "text/plain", "OK");
  } 
  else if (server.hasArg("stop")) {
    currentCommand = "";
    recordInput();
    server.send(200, "text/plain", "OK");
  }
  else if (server.hasArg("motor") && server.hasArg("value")) {
    int servoIdx = getServoIndex(server.arg("motor"));
    int angle = server.arg("value").toInt();
    
    if (servoIdx != -1 && angle >= 0 && angle <= 180) {
      setServoAngle(servoIdx, angle);
      recordInput();
      server.send(200, "text/plain", "OK");
    } else {
      server.send(400, "text/plain", "Invalid motor or angle");
    }
  }
  else {
    server.send(400, "text/plain", "Bad Args");
  }
}

void handleGetSettings() {
  char json[150];
  snprintf(json, sizeof(json), 
    "{\"frameDelay\":%d,\"walkCycles\":%d,\"motorCurrentDelay\":%d,\"faceFps\":%d,\"enableNetworkMode\":%s}",
    frameDelay, walkCycles, motorCurrentDelay, faceFps, enableNetworkMode ? "true" : "false");
  server.send(200, "application/json", json);
}

void handleSetSettings() {
  if (server.hasArg("frameDelay")) frameDelay = server.arg("frameDelay").toInt();
  if (server.hasArg("walkCycles")) walkCycles = server.arg("walkCycles").toInt();
  if (server.hasArg("motorCurrentDelay")) motorCurrentDelay = server.arg("motorCurrentDelay").toInt();
  if (server.hasArg("faceFps")) faceFps = (int)max(1L, server.arg("faceFps").toInt());
  
  if (server.hasArg("enableNetworkMode")) {
    enableNetworkMode = (server.arg("enableNetworkMode") == "true");
    Preferences settings;
    settings.begin("sesame-settings", false);
    settings.putBool("net_mode", enableNetworkMode);
    settings.end();
  }
  
  server.send(200, "text/plain", "OK");
}

// API endpoint for network clients to get robot status
void handleGetStatus() {
  char json[256];
  if (networkConnected) {
    snprintf(json, sizeof(json), 
      "{\"currentCommand\":\"%s\",\"currentFace\":\"%s\",\"networkConnected\":true,\"apIP\":\"%s\",\"networkIP\":\"%s\"}",
      currentCommand.c_str(), currentFaceName.c_str(), WiFi.softAPIP().toString().c_str(), networkIP.toString().c_str());
  } else {
    snprintf(json, sizeof(json), 
      "{\"currentCommand\":\"%s\",\"currentFace\":\"%s\",\"networkConnected\":false,\"apIP\":\"%s\"}",
      currentCommand.c_str(), currentFaceName.c_str(), WiFi.softAPIP().toString().c_str());
  }
  server.send(200, "application/json", json);
}

void handleApiCommand() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));

  if (error) {
    Serial.print("JSON Parse Error: ");
    Serial.println(error.c_str());
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  const char* command = doc["command"];
  const char* face = doc["face"];
  
  // Set face if provided
  if (face) {
    setFace(String(face));
  }
  
  // Check if we only have a face but no command
  bool faceOnly = (face != nullptr && command == nullptr);
  
  if (faceOnly) {
    recordInput();
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Face updated\"}");
    return;
  }
  
  if (!command) {
    server.send(400, "application/json", "{\"error\":\"Missing command field\"}");
    return;
  }
  
  // Execute command
  String cmdStr = String(command);
  if (cmdStr == "stop") {
    currentCommand = "";
    recordInput();
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Command stopped\"}");
  } else {
    currentCommand = cmdStr;
    recordInput();
    exitIdle();
    server.send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Command executed\"}");
  }
}

// ─── WiFi Provisioning ────────────────────────────────────────────────────────
Preferences wifiPrefs;
bool wifiConnecting = false;
bool wifiFailed     = false;

void tryNetworkConnect(const String& ssid, const String& pass) {
  wifiConnecting = true;
  wifiFailed     = false;
  networkConnected = false;
  WiFi.mode(WIFI_AP_STA);
  WiFi.setHostname(deviceHostname.c_str());
  WiFi.begin(ssid.c_str(), pass.c_str());
}

void loadSavedWifi() {
  wifiPrefs.begin("sesame-wifi", true);
  String ssid = wifiPrefs.getString("ssid", "");
  String pass = wifiPrefs.getString("pass", "");
  wifiPrefs.end();
  if (ssid.length() > 0) {
    Serial.println("[WiFi] Loading saved credentials for: " + ssid);
    tryNetworkConnect(ssid, pass);
  }
}

void handleWifiPage() {
  server.send_P(200, "text/html", wifi_config_html);
}

void handleWifiScan() {
  int n = WiFi.scanNetworks();
  String json = "[";
  json.reserve(n * 128); // Pre-allocate memory to avoid fragmentation
  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    ssid.replace("\\", "\\\\");
    ssid.replace("\"", "\\\"");
    if (i > 0) json += ",";
    
    char entry[150];
    snprintf(entry, sizeof(entry), 
      "{\"ssid\":\"%s\",\"rssi\":%d,\"secure\":%s}",
      ssid.c_str(), WiFi.RSSI(i), (WiFi.encryptionType(i) != WIFI_AUTH_OPEN ? "true" : "false"));
    json += entry;
  }
  json += "]";
  WiFi.scanDelete();
  server.send(200, "application/json", json);
}

void handleWifiConnect() {
  if (server.method() != HTTP_POST) {
    server.send(405, "application/json", "{\"error\":\"Method not allowed\"}");
    return;
  }
  
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  
  if (error) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }
  
  const char* ssid = doc["ssid"];
  const char* pass = doc["password"];

  if (!ssid) {
    server.send(400, "application/json", "{\"error\":\"Missing ssid\"}");
    return;
  }
  
  // Persist credentials
  wifiPrefs.begin("sesame-wifi", false);
  wifiPrefs.putString("ssid", ssid);
  wifiPrefs.putString("pass", pass ? pass : "");
  wifiPrefs.end();
  
  tryNetworkConnect(ssid, pass ? pass : "");
  server.send(200, "application/json", "{\"status\":\"connecting\"}");
}

void handleWifiStatus() {
  char json[100];
  if (networkConnected) {
    snprintf(json, sizeof(json), "{\"connected\":true,\"ip\":\"%s\"}", networkIP.toString().c_str());
  } else {
    snprintf(json, sizeof(json), "{\"connected\":false,\"failed\":%s}", wifiFailed ? "true" : "false");
  }
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Board: " BOARD_NAME));
  randomSeed(micros());
  
  // I2C Init for ESP32
  Wire.begin(I2C_SDA, I2C_SCL);

  // OLED Init
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println(F("SSD1306 allocation failed."));
    while (1);
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println(F("Setting up WiFi..."));
  display.display();

  // --- WIFI CONFIGURATION ---
  // Load settings from NVS
  Preferences settings;
  settings.begin("sesame-settings", true);
  enableNetworkMode = settings.getBool("net_mode", false);
  settings.end();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  IPAddress myIP = WiFi.softAPIP();
  
  Serial.print("AP Created. IP: ");
  Serial.println(myIP);

  if (enableNetworkMode) {
    loadSavedWifi();
    // If no saved WiFi, try the hardcoded one from the code
    if (!wifiConnecting && String(NETWORK_SSID).length() > 0) {
      Serial.println("Using hardcoded credentials from source code...");
      tryNetworkConnect(NETWORK_SSID, NETWORK_PASS);
    }
  }

  // Initial WiFi info text
  updateWifiInfoText();
  
  // Initialize input tracking
  lastInputTime = millis();
  firstInputReceived = false;
  showingWifiInfo = false;

  // Start mDNS responder for local network discovery
  if (MDNS.begin(deviceHostname.c_str())) {
    Serial.println("mDNS responder started");
    Serial.print("Access controller at: http://");
    Serial.print(deviceHostname);
    Serial.println(".local");
    MDNS.addService("http", "tcp", 80);
  } else {
    Serial.println("Error setting up mDNS responder!");
  }

  // Start DNS Server for Captive Portal
  // This redirects ALL domain requests to the ESP32's IP
  dnsServer.start(DNS_PORT, "*", myIP);

  // Removed redundant blocking loadSavedWifi() call as it's now handled non-blockingly above

  // Web Server Routes
  server.on("/", handleRoot);
  server.on("/cmd", handleCommandWeb);
  server.on("/getSettings", handleGetSettings);
  server.on("/setSettings", handleSetSettings);

  // WiFi provisioning UI
  server.on("/wifi",         HTTP_GET,  handleWifiPage);
  server.on("/wifi/scan",    HTTP_GET,  handleWifiScan);
  server.on("/wifi/connect", HTTP_POST, handleWifiConnect);
  server.on("/wifi/status",  HTTP_GET,  handleWifiStatus);
  
  // API endpoints for network communication
  server.on("/api/status",  handleGetStatus);
  server.on("/api/command", handleApiCommand);
  
  // Catch-all route for captive portal
  server.onNotFound(handleRoot);
  
  server.begin();

  // PWM Init
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  for (int i = 0; i < 8; i++) {
    servos[i].setPeriodHertz(50);
    // Map 0-180 to approx 732-2929us
    servos[i].attach(servoPins[i], 732, 2929);
  }
  delay(10);
  
  // Show rest face on startup without moving motors
  setFace("rest");
  
  Serial.println(F("HTTP server & Captive Portal started."));
}

void loop() {
  // Process DNS requests for captive portal
  dnsServer.processNextRequest();
  
  server.handleClient();

  // Poll WiFi connection state after a /wifi/connect request
  if (wifiConnecting) {
    wl_status_t s = WiFi.status();
    if (s == WL_CONNECTED) {
      networkConnected = true;
      networkIP        = WiFi.localIP();
      wifiConnecting   = false;
      wifiFailed       = false;
      updateWifiInfoText(); // Update scrolling text with new IP
      Serial.println("[WiFi] Connected! IP: " + networkIP.toString());
    } else if (s == WL_CONNECT_FAILED || s == WL_NO_SSID_AVAIL || s == WL_DISCONNECTED) {
      wifiFailed     = true;
      wifiConnecting = false;
      updateWifiInfoText(); // Update scrolling text to show failure
      Serial.println("[WiFi] Connection failed (status " + String(s) + ")");
    }
  }

  updateAnimatedFace();
  updateIdleBlink();
  updateWifiInfoScroll();

  if (currentCommand != "") {
    String cmd = currentCommand;
    if (cmd == "forward") runWalkPose();
    else if (cmd == "backward") runWalkBackward();
    else if (cmd == "left") runTurnLeft();
    else if (cmd == "right") runTurnRight();
    else if (cmd == "rest") { runRestPose(); if (currentCommand == "rest") currentCommand = ""; }
    else if (cmd == "stand") { runStandPose(1); if (currentCommand == "stand") currentCommand = ""; }
    else if (cmd == "wave") runWavePose();
    else if (cmd == "dance") runDancePose();
    else if (cmd == "swim") runSwimPose();
    else if (cmd == "point") runPointPose();
    else if (cmd == "pushup") runPushupPose();
    else if (cmd == "bow") runBowPose();
    else if (cmd == "cute") runCutePose();
    else if (cmd == "freaky") runFreakyPose();
    else if (cmd == "worm") runWormPose();
    else if (cmd == "shake") runShakePose();
    else if (cmd == "shrug") runShrugPose();
    else if (cmd == "dead") runDeadPose();
    else if (cmd == "crab") runCrabPose();
  }
  
  // Serial CLI for debugging (can be used to diagnose servo position issues and wiring)
  if (Serial.available()) {
    static char command_buffer[32];
    static byte buffer_pos = 0;
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (buffer_pos > 0) {
        command_buffer[buffer_pos] = '\0';
        int motorNum, angle;
        recordInput();
        if(strcmp(command_buffer, "run walk") == 0 || strcmp(command_buffer, "rn wf") == 0) { currentCommand = "forward"; runWalkPose(); currentCommand = ""; }
        else if(strcmp(command_buffer, "rn wb") == 0) { currentCommand = "backward"; runWalkBackward(); currentCommand = ""; }
        else if(strcmp(command_buffer, "rn tl") == 0) { currentCommand = "left"; runTurnLeft(); currentCommand = ""; }
        else if(strcmp(command_buffer, "rn tr") == 0) { currentCommand = "right"; runTurnRight(); currentCommand = ""; }
        else if(strcmp(command_buffer, "run rest") == 0 || strcmp(command_buffer, "rn rs") == 0) runRestPose();
        else if(strcmp(command_buffer, "run stand") == 0 || strcmp(command_buffer, "rn st") == 0) runStandPose(1);
        else if(strcmp(command_buffer, "rn wv") == 0) { currentCommand = "wave"; runWavePose(); }
        else if(strcmp(command_buffer, "rn dn") == 0) { currentCommand = "dance"; runDancePose(); }
        else if(strcmp(command_buffer, "rn sw") == 0) { currentCommand = "swim"; runSwimPose(); }
        else if(strcmp(command_buffer, "rn pt") == 0) { currentCommand = "point"; runPointPose(); }
        else if(strcmp(command_buffer, "rn pu") == 0) { currentCommand = "pushup"; runPushupPose(); }
        else if(strcmp(command_buffer, "rn bw") == 0) { currentCommand = "bow"; runBowPose(); }
        else if(strcmp(command_buffer, "rn ct") == 0) { currentCommand = "cute"; runCutePose(); }
        else if(strcmp(command_buffer, "rn fk") == 0) { currentCommand = "freaky"; runFreakyPose(); }
        else if(strcmp(command_buffer, "rn wm") == 0) { currentCommand = "worm"; runWormPose(); }
        else if(strcmp(command_buffer, "rn sk") == 0) { currentCommand = "shake"; runShakePose(); }
        else if(strcmp(command_buffer, "rn sg") == 0) { currentCommand = "shrug"; runShrugPose(); }
        else if(strcmp(command_buffer, "rn dd") == 0) { currentCommand = "dead"; runDeadPose(); }
        else if(strcmp(command_buffer, "rn cb") == 0) { currentCommand = "crab"; runCrabPose(); }
        else if (strcmp(command_buffer, "subtrim") == 0 || strcmp(command_buffer, "st") == 0) {
          Serial.println("Subtrim values:");
          for (int i = 0; i < 8; i++) {
            Serial.print("Motor "); Serial.print(i); Serial.print(": ");
            if (servoSubtrim[i] >= 0) Serial.print("+");
            Serial.println(servoSubtrim[i]);
          }
        }
        else if (strcmp(command_buffer, "subtrim save") == 0 || strcmp(command_buffer, "st save") == 0) {
          Serial.println("Copy and paste this into your code:");
          Serial.print("int8_t servoSubtrim[8] = {");
          for (int i = 0; i < 8; i++) {
            Serial.print(servoSubtrim[i]);
            if (i < 7) Serial.print(", ");
          }
          Serial.println("};");
        }
        else if (strncmp(command_buffer, "subtrim reset", 13) == 0 || strncmp(command_buffer, "st reset", 8) == 0) {
          for (int i = 0; i < 8; i++) servoSubtrim[i] = 0;
          Serial.println("All subtrim values reset to 0");
        }
        else if (strncmp(command_buffer, "subtrim ", 8) == 0 || strncmp(command_buffer, "st ", 3) == 0) {
          const char* params = (command_buffer[1] == 't') ? command_buffer + 3 : command_buffer + 8;
          int trimMotor, trimValue;
          if (sscanf(params, "%d %d", &trimMotor, &trimValue) == 2) {
            if (trimMotor >= 0 && trimMotor < 8) {
              if (trimValue >= -90 && trimValue <= 90) {
                servoSubtrim[trimMotor] = trimValue;
                Serial.print("Motor "); Serial.print(trimMotor); Serial.print(" subtrim set to ");
                if (trimValue >= 0) Serial.print("+");
                Serial.println(trimValue);
              } else {
                Serial.println("Subtrim value must be between -90 and +90");
              }
            } else {
              Serial.println("Invalid motor number (0-7)");
            }
          }
        }
        else if (strncmp(command_buffer, "all ", 4) == 0) {
             if (sscanf(command_buffer + 4, "%d", &angle) == 1) {
                 for (int i = 0; i < 8; i++) setServoAngle(i, angle);
                 Serial.print("All servos set to "); Serial.println(angle);
             }
        }
        else if (sscanf(command_buffer, "%d %d", &motorNum, &angle) == 2) {
             if (motorNum >= 0 && motorNum < 8) {
                 setServoAngle(motorNum, angle);
                 Serial.print("Servo "); Serial.print(motorNum); Serial.print(" set to "); Serial.println(angle);
             } else {
                 Serial.println("Invalid motor number (0-7)");
             }
        }
        buffer_pos = 0;
      }
    } else if (buffer_pos < sizeof(command_buffer) - 1) {
      command_buffer[buffer_pos++] = c;
    }
  }
}

// Function to update the robot's face
void updateFaceBitmap(const unsigned char* bitmap) {
  display.clearDisplay();
  display.drawBitmap(0, 0, bitmap, 128, 64, SSD1306_WHITE);
  display.display();
}

uint8_t countFrames(const unsigned char* const* frames, uint8_t maxFrames) {
  if (frames == nullptr || frames[0] == nullptr) return 0;
  uint8_t count = 0;
  for (uint8_t i = 0; i < maxFrames; i++) {
    if (frames[i] == nullptr) break;
    count++;
  }
  return count;
}

void setFace(const String& faceName) {
  if (faceName == currentFaceName && currentFaceFrames != nullptr) return;

  currentFaceName = faceName;
  currentFaceFrameIndex = 0;
  lastFaceFrameMs = 0;
  faceFrameDirection = 1;
  faceAnimFinished = false;
  currentFaceFps = getFaceFpsForName(faceName);

  currentFaceFrames = face_defualt_frames;
  currentFaceFrameCount = countFrames(face_defualt_frames, MAX_FACE_FRAMES);

  for (size_t i = 0; i < (sizeof(faceEntries) / sizeof(faceEntries[0])); i++) {
    if (faceName.equalsIgnoreCase(faceEntries[i].name)) {
      currentFaceFrames = faceEntries[i].frames;
      currentFaceFrameCount = countFrames(faceEntries[i].frames, faceEntries[i].maxFrames);
      break;
    }
  }

  if (currentFaceFrameCount == 0) {
    currentFaceFrames = face_defualt_frames;
    currentFaceFrameCount = countFrames(face_defualt_frames, MAX_FACE_FRAMES);
    currentFaceName = "default";
    currentFaceFps = getFaceFpsForName(currentFaceName);
  }

  if (currentFaceFrameCount > 0 && currentFaceFrames[0] != nullptr) {
    updateFaceBitmap(currentFaceFrames[0]);
  }
}

void setFaceMode(FaceAnimMode mode) {
  currentFaceMode = mode;
  faceFrameDirection = 1;
  faceAnimFinished = false;
}

void setFaceWithMode(const String& faceName, FaceAnimMode mode) {
  setFaceMode(mode);
  setFace(faceName);
}

int getFaceFpsForName(const String& faceName) {
  for (size_t i = 0; i < (sizeof(faceFpsEntries) / sizeof(faceFpsEntries[0])); i++) {
    if (faceName.equalsIgnoreCase(faceFpsEntries[i].name)) {
      return faceFpsEntries[i].fps;
    }
  }
  return faceFps;
}

void updateAnimatedFace() {
  if (currentFaceFrames == nullptr || currentFaceFrameCount <= 1) return;
  if (currentFaceMode == FACE_ANIM_ONCE && faceAnimFinished) return;

  unsigned long now = millis();
  int fps = max(1, (currentFaceFps > 0 ? currentFaceFps : faceFps));
  unsigned long interval = 1000UL / fps;
  if (now - lastFaceFrameMs >= interval) {
    lastFaceFrameMs = now;
    if (currentFaceMode == FACE_ANIM_LOOP) {
      currentFaceFrameIndex = (currentFaceFrameIndex + 1) % currentFaceFrameCount;
    } else if (currentFaceMode == FACE_ANIM_ONCE) {
      if (currentFaceFrameIndex + 1 >= currentFaceFrameCount) {
        currentFaceFrameIndex = currentFaceFrameCount - 1;
        faceAnimFinished = true;
      } else {
        currentFaceFrameIndex++;
      }
    } else {
      if (faceFrameDirection > 0) {
        if (currentFaceFrameIndex + 1 >= currentFaceFrameCount) {
          faceFrameDirection = -1;
          if (currentFaceFrameIndex > 0) currentFaceFrameIndex--;
        } else {
          currentFaceFrameIndex++;
        }
      } else {
        if (currentFaceFrameIndex == 0) {
          faceFrameDirection = 1;
          if (currentFaceFrameCount > 1) currentFaceFrameIndex++;
        } else {
          currentFaceFrameIndex--;
        }
      }
    }
    updateFaceBitmap(currentFaceFrames[currentFaceFrameIndex]);
  }
}

void delayWithFace(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    updateAnimatedFace();
    server.handleClient();
    dnsServer.processNextRequest();
    delay(5);
  }
}

void scheduleNextIdleBlink(unsigned long minMs, unsigned long maxMs) {
  unsigned long now = millis();
  unsigned long interval = (unsigned long)random(minMs, maxMs);
  nextIdleBlinkMs = now + interval;
}

void enterIdle() {
  idleActive = true;
  idleBlinkActive = false;
  idleBlinkRepeatsLeft = 0;
  setFaceWithMode("idle", FACE_ANIM_BOOMERANG);
  scheduleNextIdleBlink(3000, 7000);
}

void exitIdle() {
  idleActive = false;
  idleBlinkActive = false;
}

void updateIdleBlink() {
  if (!idleActive) return;

  if (!idleBlinkActive) {
    if (millis() >= nextIdleBlinkMs) {
      idleBlinkActive = true;
      if (idleBlinkRepeatsLeft == 0 && random(0, 100) < 30) {
        idleBlinkRepeatsLeft = 1; // double blink
      }
      setFaceWithMode("idle_blink", FACE_ANIM_ONCE);
    }
    return;
  }

  if (currentFaceMode == FACE_ANIM_ONCE && faceAnimFinished) {
    idleBlinkActive = false;
    setFaceWithMode("idle", FACE_ANIM_BOOMERANG);
    if (idleBlinkRepeatsLeft > 0) {
      idleBlinkRepeatsLeft--;
      scheduleNextIdleBlink(120, 220);
    } else {
      scheduleNextIdleBlink(3000, 7000);
    }
  }
}

// ====== HELPERS ======
void setServoAngle(uint8_t channel, int angle) { 
  if (channel < 8) {
    int adjustedAngle = constrain(angle + servoSubtrim[channel], 0, 180);
    servos[channel].write(adjustedAngle);
    delayWithFace(motorCurrentDelay);
  }
}

bool pressingCheck(String cmd, int ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    server.handleClient();
    dnsServer.processNextRequest();
    updateAnimatedFace();
    if (currentCommand != cmd) {
      runStandPose(1);
      return false;
    }
    yield();
  }
  return true;
}

void recordInput() {
  lastInputTime = millis();
  if (!firstInputReceived) {
    firstInputReceived = true;
    showingWifiInfo = false;
  }
}

void updateWifiInfoScroll() {
  // Don't show WiFi info if first input has been received
  if (firstInputReceived) {
    if (showingWifiInfo) {
      showingWifiInfo = false;
      // Restore the current face
      if (currentFaceFrames != nullptr && currentFaceFrameCount > 0) {
        updateFaceBitmap(currentFaceFrames[currentFaceFrameIndex]);
      }
    }
    return;
  }
  
  unsigned long now = millis();
  
  // Check if 30 seconds have passed without input
  if (!showingWifiInfo && (now - lastInputTime >= 30000)) {
    showingWifiInfo = true;
    wifiScrollPos = 0;
    lastWifiScrollMs = now;
  }
  
  if (!showingWifiInfo) return;
  
  // Update scroll every 150ms
  if (now - lastWifiScrollMs >= 150) {
    lastWifiScrollMs = now;
    
    // Clear and redraw with current face in background
    display.clearDisplay();
    
    // Draw the face bitmap in the background
    if (currentFaceFrames != nullptr && currentFaceFrameCount > 0) {
      display.drawBitmap(0, 0, currentFaceFrames[currentFaceFrameIndex], 128, 64, SSD1306_WHITE);
    }
    
    // Draw black bar for text background on top row
    display.fillRect(0, 0, 128, 10, SSD1306_BLACK);
    
    // Draw scrolling text
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);
    display.setCursor(-wifiScrollPos, 1);
    display.print(wifiInfoText);
    display.setTextWrap(true);
    
    display.display();
    
    // Advance scroll position
    wifiScrollPos += 2;
    if (wifiScrollPos >= (int)(wifiInfoText.length() * 6)) {
      wifiScrollPos = 0;
    }
  }
}
void updateWifiInfoText() {
  IPAddress myIP = WiFi.softAPIP();
  if (networkConnected) {
    wifiInfoText = "AP: " + String(AP_SSID) + " (" + myIP.toString() + ")  |  Network: " + WiFi.SSID() + " (" + networkIP.toString() + ") or " + deviceHostname + ".local  |  ";
  } else if (wifiConnecting) {
    wifiInfoText = "Connecting to WiFi...  |  AP: " + String(AP_SSID) + " (" + myIP.toString() + ")  |  ";
  } else {
    wifiInfoText = "Connect to WiFi: " + String(AP_SSID) + "  |  Pass: " + String(AP_PASS) + "  |  IP: " + myIP.toString() + "  |  Captive Portal will auto-open!  |  ";
  }
}
