//Initialisation des variables
//Capteurs
uint8_t pin_sensor_3 = 8;
uint8_t pin_sensor_2 = 9;
uint8_t pin_sensor_1 = 12;
unsigned long sensorsValues[4];

//Bouton + LED
int ledPin=10;
int button = 2;
boolean allume=false;
boolean etat=0;
boolean ancien_etat=0;

//Moteurs
int M1_direction = 4;
int M1_speed = 5;
int M2_direction = 7;
int M2_speed = 6;

//Vitesses des moteurs (base) + vitesse max
int vitesse_moteur_M1=130;
int vitesse_moteur_M2=120;
int vitesse_max=255;

//Variables pour la calibration
int calibration_max=0;
int calibration_min=0;
int calibration_max_1, calibration_max_2, calibration_max_3;
int calibration_min_1, calibration_min_2, calibration_min_3;

//Variable pour que le robot s'arrête directement sur du blanc
boolean stop = false;

////////////////////////////////////////////////////////////////////
//Fonctions
////////////////////////////////////////////////////////////////////

//Fonction driver qui permet de calculer la réflectivité de la surface
//Plus la valeur est élevée, moins la surface est réflective (on se rapproche du noir)
//Si la valeur se rapproche de 0, la surface est réflective (on se rapproche du blanc)
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

//Fonction qui permet de faire marcher le bouton (utiliser pour faire démarrer le robot)
//Aussi pour le faire repartir (gain de temps pour ne pas à chaque fois recalibrer le robot à chaque essai)
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
        //On passe stop à false car si le robot s'arrête car les valeurs détectent du blanc
        //On peut toujours le replacer sur une ligne noire pour le faire repartir
        //Sans avoir à recalibrer le robot à nouveau
        stop=false;
      }
    }
    ancien_etat=etat;
   }
   delay(1); 
}

//Fonction qui remet les valeurs de calibration min et max à 0 à chaque passage des capteurs 
void reset_calibration(){
  calibration_max=0;
  calibration_min=0;
}

//Fonction calibration qui prend les valeurs de la fonction driver
//Execution d'un calcul pour avoir une marge d'erreur
void calibration(int sensor){
  int i=0;

  //Dès qu'on appuie sur le bouton, le programme prend la valeur à partir de driver
  //Et l'enregistre dans un tableau
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

  //Calcul de la marge d'erreur
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


////////////////////////////////////////////////////////////////////
//Setup/Loop
////////////////////////////////////////////////////////////////////

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

  //Bouton + LED
  pinMode(button, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(9600);

  //Calibration
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
  //Prendre les valeurs des capteurs
  int sensor_1=driver(pin_sensor_1);
  int sensor_2=driver(pin_sensor_2);
  int sensor_3=driver(pin_sensor_3);
  int i=0;

  //Print des valeurs
  Serial.print(sensor_1);
  Serial.print(" ");
  Serial.print(sensor_2);
  Serial.print(" ");
  Serial.print(sensor_3);
  Serial.println();

  //LED est éteinte
  toggle_up_button();
  //Si on appuie sur le bouton pour allumer la LED, le robot démarre
  //Si le robot était déjà en marche mais qu'il s'est arrêté à cause d'une surface réflective
  //On peut le refaire démarrer en appuyant de nouveau sur le bouton
  
  if (!stop && allume){
    //M1 à HIGH et M2 à LOW pour que les roues tournent dans le même sens
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
      analogWrite(M1_speed, vitesse_moteur_M1+i);
      analogWrite(M2_speed, vitesse_moteur_M2-i);
      
      //Pour faire tourner le robot, on incrémente la vitesse de base sans dépasser la vitesse max
      //On tourne à gauche
      if ((vitesse_moteur_M1+i)<vitesse_max && (vitesse_moteur_M2-i)>0){
        i+=5;
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
      analogWrite(M1_speed, vitesse_moteur_M1-i);
      analogWrite(M2_speed, vitesse_moteur_M2+i);

      //Pour faire tourner le robot, on incrémente la vitesse de base sans dépasser la vitesse max
      //On tourne à droite
      if ((vitesse_moteur_M2+i)<vitesse_max && (vitesse_moteur_M1-i)>0){
        i+=5;
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
