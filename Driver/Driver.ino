uint8_t pin_sensor_3 = 8;
uint8_t pin_sensor_2 = 9;
uint8_t pin_sensor_1 = 12;
unsigned long sensorsValues[4];

int ledPin=10;
int button = 2;
boolean allume=false;
boolean etat=0;
boolean ancien_etat=0;

int M1_direction = 4;
int M1_speed = 5;
int M2_direction = 7;
int M2_speed = 6;

int calibration_max;
int calibration_min;

boolean stop = false;

////////////////////////////////////////////////Fonctions
unsigned long driver(int sensor){
  pinMode(sensor, OUTPUT);
  digitalWrite(sensor, HIGH);
  delayMicroseconds(10);
  pinMode(sensor, INPUT);
  digitalWrite(sensor, LOW);

  int x=digitalRead(sensor);
  unsigned long t=micros();
  unsigned long t_2=t;
  unsigned long diff;
  boolean fin=false;

  while (!fin && (t_2-t)<1000){
    x=digitalRead(sensor);
    t_2=micros();
    fin=(x==0);
  }
  
  diff=t_2-t;
  Serial.print(diff);
  Serial.println();

  return diff;
  
}

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

void calibration(int sensor){
  int i=0;
  while(i<4){
    etat=digitalRead(button);
    
    if (etat!=ancien_etat){
      if (etat==HIGH){
        if (!allume){
          digitalWrite(ledPin, HIGH);
          allume=true;
          sensorsValues[i]=driver(sensor);
          i++;
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

  //Calcul
  int mini=sensorsValues[0];
  int maxi=sensorsValues[0];
  for(int i=0;i<4;i++){
    if (mini>sensorsValues[i]){
      mini=sensorsValues[i];
    }
    if (maxi<sensorsValues[i]){
      maxi=sensorsValues[i];
    }
  }

  calibration_max=maxi-((maxi*20)/100);
  calibration_min=mini+((maxi*20)/100);
}

///////////////////////////////////////////Setup/Loop
void setup() {
  //Moteurs
  pinMode(M1_direction,OUTPUT);
  pinMode(M1_speed,OUTPUT);
  pinMode(M2_direction,OUTPUT);
  pinMode(M2_speed,OUTPUT);

  //Capteurs
  pinMode(pin_sensor_1, OUTPUT);
  pinMode(pin_sensor_2, OUTPUT);
  pinMode(pin_sensor_3, OUTPUT);

  pinMode(button, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(9600);

  //Calibration
  calibration(pin_sensor_2);
  allume=false;
  digitalWrite(ledPin, LOW);
}

void loop() {
  //int sensor_1=driver(pin_sensor_1);
  int sensor_2=driver(pin_sensor_2);
  //int sensor_3=driver(pin_sensor_3);

  toggle_up_button();
  
  //Moteur 1
  if (!stop && allume){
    digitalWrite(M1_direction, HIGH);
    analogWrite(M1_speed, 150); 
    //Moteur 2
    digitalWrite(M2_direction, LOW);
    analogWrite(M2_speed, 150);
    
    if (sensor_2<calibration_min){
      analogWrite(M1_speed, 0);
      analogWrite(M2_speed, 0);
      stop = true;
    } 
  }
  
}
