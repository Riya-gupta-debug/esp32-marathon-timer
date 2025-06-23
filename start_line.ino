//Start Line
#define BLYNK_TEMPLATE_ID "TMPL3qTE7ANmU"
#define BLYNK_TEMPLATE_NAME "unknown runners"
#define BLYNK_AUTH_TOKEN "WL5R-YxmO3c7Z1xjXse48NT0lRHusZ0p"

#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <BlynkSimpleEsp32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define SS_PIN 5
#define RST_PIN 22

MFRC522 rfid(SS_PIN, RST_PIN);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // IST Timezone

const char* ssid = "Abbb";
const char* password = "Princi@9";

struct Runner {
    String id;
    String startTime;
};

Runner runners[10];  // Array to store data of up to 10 runners
int runnerCount = 0;

void setup() {
    Serial.begin(9600);
    SPI.begin();
    rfid.PCD_Init();
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
    timeClient.begin();

    Serial.println("RFID Start Line Ready!");
}

void loop() {
    Blynk.run();
    timeClient.update();

    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

    String runnerID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
        runnerID += String(rfid.uid.uidByte[i], HEX);
    }


	bool runnerExist = false;
	for (int i = 0; i < runnerCount; i++) {
		// Check each runner in the array (example: print the IDs)
		if(runners[i].id == runnerID){
			Serial.print("\nRunner Id: "+ runnerID +" is already scanned.");
			Blynk.virtualWrite(V0, "\nRunner " + runnerID + " is already scanned.");
			runnerExist = true;
			break;
		}
	}

	if (!Blynk.connected()) {
		Serial.println("Blynk not connected, retrying...");
	}


	if(!runnerExist && Blynk.connected()){
		
		String startTime = timeClient.getFormattedTime();
		runners[runnerCount].id = runnerID;
    runners[runnerCount].startTime = startTime;
    runnerCount++;
		String data = runnerID + "," + String(startTime);
    Blynk.virtualWrite(V10, data);      // Save runner ID for finish line
    Blynk.virtualWrite(V0, "\nRunner: " + runnerID + " | Start Time: " + startTime);  // Show in app
		Serial.print("\n  Runner " + runnerID + " Start Time: " + startTime);
	}
    
    rfid.PICC_HaltA();
    delay(3000);
}
