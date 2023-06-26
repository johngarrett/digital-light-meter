#define CLK 5
#define DT 6
#define SW 9

int counter = 0;
int currentStateCLK;
int lastStateCLK;
String currentDir ="";
unsigned long lastButtonPress = 0;

void setup() { 
	pinMode(SW, INPUT_PULLUP);
	pinMode(CLK,INPUT);
	pinMode(DT,INPUT);
   
	Serial.begin(9600);
}

void loop() {
	int sw_val = digitalRead(SW);
	//Serial.println(sw_val);

	// Read the current state of CLK
	currentStateCLK = digitalRead(CLK);

	// If last and current state of CLK are different, then pulse occurred
	// React to only 1 state change to avoid double count
	if (currentStateCLK != lastStateCLK  && currentStateCLK == 1){

		// If the DT state is different than the CLK state then
		// the encoder is rotating CCW so decrement
		if (digitalRead(DT) != currentStateCLK) {
			counter --;
			currentDir ="CCW";
		} else {
			// Encoder is rotating CW so increment
			counter ++;
			currentDir ="CW";
		}

		Serial.print("Direction: ");
		Serial.print(currentDir);
		Serial.print(" | Counter: ");
		Serial.println(counter);
	}

	// Remember last CLK state
	lastStateCLK = currentStateCLK;

	// Read the button state
	int btnState = digitalRead(SW);

	//If we detect LOW signal, button is pressed
	if (btnState == LOW) {
		//if 50ms have passed since last LOW pulse, it means that the
		//button has been pressed, released and pressed again
		if (millis() - lastButtonPress > 50) {
			Serial.println("Button pressed!");
		}

		// Remember last button press event
		lastButtonPress = millis();
	}

	// Put in a slight delay to help debounce the reading
	delay(1);
}
