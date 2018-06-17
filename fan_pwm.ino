int relayPin = 5;
int pwmPin = 4;
int tachPin = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(relayPin, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(tachPin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(relayPin, LOW); //Set the pin to HIGH (3.3V)
   
  Serial.println("10 % PWM");
  analogWrite(pwmPin, 102);
  delay(5000);

  Serial.println("Lettura: " + digitalRead(tachPin));
 
  Serial.println("20 % PWM");
  analogWrite(pwmPin, 205);
  delay(5000);

  Serial.println("Lettura: " + digitalRead(tachPin));
 
  Serial.println("40 % PWM");
  analogWrite(pwmPin, 410);
  delay(5000);

  Serial.println("Lettura: " + digitalRead(tachPin));
 
  Serial.println("70 % PWM");
  analogWrite(pwmPin, 714);
  delay(5000);

  Serial.println("Lettura: " + digitalRead(tachPin));
 
  Serial.println("100 % PWM");
  analogWrite(pwmPin, 1024);
  delay(5000);
  
  Serial.println("Lettura: " + digitalRead(tachPin));

  Serial.println("1 % PWM");
  analogWrite(pwmPin, 1);
  delay(7000);

  Serial.println("Lettura: " + digitalRead(tachPin));

  digitalWrite(relayPin, HIGH); //Set the pin to HIGH (3.3V)
  delay(10000);   

  Serial.println("Lettura: " + digitalRead(tachPin));
}
