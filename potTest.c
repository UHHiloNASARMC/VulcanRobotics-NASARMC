int potPin = 0;
int i = 0;

void setup() {
	Serial.begin(115200);
}

void loop() {
	i=analogRead(potPin);
	Serial.println(i);
	delay(750);
}
