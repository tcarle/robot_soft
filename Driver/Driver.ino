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

int vitesse_moteur_M1=95;
int vitesse_moteur_M2=85;
int vitesse_max=255;

int calibration_max=0;
int calibration_min=0;
int calibration_max_1, calibration_max_2, calibration_max_3;
int calibration_min_1, calibration_min_2, calibration_min_3;


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
  //Serial.print(diff);
  //Serial.println();

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
        stop=false;
      }
    }
    ancien_etat=etat;
   }
   delay(1); 
}

void reset_calibration(){
  calibration_max=0;
  calibration_min=0;
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
          sensorsValues[i]=driver(sensor);
          i++;
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

  calibration_max=maxi-((maxi*30)/100);
  calibration_min=mini+((maxi*30)/100);
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
  /*calibration_max_1=797;
  calibration_min_1=407;
  calibration_max_2=503;
  calibration_min_2=281;
  calibration_max_3=682;
  calibration_min_3=302;*/
  calibration(pin_sensor_1);
  calibration_max_1=calibration_max;
  calibration_min_1=calibration_min;
  reset_calibration();
  calibration(pin_sensor_2);
  calibration_max_2=calibration_max;
  calibration_min_2=calibration_min;
  reset_calibration();
  calibration(pin_sensor_3);
  calibration_max_3=calibration_max;
  calibration_min_3=calibration_min;
  allume=false;
  digitalWrite(ledPin, LOW);

}

void loop() {
  int sensor_1=driver(pin_sensor_1);
  int sensor_2=driver(pin_sensor_2);
  int sensor_3=driver(pin_sensor_3);
  int i=0;

  Serial.print(sensor_1);
  Serial.print(" ");
  Serial.print(sensor_2);
  Serial.print(" ");
  Serial.print(sensor_3);
  Serial.println();

  toggle_up_button();
  
  if (!stop && allume){

    digitalWrite(M1_direction, HIGH);
    analogWrite(M1_speed, vitesse_moteur_M1); 
    digitalWrite(M2_direction, LOW);
    analogWrite(M2_speed, vitesse_moteur_M2);

    //Tous les capteurs sur du noir
     while ((sensor_1>calibration_min_1 && sensor_2>calibration_min_2 && sensor_3>calibration_min_3)){
      analogWrite(M1_speed, vitesse_moteur_M1);
      analogWrite(M2_speed, vitesse_moteur_M2);
      sensor_1=driver(pin_sensor_1);
      sensor_2=driver(pin_sensor_2);
      sensor_3=driver(pin_sensor_3);
    }

    //Capteur 1 et 2 sur noir, 3 sur blanc
    //Capteur 1 sur noir, 2 et 3 sur blanc
    while ((sensor_1>calibration_min_1 && sensor_2>calibration_min_2 && sensor_3<calibration_min_3) || 
      (sensor_1>calibration_min_1 && sensor_2<calibration_min_2 && sensor_3<calibration_min_3)){
      Serial.print("droite");
      Serial.println();
      Serial.print(vitesse_moteur_M1+i);
      Serial.print(" ");
      Serial.print(vitesse_moteur_M2);
      Serial.println();
      analogWrite(M1_speed, vitesse_moteur_M1+i);
      analogWrite(M2_speed, vitesse_moteur_M2);
      if ((vitesse_moteur_M1+i)<vitesse_max){
        i+=2;
      }
      sensor_1=driver(pin_sensor_1);
      sensor_2=driver(pin_sensor_2);
      sensor_3=driver(pin_sensor_3);
    }
    i=0;

    //Capteur 2 et 3 sur noir, 1 sur blanc
    //Capteur 3 sur noir, 1 et 2 sur blanc
    while ((sensor_1<calibration_min_1 && sensor_2>calibration_min_2 && sensor_3>calibration_min_3) || 
      (sensor_1<calibration_min_1 && sensor_2<calibration_min_2 && sensor_3>calibration_min_3)){
      Serial.print("gauche");
      Serial.println();
      Serial.print(vitesse_moteur_M1);
      Serial.print(" ");
      Serial.print(vitesse_moteur_M2+i);
      Serial.println();
      analogWrite(M1_speed, vitesse_moteur_M1);
      analogWrite(M2_speed, vitesse_moteur_M2+i);
      if ((vitesse_moteur_M2+i)<vitesse_max){
        i+=2;
      }
      sensor_1=driver(pin_sensor_1);
      sensor_2=driver(pin_sensor_2);
      sensor_3=driver(pin_sensor_3);
    }
    i=0;
    
    //Tous les capteurs sur du blanc
    if (sensor_1<calibration_min_1 && sensor_2<calibration_min_2 && sensor_3<calibration_min_3){
      analogWrite(M1_speed, 0);
      analogWrite(M2_speed, 0);
      stop = true;
    }
  }
  
}
