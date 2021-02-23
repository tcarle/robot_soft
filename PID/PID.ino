//Initialisation des variables
//Capteurs
uint8_t pin_sensor_3 = 8;
uint8_t pin_sensor_2 = 9;
uint8_t pin_sensor_1 = 12;

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
unsigned long G_0;
unsigned long D_0;

//Variable pour que le robot s'arrête directement sur du blanc
boolean stop = false;

//Variables loop
int erreur_g=0;
int erreur_d=0;
int erreur=0;
int derniere_erreur=0;
int erreur_totale=0;

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

void calibration(int sensor_g, int sensor_d){
  int i=0;

  //Dès qu'on appuie sur le bouton, le programme prend la valeur à partir de driver
  //Et l'enregistre dans un tableau
  while(i<2){
    etat=digitalRead(button);
    
    if (etat!=ancien_etat){
      if (etat==HIGH){
        if (!allume){
          digitalWrite(ledPin, HIGH);
          allume=true;
          G_0=driver(sensor_g);
          D_0=driver(sensor_d);
          i++;
        }
        else{
          digitalWrite(ledPin, LOW);
          allume=false;
          i++;
        }
      }
      ancien_etat=etat;
     }
     delay(1);
  }
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
  calibration(pin_sensor_1, pin_sensor_3);

  Serial.print(G_0);
  Serial.print(" ");
  Serial.print(D_0);
  Serial.println();
  
  allume=false;
  digitalWrite(ledPin, LOW);
}

void loop() {
  //Prendre les valeurs des capteurs
  int sensor_g=driver(pin_sensor_1);
  int sensor_d=driver(pin_sensor_3);
  int i=0;

  derniere_erreur=erreur;
  erreur_g=G_0-sensor_g;
  erreur_d=D_0-sensor_d;
  erreur=erreur_g-erreur_d;
  erreur_totale+=erreur;

  //Print des valeurs
  Serial.print(erreur_g);
  Serial.print(" ");
  Serial.print(erreur_d);
  Serial.println();
  Serial.println(erreur);
  Serial.println(erreur_totale);

  
}
