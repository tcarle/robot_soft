//Initialisation des variables
//Capteurs
uint8_t pin_sensor_3 = 8;
uint8_t pin_sensor_2 = 9;
uint8_t pin_sensor_1 = 12;

unsigned long sensorsValues1[4], sensorsValues2[4], sensorsValues3[4];
unsigned long sensorsValuesPID_G[10], sensorsValuesPID_D[10];

float moyenne1=0, moyenne2=0, moyenne3=0;

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
int vitesse_moteur_M1=110;
int vitesse_moteur_M2=110;
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
int indice=0;
int indice_loop=0;

//Variables PID
float k_p=0.08;
float k_d=0.01;
float k_i=0;//0.005;
float k_sr=0.08;
float erreur_tableau[50];

//Variable temps
unsigned long start=0;
unsigned long stopp=0;


int out;
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
          for(int j=0; j<10;j++){
            sensorsValuesPID_G[j]=driver(sensor_g);
            sensorsValuesPID_D[j]=driver(sensor_d);
          }
          
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

  for(int i=0; i<10;i++){
    G_0+=sensorsValuesPID_G[i];
    D_0+=sensorsValuesPID_D[i];
  }
  G_0/=10;
  D_0/=10;
}

//Fonction pour calculer le PID
int calcul_pid(int erreur, int derniere_erreur, int erreur_totale){
  int p =(int)(k_p*erreur + k_d*(erreur-derniere_erreur) + k_i*erreur_totale);
  return p;
}

//Fonction calibration qui prend les valeurs de la fonction driver
//Execution d'un calcul pour avoir une marge d'erreur
void calibration_blanc(int sensor1, int sensor2, int sensor3){
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
          sensorsValues1[i]=driver(sensor1);
          sensorsValues2[i]=driver(sensor2);
          sensorsValues3[i]=driver(sensor3);
          i++;
        }
        else{
          digitalWrite(ledPin, LOW);
          allume=false;
          sensorsValues1[i]=driver(sensor1);
          sensorsValues2[i]=driver(sensor2);
          sensorsValues3[i]=driver(sensor3);
          i++;
        }
      }
      ancien_etat=etat;
     }
     delay(1);
  }

  //Calcul moyenne
  for(int i=0;i<4;i++){
    moyenne1+=sensorsValues1[i];
    moyenne2+=sensorsValues2[i];
    moyenne3+=sensorsValues3[i];
  }
  moyenne1/=4;
  moyenne2/=4;
  moyenne3/=4;
  moyenne1=moyenne1*1.1;
  moyenne2=moyenne2*1.1;
  moyenne3=moyenne3*1.1;
}

//Fonction initialisation tableau
void init_tableau(){
  for(int i=0; i<50; i++){
    erreur_tableau[i]=0;
  }
}

//Fonction qui fait la somme des 10 dernières erreurs
int calcul_variation_vitesse(){
  int somme=0;
  for(int i=0; i<50; i++){
    somme+=erreur_tableau[i];
  }
  return somme/5;
}


//Fonction qui fait la somme des 10 dernières erreurs
int maximum(int erreur_g, int erreur_d){
  int maxi=erreur_g;
  if (erreur_d>erreur_g){
    maxi=erreur_d;
  }
  return maxi;
}

void arret_robot(){
  analogWrite(M1_speed, 0);
  analogWrite(M2_speed, 0);
  stop = true;
}

int calcul_sr(int erreur){
  if (erreur<10){
    digitalWrite(ledPin, HIGH);
    return 0;
  }
  int somme=0;
  for(int i=0; i<50; i++){
    somme+=erreur_tableau[i];
  }
  somme=abs(somme)/50;
  int res=k_sr*somme;
  //digitalWrite(ledPin, LOW);
  return res;
}

void remise_tableau_zero(int derniere_erreur, int erreur, int indice){
  if ((derniere_erreur>0 && erreur<0) || (derniere_erreur<0 && erreur>0)){
    for(int i=0; i<50; i++){
      if (indice!=i){
        erreur_tableau[i]=0;
      }
    }
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

  digitalWrite(M1_direction, HIGH);
  digitalWrite(M2_direction, LOW);
  //Calibration blanc
  calibration_blanc(pin_sensor_1, pin_sensor_2, pin_sensor_3);

  /*Serial.print(moyenne1);
  Serial.print("  ");
  Serial.print(moyenne2);
  Serial.println();*/

  //Calibration PID
  calibration(pin_sensor_1, pin_sensor_3);
  
  allume=false;
  out = 0;
  digitalWrite(ledPin, LOW);
}

void loop() {
  start=millis();
  
  //Prendre les valeurs des capteurs
  int sensor_g=driver(pin_sensor_1);
 // Serial.println("G0");
 // Serial.println(G_0);
 // Serial.println(sensor_g);
  
  int sensor_m=driver(pin_sensor_2);
  int sensor_d=driver(pin_sensor_3);
 // Serial.println("D0=");
 // Serial.println(D_0);
 // Serial.println(sensor_d);
  
  int i=0;

  derniere_erreur=erreur;
  erreur_g=sensor_g-G_0;
  erreur_d=sensor_d-D_0;
  erreur=erreur_g-erreur_d;
  erreur_totale+=erreur;
  
  float pid=calcul_pid(erreur, derniere_erreur, calcul_variation_vitesse());// erreur_totale);
  erreur_tableau[indice]=erreur;//max(erreur_g,erreur_d);

  int sr=calcul_sr(erreur);
  //Serial.println(erreur);

  remise_tableau_zero(derniere_erreur, erreur_tableau[indice], indice);

  //LED est éteinte
  toggle_up_button();
  //Si on appuie sur le bouton pour allumer la LED, le robot démarre
  //Si le robot était déjà en marche mais qu'il s'est arrêté à cause d'une surface réflective
  //On peut le refaire démarrer en appuyant de nouveau sur le bouton
  
  if (!stop && allume){
    //M1 à HIGH et M2 à LOW pour que les roues tournent dans le même sens
    //digitalWrite(M1_direction, HIGH);
    //analogWrite(M1_speed, vitesse_moteur_M1); 
    //digitalWrite(M2_direction, LOW);
    //analogWrite(M2_speed, vitesse_moteur_M2);
    int vit_M1 = min(254, vitesse_moteur_M1+pid-sr);
    vit_M1 = max(0,vit_M1);
    int vit_M2 = min(254, vitesse_moteur_M2-pid-sr);
    vit_M2 = max(0, vit_M2);
    analogWrite(M1_speed, vit_M1);
    analogWrite(M2_speed, vit_M2);
    
    if(indice==0){
      if (sensor_g<moyenne1 && sensor_d<moyenne3 && out){
        arret_robot();
      }
      else{
        if (sensor_g<moyenne1 && sensor_d<moyenne3){
          out =1;
          }
        else{
          out=0;
         }
      }
    }
    
  }

  indice++, indice_loop++;

  if (indice==50){
    indice=0;
  }

  stopp=millis();
  unsigned long reste=10-(stopp-start);

  if (reste<0){
    digitalWrite(ledPin, LOW);
    arret_robot();
  }

  delay(reste);
}
