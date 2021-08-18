#include <SPI.h>
#include <RH_RF69.h>

RH_RF69 rf69;

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

unsigned long previousMillis = 0;
int redLedState = LOW;

enum settings
{
	TIMEOUT_ACK = 500
};

enum states
{
	IDLE,
	NEED_YOU_NOW,
	NEED_YOU_IN_A_BIT,
	LOVE_YOU,
	OK,
	ERROR
};

enum states state;

enum pins
{
	BUTTON_NOW_PIN = A0,
	BUTTON_BIT_PIN = A1,
	BUTTON_LOVE_PIN = A2,
	LIGHT_GREEN_PIN = 5,
	LIGHT_BLUE_PIN = 9,
	LIGHT_RED_PIN = 6,
	BUZZER_PIN = 4
};

enum colors
{
	RED,
	GREEN,
	BLUE,
	YELLOW,
	PURPLE
};

const char NEED_YOU_NOW_MSG[] = "need you now";
const char NEED_YOU_IN_A_BIT_MSG[] = "need you in a bit";
const char LOVE_YOU_MSG[] = "love you";
const char OK_MSG[] = "ok";
const char ACK_MSG[] = "ack";

void setupRF() {
	Serial.println("INIT");
	if (!rf69.init())
    	Serial.println("init failed");
	Serial.println("AFTER INIT");
	// Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
	// No encryption
	if (!rf69.setFrequency(915.0))
		Serial.println("setFrequency failed");
	Serial.println("AFTER FREQUENCY");

	// If you are using a high power RF69, you *must* set a Tx power in the
	// range 14 to 20 like this:
	rf69.setTxPower(14);
	Serial.println("AFTER SET POWER");

	// The encryption key has to be the same as the one in the client
	uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
					0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	rf69.setEncryptionKey(key);
	Serial.println("AFTER ENCRYPTION KEY");
}

void setup() {
	pinMode(BUTTON_NOW_PIN, INPUT);
	pinMode(BUTTON_BIT_PIN, INPUT);
	pinMode(BUTTON_LOVE_PIN, INPUT);
	pinMode(LIGHT_GREEN_PIN, OUTPUT);
	pinMode(LIGHT_BLUE_PIN, OUTPUT);
	pinMode(LIGHT_RED_PIN, OUTPUT);
	Serial.begin(115200);

	setupRF();
	state = IDLE;
	playIntro();
};

bool checkClick(int PIN){
	if (digitalRead(PIN) == HIGH) {
		delay(50);
		if (digitalRead(PIN) == HIGH) {
			return true;
		} else {
			return false;
		}
	} else {
		return false;
	}
};

void turnLightsOff() {
	digitalWrite(LIGHT_RED_PIN, LOW);
	digitalWrite(LIGHT_GREEN_PIN, LOW);
	digitalWrite(LIGHT_BLUE_PIN, LOW);
}

void setLight(enum colors color) {
	switch (color) {
		case RED:
			digitalWrite(LIGHT_RED_PIN, HIGH);
			digitalWrite(LIGHT_GREEN_PIN, LOW);
			digitalWrite(LIGHT_BLUE_PIN, LOW);
			break;
		case GREEN:
			digitalWrite(LIGHT_RED_PIN, LOW);
			digitalWrite(LIGHT_GREEN_PIN, HIGH);
			digitalWrite(LIGHT_BLUE_PIN, LOW);
			break;
		case BLUE:
			digitalWrite(LIGHT_RED_PIN, LOW);
			digitalWrite(LIGHT_GREEN_PIN, LOW);
			digitalWrite(LIGHT_BLUE_PIN, HIGH);
			break;
		case YELLOW:
			digitalWrite(LIGHT_RED_PIN, HIGH);
			analogWrite(LIGHT_GREEN_PIN, 60);
			digitalWrite(LIGHT_BLUE_PIN, LOW);
			break;
		case PURPLE:
			digitalWrite(LIGHT_RED_PIN, HIGH);
			digitalWrite(LIGHT_GREEN_PIN, LOW);
			digitalWrite(LIGHT_BLUE_PIN, HIGH);
			break;
		default:
			break;
	}
}

void flashRed() {
	unsigned long currentMillis = millis();
	if (currentMillis - previousMillis >= 1000) {
		previousMillis = currentMillis;
		if (redLedState == LOW) {
			redLedState = HIGH;
			setLight(RED);
		} else {
			redLedState = LOW;
			turnLightsOff();
		}
	}
}

void flashYellow() {
	setLight(YELLOW);
}

void flashPurple() {
	for (int i = 0; i < 15; ++i) {
		setLight(PURPLE);
		delay(50);
		turnLightsOff();
		delay(50);
	}
}

void flashGreen() {
	setLight(GREEN);
	delay(500);
	turnLightsOff();
	delay(500);
	setLight(GREEN);
	delay(500);
	turnLightsOff();
	delay(500);
}

