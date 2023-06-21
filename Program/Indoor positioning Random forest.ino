#include <stdio.h>
#include <string.h>

#include <Wire.h>
#include <MPU6050.h>

#include <Adafruit_SSD1306.h>

#include  <ESP8266WiFi.h>
#include "eloquent.h"
#include "eloquent/networking/wifi/WifiScanner.h"

//Firebase :
#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Ananth 4G"
#define WIFI_PASSWORD "Qwerty@123"

#define API_KEY "AIzaSyBUhS6jIPs8J6IFcy7y65NLO4RqbXWEhGs"

#define DATABASE_URL "newonne-34ac9-default-rtdb.firebaseio.com" 

FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int led=0,count = 0;
bool signupOK = false;

// MPU:

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);
MPU6050 mpu;
int16_t ax, ay, az; // Acceleration values
int16_t ax_offset, ay_offset, az_offset; // Acceleration offsets


class WiFiIndoorPositioning {
    public:

        /**
         * Get current room id
         */
        template<typename Scanner>
        int predictRoomId(Scanner& scanner) {
            float x[2] = {0};
            uint16_t votes[2] = { 0 };

            scanner.scan();
            convertToFeatures(scanner, x);

            if (x[0] <= -67.5) {   votes[0] += 1;  } else {  if (x[0] <= -63.5) {  if (x[0] <= -65.5) {   votes[1] += 1;  } else {   votes[0] += 1;  }   } else {   votes[1] += 1;  }   } 
            if (x[0] <= -63.5) {  if (x[1] <= -81.5) {   votes[1] += 1;  } else {  if (x[0] <= -67.5) {   votes[0] += 1;  } else {  if (x[0] <= -66.5) {   votes[1] += 1;  } else {   votes[0] += 1;  }   }   }   } else {   votes[1] += 1;  } 
            if (x[0] <= -63.5) {  if (x[1] <= -81.5) {   votes[1] += 1;  } else {  if (x[0] <= -67.5) {   votes[0] += 1;  } else {  if (x[1] <= -74.0) {   votes[1] += 1;  } else {   votes[0] += 1;  }   }   }   } else {   votes[1] += 1;  } 
            if (x[1] <= -77.5) {   votes[1] += 1;  } else {   votes[0] += 1;  } 
            if (x[1] <= -77.0) {   votes[1] += 1;  } else {  if (x[0] <= -63.5) {   votes[0] += 1;  } else {   votes[1] += 1;  }   } 
            if (x[0] <= -67.0) {   votes[0] += 1;  } else {  if (x[1] <= -76.5) {   votes[1] += 1;  } else {   votes[0] += 1;  }   } 
            if (x[0] <= -63.5) {  if (x[0] <= -71.0) {   votes[0] += 1;  } else {  if (x[0] <= -66.5) {  if (x[1] <= -74.5) {   votes[1] += 1;  } else {   votes[0] += 1;  }   } else {  if (x[0] <= -65.5) {  if (x[1] <= -75.0) {   votes[1] += 1;  } else {   votes[0] += 1;  }   } else {   votes[0] += 1;  }   }   }   } else {   votes[1] += 1;  } 
            if (x[1] <= -73.0) {  if (x[0] <= -67.5) {  if (x[0] <= -69.0) {   votes[1] += 1;  } else {   votes[0] += 1;  }   } else {   votes[1] += 1;  }   } else {   votes[0] += 1;  } 
            if (x[1] <= -76.5) {  if (x[1] <= -80.5) {   votes[1] += 1;  } else {  if (x[1] <= -79.5) {   votes[0] += 1;  } else {   votes[1] += 1;  }   }   } else {  if (x[0] <= -63.5) {   votes[0] += 1;  } else {   votes[1] += 1;  }   } 
            if (x[1] <= -77.0) {  if (x[0] <= -67.0) {  if (x[0] <= -69.0) {   votes[1] += 1;  } else {   votes[0] += 1;  }   } else {   votes[1] += 1;  }   } else {   votes[0] += 1;  } 
            

            // return argmax of votes
            uint8_t classIdx = 0;
            float maxVotes = votes[0];

            for (uint8_t i = 1; i < 2; i++) {
                if (votes[i] > maxVotes) {
                    classIdx = i;
                    maxVotes = votes[i];
                }
            }

            return (lastRoomId = classIdx);
        }

