int ledPin=7;
int button=2;

boolean allume=false;
boolean etat=0;
boolean ancien_etat=0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(button, INPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(9600);
}

void loop() {
  etat=digitalRead(button);
  
  if (etat!=ancien_etat){
    if (etat==HIGH){
      if (!allume){
        digitalWrite(ledPin, HIGH);
        allume=true;
      }
      else{
        digitalWrite(ledPin, LOW);
        allume=false;
      }
    }
    ancien_etat=etat;
  }
  delay(1); 
}
