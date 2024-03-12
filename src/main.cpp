#include <Arduino.h>
#include <ProtoTGP.h>

const int POTENTIOMETER_PIN = 34;
ProtoTGP proto;

/**
 * Petit truc que j'ai apris en lisant le code de TGP Ecran. :)
 * Détermine si la ligne spécifiée vas être visible sur l'écran ou pas.
 */
bool isLineValid(Adafruit_GFX &screen, int line)
{
  return line >= 0 && line < (screen.height() / 8);
}

/**
 * @param screen L'écran spécifique sur lequel dessiner. Doit avoir été initialisé au préalable.
 * @param value Est assumée d'être entre 0 et 1.
 * @param line La ligne est validée avec la taille de l'écran. la fonction ne fera pas de travail si celle-ci se trouve au dela des limites de l'écran
 */
void drawDimmer(Adafruit_GFX &screen, const float value, const int line)
{
  // Si le gradateur n'est pas sur une ligne visible, ça ne sert à rien de l'afficher.
  // Dans ce cas, aussi bien retourner tôt.
  if (!isLineValid(screen, line))
  {
    return;
  }

  // La hauteur des charactères à la plus petite police est de 7 pixels + 1 pixel
  // d'espacement entre les lignes. Vu que notre gradateur utilise une ligne
  // complète, c'est également cette hauteur que nous allons utiliser.
  auto height = 7;
  auto width = screen.width();

  // La hauteur du coin supérieur gauche de notre gradateur.
  auto starty = line * 8;

  // On commence par remplir notre espace de travail par un rectangle blanc pour effacer
  // ce qui peut être dessous, suivit d'un rectangle noir légèrement plus petit en son centre
  // pour créer une bordure blanche.
  screen.fillRect(0, starty, width, height, WHITE);
  screen.fillRect(1, starty + 1, width - 2, height - 2, BLACK);

  // Finalement, on dessine le rectangle qui représentera la position de notre gradateur.
  // Pour ce faire, il nous faut la composante x de la coordonnée de fin du rectangle, qu'on
  // trouve via produit croisé avec la longeur de l'écran.
  auto stopx = width * value;

  // Ensuite, on dessine le rectangle, en blanc!
  screen.fillRect(0, starty, stopx, height, WHITE);
}

/**
 * REMPLIS LES DEMANDES DU DEVIS!
 * @param valeur Valeur que le gradateur devra représenter. Elle est assumée d'être entre [min] et [max].
 * @param ligne Ligne de l'écran sur laquelle le gradateur sera dessiné. le gradateur prendra la largeur complète de la ligne et la hauteur du texte à sa plus petite police.
 */
void dessinerGradateur(float valeur, float min = 0, float max = 1, int ligne = 0)
{
  // Déterminer la valeur entre 0 et 1.
  auto value = (valeur - min) / (max - min);

  drawDimmer(proto.ecran, value, ligne);
}

/**
 * Imprime la mesure à tous les endroits pertinants
*/
void printMesureEverywhere(int mesure) {
  Serial.println(mesure);
  dessinerGradateur(mesure, 0, 4095, 0);
  proto.ecran.ecrire(String(mesure), 1);
}

void setup()
{
  Serial.begin(115200);
  proto.begin();
  pinMode(POTENTIOMETER_PIN, INPUT);
}

void loop()
{
  proto.refresh();
  proto.ecran.clearDisplay();

  int mesure = analogRead(POTENTIOMETER_PIN);
  // la mesure n'est pas constante. faire une regression linéaire des 10 dernières valeurs?

  printMesureEverywhere(mesure);
}

