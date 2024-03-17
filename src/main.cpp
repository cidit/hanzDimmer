#include <Arduino.h>
#include <ProtoTGP.h>

const int POTENTIOMETER_PIN = 34;
const int LINE_HEIGHT = 8;
ProtoTGP proto;

/**
 * Petit truc que j'ai apris en lisant le code de TGP Ecran. :)
 * Détermine si la ligne spécifiée vas être visible sur l'écran ou pas.
 * @param screen Référence à l'écran à mesurer.
 * @param line Ligne à vérifier.
 */
bool isLineValid(Adafruit_GFX &screen, int line)
{
  return line >= 0 && line < (screen.height() / LINE_HEIGHT);
}

/**
 * Obtient le ratio d'une valeur donnée.
 * @param value La valeur. Assumée d'être entre [max] et [min].
 * @param max La valeur maximalle.
 * @param min La valeur minimalle.
 */
float getRatio(float value, float max, float min)
{
  return (value - min) / (max - min);
}

/**
 * operation inverse de getRatio. obtient la valeur entre [max] et [min] qui correspond au ratio
 */
float getIntermediate(float ratio, float max, float min)
{
  return ((max - min) * ratio) + min;
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

  // La hauteur des lignes à la plus petite police est de 8 pixels, le dernier
  // étant réservé pour laisser un espace entre les charactères. (Les charactères sont donc
  // techniquement 7px de haut + 1 buffer.) Vu que notre gradateur utilise une ligne de
  // texte complète, c'est également cette hauteur que nous allons utiliser.
  auto height = LINE_HEIGHT - 1;
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
  auto ratio = getRatio(valeur, max, min);
  drawDimmer(proto.ecran, ratio, ligne);
}

/**
 * Le gradateur circulaire est dessiné avec un rayon de taille de police 3.
 */
void drawCircularDimmer(Adafruit_GFX &screen, float value, float line, int xoffset = 0)
{
  auto radius = LINE_HEIGHT * 3;
  // line+1 Parce que on veut que le gradateur soit dessiné au bas de la ligne
  auto y = LINE_HEIGHT * (line + 1);
  auto x = radius + xoffset;

  auto angleRads = getIntermediate(value, PI, 0);
  auto linex = cosf(angleRads) * radius;
  auto liney = sinf(angleRads) * radius;

  screen.startWrite();
  screen.fillCircleHelper(x, y, radius, 0x1, 0, BLACK);
  screen.fillCircleHelper(x, y, radius, 0x2, 0, BLACK);
  screen.drawCircleHelper(x, y, radius, 0x1, WHITE);
  screen.drawCircleHelper(x, y, radius, 0x2, WHITE);
  screen.drawLine(x, y, x - linex, y - liney, WHITE);
  screen.endWrite();
}

void dessinerGradateurCirculaire(float valeur, float min = 0, float max = 1, int ligne = 0, int offset = 0)
{
  auto ratio = getRatio(valeur, max, min);
  drawCircularDimmer(proto.ecran, ratio, ligne, offset);
}

/**
 * Imprime la mesure à tous les endroits pertinants
 * @param analogMesure La mesure à imprimer. Le maximum est de 4095.
 */
void printMesureEverywhere(int analogMesure)
{
  auto maxAnalogValue = 4095;
  Serial.println(analogMesure);
  dessinerGradateur(analogMesure, 0, maxAnalogValue, 0);
  proto.ecran.ecrire(String(analogMesure), 1);
  proto.ecran.ecrire("0         4095", 4);
  dessinerGradateurCirculaire(analogMesure, 0, maxAnalogValue, 4, 8);
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
  // la mesure n'est pas constante. faire une moyenne des 10 dernières valeurs?

  printMesureEverywhere(mesure);
}