void flashError() {
	setLight(RED);
	delay(500);
	turnLightsOff();
	delay(500);
	setLight(RED);
	delay(500);
	turnLightsOff();
	delay(500);
}

void sendAck() {
	rf69.send(ACK_MSG, sizeof(ACK_MSG));
	rf69.waitPacketSent();
	Serial.println("Sent ACK message");
}

void playIntro() {
	// notes in the melody:
	int melody[] = {

	NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
	};

	// note durations: 4 = quarter note, 8 = eighth note, etc.:
	int noteDurations[] = {

	4, 8, 8, 4, 4, 4, 4, 4
	};
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(BUZZER_PIN, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);

    // stop the tone playing:
    noTone(BUZZER_PIN);

  }
}
void listeningForMsg() {
	if (rf69.available()) {
		// Should be a message for us now   
		uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
		uint8_t len = sizeof(buf);

		if (rf69.recv(buf, &len)) {
		//      RH_RF69::printBuffer("request: ", buf, len);
			Serial.print("got request: ");
			Serial.println((char*)buf);

			if (strncmp(buf, NEED_YOU_NOW_MSG, sizeof(NEED_YOU_NOW_MSG)) == 0) {
				state = NEED_YOU_NOW;
				sendAck();
			}
			else if (strncmp(buf, NEED_YOU_IN_A_BIT_MSG, sizeof(NEED_YOU_IN_A_BIT_MSG)) == 0)
			{
				state = NEED_YOU_IN_A_BIT;
				sendAck();
			}
			else if (strncmp(buf, LOVE_YOU_MSG, sizeof(LOVE_YOU_MSG)) == 0)
			{
				state = LOVE_YOU;
				sendAck();
			}
			else if (strncmp(buf, OK_MSG, sizeof(OK_MSG)) == 0)
			{
				state = OK;
				sendAck();
			}
			Serial.print("RSSI: ");
			Serial.println(rf69.lastRssi(), DEC);
		} else {
			state = ERROR;
			Serial.println("recv failed");
		}
	}
}

void clickOk() {
	if (checkClick(BUTTON_NOW_PIN) || checkClick(BUTTON_BIT_PIN) || checkClick(BUTTON_LOVE_PIN)) {
		Serial.println("OK button Pressed");
		rf69.send(OK_MSG, sizeof(OK_MSG));
		rf69.waitPacketSent();
		Serial.println("Sent a message");
		state = IDLE;
		while (checkClick(BUTTON_NOW_PIN) || checkClick(BUTTON_BIT_PIN) || checkClick(BUTTON_LOVE_PIN));
	}
}

void recvfromAckTimeout() {
    unsigned long starttime = millis();
    while ((TIMEOUT_ACK - (millis() - starttime)) > 0)
	{
		if (rf69.available()) {
			// Should be a message for us now   
			uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
			uint8_t len = sizeof(buf);

			if (rf69.recv(buf, &len)) {
				if (strncmp(buf, ACK_MSG, sizeof(ACK_MSG)) == 0) {
					return;
				}
			}
		}
	} 
    state = ERROR;
}

void clickHandling() {
	if (checkClick(BUTTON_NOW_PIN)) {
		Serial.println("Button Pressed");
		rf69.send(NEED_YOU_NOW_MSG, sizeof(NEED_YOU_NOW_MSG));
		rf69.waitPacketSent();
		recvfromAckTimeout();
		Serial.println("Sent a message");
		while (checkClick(BUTTON_NOW_PIN))
			;
	} else if (checkClick(BUTTON_BIT_PIN)) {
		Serial.println("Button Pressed");
		rf69.send(NEED_YOU_IN_A_BIT_MSG, sizeof(NEED_YOU_IN_A_BIT_MSG));
		rf69.waitPacketSent();
		recvfromAckTimeout();
		Serial.println("Sent a message");
		while (checkClick(BUTTON_BIT_PIN))
			;
	} else if (checkClick(BUTTON_LOVE_PIN)) {
		Serial.println("Button Pressed");
		rf69.send(LOVE_YOU_MSG, sizeof(LOVE_YOU_MSG));
		rf69.waitPacketSent();
		recvfromAckTimeout();
		Serial.println("Sent a message");
		while (checkClick(BUTTON_LOVE_PIN))
			;
	}
}

void loop() {
	switch (state) {
		case IDLE:
			turnLightsOff();
			listeningForMsg();
			clickHandling();
			break;
		case NEED_YOU_NOW:
			flashRed();
			listeningForMsg();
			clickOk();
			break;
		case NEED_YOU_IN_A_BIT:
			flashYellow();
			listeningForMsg();
			clickOk();
			break;
		case LOVE_YOU:
			flashPurple();
			state = IDLE;
			break;
		case OK:
			flashGreen();
			state = IDLE;
			break;
		case ERROR:
			flashError();
			state = IDLE;
			break;
		default:
			break;
	}

}