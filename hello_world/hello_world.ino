
int pin = 2;
unsigned ms = 0;

void setup() {
  pinMode(pin, OUTPUT);
}

void loop() {
  digitalWrite(pin, ms%2);
  delay(ms++ %1000);
}



