int green_pin = 6;
int red_pin = 9;
int blue_pin = 5;

void setup() {
  // put your setup code here, to run once:
  pinMode(green_pin, OUTPUT);
  pinMode(red_pin, OUTPUT);
  pinMode(blue_pin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(green_pin, LOW);
  digitalWrite(red_pin, HIGH);
  digitalWrite(blue_pin, HIGH);

  delay(1000);

  digitalWrite(green_pin, HIGH);
  digitalWrite(red_pin, LOW);
  digitalWrite(blue_pin, HIGH);

  delay(1000);

  digitalWrite(green_pin, HIGH);
  digitalWrite(red_pin, HIGH);
  digitalWrite(blue_pin, LOW);

  delay(1000);
}
