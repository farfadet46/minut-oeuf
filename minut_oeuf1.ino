#include <Adafruit_NeoPixel.h>
#define DELAYVAL 500
#define PIN 9
#define buzzerPIN 6
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 4
// When setting up the NeoPixel library, we tell it how many pixels,
// and which pin to use to send signals. Note that for older NeoPixel
// strips you might need to change the third parameter -- see the
// strandtest example for more information on possible values.
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int valor = 0;
String gauche = "GAUCHE";
String droite = "DROITE";
String ok = "VALIDER";
String no = "ANNULER";
String aucun = "Aucun";

int mode = 0;
// mode 0 = coque     = 3 minutes   = 180 sec
// mode 1 = mollet    = 6 minutes   = 360 sec
//mode 2 = cuit dur   = 10 minutes  = 600 sex
// mode 4 = 4secondes

int temps_cuisson[] = {180,360,600,4};
int nombre_modes = sizeof(temps_cuisson) / sizeof(temps_cuisson[0]);
bool decompte = false;
float minuterie = 0.0f;
unsigned long debutDecompte = 0; 
unsigned long dernierChangementBouton = 0;

void setup()
{
  pinMode(A4, INPUT);
  pinMode(buzzerPIN, OUTPUT);
  Serial.begin(9600);
  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif
  pixels.begin();
}

void loop()
{
  int tempread = analogRead(A4);
  Serial.println(tempread);
  if(tempread != valor && millis() - dernierChangementBouton >100 ) {
    Serial.println(action(tempread));
    valor = tempread;
    dernierChangementBouton = millis();
    if (decompte) {
      // Si le décompte est en cours, vérifiez également le bouton "ANNULER"
      if (action(tempread) == no) {
        modeAnnuler();
        Serial.println("Annuler");
        Serial.println(tempread);
      }
    } else {
      // Si le décompte n'est pas en cours, gérez les autres boutons
      if (action(tempread) == ok) {
        modeSelectionne();
        Serial.println("Valider");
        Serial.println(tempread);
      } else if (action(tempread) == gauche) {
        modePrecedent();
        Serial.println("Precedent");
        Serial.println(tempread);
      } else if (action(tempread) == droite) {
        modeSuivant();
        Serial.println("Suivant");
        Serial.println(tempread);
      }
      // Affichage statique du mode sélectionné
      pixels.clear();
      pixels.setPixelColor(mode, pixels.Color(55, 0, 0));
      pixels.show();
    }
  }

  if (decompte)
  {
    gestionDecompte();
  }
}

void gestionDecompte()
{
 float temps_restant = tempsRestant();
  if (temps_restant <= 0)
  {
    finMinuteur(); // Si le temps restant est inférieur ou égal à 0, indiquez la fin du minuteur
  }
  else
  {
    Serial.println(temps_restant);
    miseAJourLEDs();
  }
}

void miseAJourLEDs()
{
  pixels.clear();
  // Calculez le pourcentage de temps écoulé
  float pourcentage_ecoule = (minuterie - tempsRestant()) / minuterie * 100.0;

  // Calculez le nombre de LEDs à allumer en fonction du pourcentage écoulé
  int leds_allumees = (NUMPIXELS * pourcentage_ecoule) / 100;

  // Allumez les LEDs correspondantes
  for (int i = 0; i < leds_allumees; i++) {
    pixels.setPixelColor(i, pixels.Color(55, 0, 0)); // Couleur rouge pour les LEDs allumées
  }
  pixels.show();
}

float tempsRestant() {
  unsigned long temps_ecoule = millis() - debutDecompte; // Temps écoulé depuis le début du décompte
  float temps_restant = minuterie - (temps_ecoule / 1000.0); // Temps restant en secondes
  if (temps_restant < 0) {
    temps_restant = 0; // Assurez-vous que le temps restant ne soit pas négatif
  }
  return temps_restant;
}

void modeSelectionne() {
  // Sélection du mode de cuisson en fonction de la valeur de la variable mode
  if (mode >= 0 && mode < nombre_modes) {
    minuterie = temps_cuisson[mode];
    debutDecompte = millis();
    pixels.clear();
    pixels.show();
    decompte = true; // Commencer le décompte
  }
}

void modePrecedent() {
  mode--;
  if (mode < 0) {
    mode = nombre_modes - 1; // Si mode est inférieur à 0, revenir au dernier mode
  }
}

void modeSuivant() {
  mode++;
  if (mode >= nombre_modes) {
    mode = 0; // Si mode dépasse le nombre de modes, revenir au premier mode
  }
}

void modeAnnuler(){
  minuterie = 0.0f;
  decompte = false;
}

String action(int v) {
  if(v == 681 ) {
    return gauche;
  } else if(v == 510) {
    return droite;
  } else if(v == 767) {
    return ok;
  } else if(v == 817) {
    return no;
  }
  return aucun;
}

void finMinuteur() {
  Serial.println("c'est cuit !");
  decompte = false; // Arrêtez le décompte
  for (int i = 0; i < 5; i++) {
    fadeOut(500); // Fondu des LEDs sur une demi-seconde
    delay(500); // Attendez 500 millisecondes
  }
  // Jouez un son avec le buzzer
  tone(buzzerPIN, 1000, 1000); // Jouez un son de 1000 Hz pendant 1 seconde
}

void fadeOut(int delay_time) {
  int fade_step = 100; // Pas de diminution pour chaque étape de fondu
  for (int brightness = 55; brightness >= 0; brightness -= fade_step) {
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(brightness, 0, 0)); // Réglez la luminosité de toutes les LEDs
    }
    pixels.show();
    delay(delay_time);
  }
  pixels.clear(); // Éteignez toutes les LEDs une fois le fondu terminé
  pixels.show();
}

/*
void affichageStatique() {
  pixels.clear();
  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 150, 0));
    pixels.show();
    delay(DELAYVAL);
  }
}
*/
