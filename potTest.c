int potPin = 0;
int i = 0;

void setup() {
	Serial.begin(9600);
}

void loop() {
	i=analogRead(potPin);
	Serial.println(i);
	delay(750);
}
