void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(Serial.read(), BIN);
}