        /**
         * Get current room name
         */
        template<typename Scanner>
        String predictRoomName(Scanner& scanner) {
            uint8_t roomId = predictRoomId(scanner);

            switch (roomId) {
                
                case 0: return (lastRoomName = "Room1");
                
                case 1: return (lastRoomName = "Room2");
                
                default: return (lastRoomName = "???");
            }
        }

        /**
         * Test if current location is the given one by id
         */
        bool isIn(uint8_t roomId) {
            return roomId == lastRoomId;
        }

        /**
         * Test if current location is the given one by name
         */
        bool isIn(String roomName) {
            return roomName == lastRoomName;
        }


    protected:
        uint8_t lastRoomId = 255;
        String lastRoomName;

        template<typename Scanner>
        void convertToFeatures(Scanner& scanner, float *x) {
            while (scanner.hasNext()) {
                String ssid = scanner.ssid();
                float rssi = scanner.rssi();

                scanner.next();

                
                if (ssid == "MALWARE!") {
                    x[0] = rssi;
                    continue;
                }
                
                if (ssid == "Node1") {
                    x[1] = rssi;
                    continue;
                }
                
            }
        }
};


WiFiIndoorPositioning positioning;


void setup() {
    Serial.begin(115200);
    pinMode(led, OUTPUT);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x64
      Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; 
  }
 display.display();
  delay(500); // Pause for 2 seconds
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setRotation(0);
//Fire base 

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);


//MPU :
  Wire.begin();

  mpu.initialize(); // Initialize the MPU6050
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);
  mpu.setZGyroOffset(0);
  
  delay(1000); // Wait for the sensor to stabilize
  
  calibrateSensor(); // Calibrate the sensor
    Serial.println("Instructions");
    Serial.println("\tMove around your house/space and read the detected room/zone on the Serial monitor");
}


void loop() {
  mpu.getAcceleration(&ax, &ay, &az);
  display.clearDisplay();
  display.setCursor(0,0);
    Serial.print("Current location: ");
    Serial.println(positioning.predictRoomName(wifiScanner));
    display.println(positioning.predictRoomName(wifiScanner));
   // Firebase.setString("Temp",positioning.predictRoomName(wifiScanner));
    Firebase.RTDB.setString(&fbdo, "Temp", (positioning.predictRoomName(wifiScanner)));
       
    if (positioning.isIn("Room 1")) {
        Serial.println("You are currently in Room 1");
         display.println("You are currently in Room 1");
         //Firebase.setString("Temp","Room 1");
         Firebase.RTDB.setString(&fbdo, "Temp", "Room1");
       
    }

    // customize as per your needs
   // Read acceleration values
  
  // Calculate the difference between the current acceleration values and the offsets
  int16_t dx = abs(ax - ax_offset);
  int16_t dy = abs(ay - ay_offset);
  int16_t dz = abs(az - az_offset);
  if (dx > 10000 || dy > 10000 || dz > 10000) {
    Serial.println("Fall detected!");
    Firebase.RTDB.setString(&fbdo, "Humidity","Fall_detected");
    // Add code here to send an alert or take other action
  }
    
    //Add your fall detection algorithm here
  
  display.display();
  delay(5000);
}

void calibrateSensor() {
  Serial.println("Calibrating sensor...");
  
  // Read 1000 acceleration values and calculate the average
  int16_t ax_sum = 0, ay_sum = 0, az_sum = 0;
  for (int i = 0; i < 1000; i++) {
    mpu.getAcceleration(&ax, &ay, &az);
    ax_sum += ax;
    ay_sum += ay;
    az_sum += az;
    delay(1);
  }
  ax_offset = ax_sum / 1000;
  ay_offset = ay_sum / 1000;
  az_offset = az_sum / 1000;
  
  Serial.println("Sensor calibration complete.");
  Serial.print("ax_offset = "); Serial.println(ax_offset);
  Serial.print("ay_offset = "); Serial.println(ay_offset);
  Serial.print("az_offset = "); Serial.println(az_offset);
}