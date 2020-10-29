int M1_direction = 4;
int M1_speed = 5;
int M2_direction = 7;
int M2_speed = 6;

boolean stop = false;
int ledPin=10;
int button = 2;
boolean allume=false;
boolean etat=0;
boolean ancien_etat=0;

void toggle_up_button(){
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

void setup() {
  pinMode(M1_direction,OUTPUT);
  pinMode(M1_speed,OUTPUT);
  pinMode(M2_direction,OUTPUT);
  pinMode(M2_speed,OUTPUT);

  pinMode(button, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  toggle_up_button();
  if (allume){
    digitalWrite(M1_direction, HIGH);
    analogWrite(M1_speed, 99); 
    digitalWrite(M2_direction, LOW);
    analogWrite(M2_speed, 105);
  }
  if (!allume){
    analogWrite(M1_speed, 0);
    analogWrite(M2_speed, 0);
    stop = true;
  }
}
