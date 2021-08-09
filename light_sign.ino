#include <SPI.h>
#include <RH_RF69.h>

RH_RF69 rf69;

unsigned long previousMillis = 0;
int redLedState = LOW;

enum states
{
	IDLE,
	NEED_YOU_NOW,
	NEED_YOU_IN_A_BIT,
	LOVE_YOU,
	ACKNOWLEDGED
};

enum states state;

enum pins
{
	BUTTON_NOW_PIN = 4,
	BUTTON_BIT_PIN = 7,
	BUTTON_LOVE_PIN = 8,
	LIGHT_GREEN_PIN = 5,
	LIGHT_BLUE_PIN = 9,
	LIGHT_RED_PIN = 6
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
const char ACK_MSG[] = "ok";

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

void listeningForMsg() {
	if (rf69.available()) {
		// Should be a message for us now   
		uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
		uint8_t len = sizeof(buf);

		if (rf69.recv(buf, &len)) {
		//      RH_RF69::printBuffer("request: ", buf, len);
			Serial.print("got request: ");
			Serial.println((char*)buf);
			Serial.println(NEED_YOU_NOW_MSG);
			Serial.println(sizeof(NEED_YOU_NOW_MSG));

			if (strncmp(buf, NEED_YOU_NOW_MSG, sizeof(NEED_YOU_NOW_MSG)) == 0) {
				state = NEED_YOU_NOW;
			} else if (strncmp(buf, NEED_YOU_IN_A_BIT_MSG, sizeof(NEED_YOU_IN_A_BIT_MSG)) == 0) {
				state = NEED_YOU_IN_A_BIT;
			} else if (strncmp(buf, LOVE_YOU_MSG, sizeof(LOVE_YOU_MSG)) == 0) {
				state = LOVE_YOU;
			} else if (strncmp(buf, ACK_MSG, sizeof(ACK_MSG)) == 0) {
				state = ACKNOWLEDGED;
			}
			Serial.print("RSSI: ");
			Serial.println(rf69.lastRssi(), DEC);
		} else {
			Serial.println("recv failed");
		}
	}
}

void clickAck() {
	if (checkClick(BUTTON_NOW_PIN) || checkClick(BUTTON_BIT_PIN) || checkClick(BUTTON_LOVE_PIN)) {
		Serial.println("Ack button Pressed");
		rf69.send(ACK_MSG, sizeof(ACK_MSG));
		rf69.waitPacketSent();
		Serial.println("Sent a message");
		state = IDLE;
		while (checkClick(BUTTON_NOW_PIN) || checkClick(BUTTON_BIT_PIN) || checkClick(BUTTON_LOVE_PIN));
	}
}

void clickHandling() {
	if (checkClick(BUTTON_NOW_PIN)) {
		Serial.println("Button Pressed");
		rf69.send(NEED_YOU_NOW_MSG, sizeof(NEED_YOU_NOW_MSG));
		rf69.waitPacketSent();
		Serial.println("Sent a message");
		while (checkClick(BUTTON_NOW_PIN))
			;
	} else if (checkClick(BUTTON_BIT_PIN)) {
		Serial.println("Button Pressed");
		rf69.send(NEED_YOU_IN_A_BIT_MSG, sizeof(NEED_YOU_IN_A_BIT_MSG));
		rf69.waitPacketSent();
		Serial.println("Sent a message");
		while (checkClick(BUTTON_BIT_PIN))
			;
	} else if (checkClick(BUTTON_LOVE_PIN)) {
		Serial.println("Button Pressed");
		rf69.send(LOVE_YOU_MSG, sizeof(LOVE_YOU_MSG));
		rf69.waitPacketSent();
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
			clickAck();
			break;
		case NEED_YOU_IN_A_BIT:
			flashYellow();
			listeningForMsg();
			clickAck();
			break;
		case LOVE_YOU:
			flashPurple();
			state = IDLE;
			break;
		case ACKNOWLEDGED:
			flashGreen();
			state = IDLE;
			break;
		default:
			break;
	}

}