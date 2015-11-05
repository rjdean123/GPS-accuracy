#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <TinyGPS++.h>

#define RXPin 8
#define TXPin 3
#define GPSBaud 9600
#define SAT_MAX 32

SoftwareSerial ss(RXPin, TXPin);
TinyGPSPlus gps;
TinyGPSCustom prnMsg[4];
TinyGPSCustom elevMsg[4];
TinyGPSCustom azimMsg[4];
LiquidCrystal lcd(12, 11, 10, 7, 6, 5, 4);

typedef struct {
	int elev;
	int azim;
	int prn;
	bool found;
} FoundSat;

double latt;
double lon;
int numFound;
FoundSat sats[SAT_MAX];

void setup(void);
void loop(void);
void readGPS(void);
void update(void);
void render(void);
void gpgsvHandler(void);

void setup() {
	//variable initialization
	latt = 0.0;
	lon = 0.0;
	numFound = 0;

	//sets all 32 possible sats to not found
	for (int i = 0; i < SAT_MAX; i++) {
		sats[i].found = false;
	}

	//initializes custom nmea parsers
	prnMsg[0].begin(gps, "GPGSV", 4);
	prnMsg[1].begin(gps, "GPGSV", 8);
	prnMsg[2].begin(gps, "GPGSV", 12);
	prnMsg[3].begin(gps, "GPGSV", 16);
	elevMsg[0].begin(gps, "GPGSV", 5);
	elevMsg[1].begin(gps, "GPGSV", 9);
	elevMsg[2].begin(gps, "GPGSV", 13);
	elevMsg[3].begin(gps, "GPGSV", 17);
	azimMsg[0].begin(gps, "GPGSV", 6);
	azimMsg[1].begin(gps, "GPGSV", 10);
	azimMsg[2].begin(gps, "GPGSV", 14);
	azimMsg[3].begin(gps, "GPGSV", 18);

	//lcd initial setup
	pinMode(9, OUTPUT);
	analogWrite(9, 100);
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);

	//display intial screen
	lcd.begin(16,2);
  	lcd.clear();
  	lcd.setCursor(0,0);
  	lcd.print("Searching for");
  	lcd.setCursor(6,2);
  	lcd.print("Signal ...");
  	ss.begin(GPSBaud);	
}

void loop() {
	readGPS();
	update();
}

/** Receives data from the gps if available */
void readGPS() {
	while (ss.available() > 0) {
		gps.encode(ss.read());
		if (azimMsg[3].isUpdated())
			gpgsvHandler();
	}
}

/** Processes the data read by readGPS() */
void update() {
	if (gps.location.isUpdated()) {
		latt = gps.location.lat();
		lon = gps.location.lng();
		render();
	}
}

/** Displays info on the lcd */
void render() {
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Lat: ");
	lcd.print(latt);
	lcd.setCursor(14,0);
	lcd.print(gps.satellites.value());	
	lcd.setCursor(0,1);
	lcd.print("Lng:");
	lcd.print(lon);
	lcd.setCursor(14,1);
	lcd.print(numFound);
}

/** Checks if satellite exists in array, adds if not */
void gpgsvHandler() {
	for (int i = 0; i < 4; i++) {
		for (int n = 0; n < SAT_MAX; n++) {
			if (!sats[n].found) {
				sats[n].prn = *prnMsg[i].value();
				sats[n].elev = *elevMsg[i].value();
				sats[n].azim = *azimMsg[i].value();
				sats[n].found = true;
				numFound++;
				break;
			}
			if (*prnMsg[i].value() == sats[n].prn) {
				break;
			}
		}
	}
}