#define BUTTON_PIN 21  

int count = 0;

void setup() 
{
  Serial.begin(9600);         
  pinMode(BUTTON_PIN, INPUT_PULLUP); 
  Serial.println("Proba...");
}

void loop() {
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW) 
  { 
    count++;
    Serial.print(count);
    Serial.println(" : Apasat");
  } 

  delay(500); 
}
