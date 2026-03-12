                                                                                                                 // ============================================================
// ⚠️ CONFIGURATION TFT COMPLÈTE - AVANT TOUS LES INCLUDES !
// ============================================================
#define USER_SETUP_LOADED
#define ILI9488_DRIVER

// Broches TFT selon votre branchement
#define TFT_MISO 19
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 17
#define TFT_RST -1  // Connecté à EN (broche reset ESP32)
#define TOUCH_CS 16

// Configuration SPI optimisée
#define SPI_FREQUENCY 40000000
#define SPI_READ_FREQUENCY 16000000
#define SPI_TOUCH_FREQUENCY 2500000

// Rétroéclairage - BROCHE CRITIQUE
#define TFT_BL 14
#define TFT_BL_ON HIGH

// Polices
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// ============================================================
// INCLUDES
// ============================================================
#include <TB_TFT_eSPI.h>
#include <MAX30105.h>
#include <heartRate.h>
#include <HX711_ADC.h>
#include <Adafruit_MLX90614.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>

// ============================================================
// CONFIGURATION MATÉRIELLE - CORRIGÉE ✅
// ============================================================
#define HX711_DT 26
#define HX711_SCK 27

// ✅ COMMUNICATION UART AVEC ESP32 ESCLAVE RFID
#define UART_RFID_RX 16   // Reception depuis ESP32 esclave
#define UART_RFID_TX 17   // Transmission vers ESP32 esclave

#define BUZZER_PIN 32

// ============================================================
// CONFIGURATION WiFi & SERVEUR - MODIFIÉE ✅
// ============================================================
char WIFI_SSID[32] = "MERA";
char WIFI_PASS[64] = "695DD4FB";

// === URLs DES APIs - UTILISATION DE L'API ARDUINO DÉDIÉE ===
const String API_ARDUINO = "http://192.168.1.104:8000/backend/api_arduino.php";

// URL directe vers add_mesure.php utilisée par le POST x-www-form-urlencoded
const String ADD_MESURE_URL = "http://192.168.1.104:8000/backend/add_mesure.php";

// ⭐⭐ PC LOCAL - IP À MODIFIER
const char* PC_AUDIO_IP = "192.168.1.104";  // ⚠️ À CHANGER : IP de votre PC sur le réseau
const int PC_AUDIO_PORT = 5000;

// RASPBERRY PI (pour capture oculaire)
const char* RASPBERRY_IP = "192.168.1.104";  // À CONFIGURER: IP de la Raspberry sur votre réseau
const int RASPBERRY_PORT = 5000;

bool WIFI_ENABLED = true;

// ============================================================
// CODES AUDIO - COMMUNICATION ESP32 ↔ PC FEDORA
// ============================================================
enum AudioCode {
  AUDIO_BADGE = 1,           // 001.mp3 - "Bienvenue. Scanner votre carte..."
  AUDIO_YEUX = 2,            // 002.mp3 - "Analyse oculaire..."
  AUDIO_RYTHME = 3,          // 003.mp3 - "Posez votre doigt sur capteur..."
  AUDIO_TEMPERATURE = 4,     // 004.mp3 - "Capteur de température..."
  AUDIO_POIDS = 5,           // 005.mp3 - "Montez sur la balance..."
  AUDIO_FIN = 6              // 006.mp3 - "Merci. Bilan terminé..."
};

// ============================================================
// NOTES MUSICALES (Hz)
// ============================================================
#define NOTE_C4 262
#define NOTE_D4 294
#define NOTE_E4 330
#define NOTE_F4 349
#define NOTE_G4 392
#define NOTE_A4 440
#define NOTE_B4 494
#define NOTE_C5 523
#define NOTE_D5 587
#define NOTE_E5 659
#define NOTE_F5 698
#define NOTE_G5 784
#define NOTE_REST 0

// ============================================================
// CONSTANTES SYSTÈME - OPTIMISÉES POUR STABILITÉ ✅
// ============================================================
const unsigned long TIMEOUT_ETAT = 60000;
const unsigned long DELAI_AFFICHAGE = 15000;
const unsigned long DELAI_ANTI_REBOND_RFID = 2000;
const unsigned long WIFI_TIMEOUT = 10000;
const unsigned long SERVER_TIMEOUT = 5000;
const int MAX_RETRY_SEND = 3;
const int MAX_QUEUE_SIZE = 20;

// ✅ PARAMÈTRES OPTIMISÉS POUR STABILITÉ
const unsigned long DELAI_MESURE_POIDS = 5000;
const unsigned long DELAI_AFFICHAGE_RESULTATS = 8000;
const float SEUIL_POIDS_DETECTION = 5.0;  // 2 kg au lieu de 2000 g
const float CONVERSION_KG = 1000.0;
const int SEUIL_BPM_MIN = 40;
const int SEUIL_BPM_MAX = 200;
const int SEUIL_IR_DOIGT = 15000;

// Paramètres améliorés pour la stabilité BPM
const int NOMBRE_MESURES_STABLES_REQUIS = 2;
const int TOLERANCE_BPM = 7;
const unsigned long TEMPS_MIN_MESURE_BPM = 3500;  // Équilibre: 3.5s (rapide mais stable)

// Paramètres de stabilisation du poids
const float VARIATION_MAX_STABILITE = 0.1;
const unsigned long TEMPS_STABILITE_REQUIS = 500;  // Réduit de 800ms à 500ms
const int NOMBRE_LECTURES_STABILITE = 4;  // Réduit de 5 à 4

// Paramètres température
const float TEMPERATURE_NORMALE_MIN = 36.1;
const float TEMPERATURE_NORMALE_MAX = 37.2;
const float TEMPERATURE_FIEVRE_LEGERE = 37.5;
const float TEMPERATURE_FIEVRE = 38.0;

const float SPO2_ALPHA = 0.95;

// 🌡️ ALGORITHME PULSEOX PROFESSIONNEL - CONSTANTES
const int SPO2_BUFFER_SIZE = 7;  // Équilibre: 7 (response time ~700-1000ms, bonne stabilité)
const float SPO2_COEFF_A = 104.0;  // Coefficient A (datasheet MAX30102)
const float SPO2_COEFF_B = -17.0;  // Coefficient B (datasheet MAX30102)
const float SPO2_DC_ALPHA = 0.8;   // Filtre DC (composante continue)
const float SPO2_AC_ALPHA = 0.3;   // Filtre AC (composante variable)

// 🫀 ALGORITHME BEAT FINDER PRO - CONSTANTES
const int BPM_BUFFER_SIZE = 6;     // Historique pour moyenne pondérée
const float KALMAN_PROCESS_NOISE = 1.0;  // Q (processus)
const float KALMAN_MEASUREMENT_NOISE = 2.0;  // R (mesure)
const int BEAT_PATTERN_MIN = 200;   // Temps min entre 2 beats (ms)
const int BEAT_PATTERN_MAX = 1800;  // Temps max entre 2 beats (ms)
const float TOLERANCE_ADAPTATIVE_FACTOR = 0.15;  // 15% de variabilité

// 🏋️ CONSTANTES SMART WEIGHT OPTIMIZER
const float KALMAN_WEIGHT_PROCESS = 0.5;          // Léger mouvement toléré
const float KALMAN_WEIGHT_MEASUREMENT = 1.5;      // HX711 bon capteur
const float MOVEMENT_DETECTION_THRESHOLD = 0.5;   // 0.5kg = mouvement
const int WEIGHT_VARIANCE_BUFFER_SIZE = 5;        // Buffer variance (doit être int)
const float WEIGHT_OUTLIER_SIGMA = 3.0;           // Rejet > 3σ
const int WEIGHT_FAST_DETECTION_TIME = 1000;      // Mode rapide: 1s

const int MAX_ETUD = 50;
const int EEPROM_SIZE = 4096;
const int EEPROM_MAGIC = 0xCAFE;

// ✅ NOUVELLE RÉSOLUTION 480x320
const int SCREEN_W = 480;
const int SCREEN_H = 320;

// ============================================================
// COULEURS
// ============================================================
#define BACKGROUND_COLOR 0x18E0
#define PRIMARY_COLOR 0x07FF
#define SECONDARY_COLOR 0xFD20
#define SUCCESS_COLOR 0x3666
#define WARNING_COLOR 0xFD20
#define ERROR_COLOR 0xF9A6
#define TEXT_COLOR 0xFFFF
#define HIGHLIGHT_COLOR 0xFD20
#define TEMP_NORMAL_COLOR 0x3666
#define TEMP_WARNING_COLOR 0xFD20
#define TEMP_HIGH_COLOR 0xF9A6

// ============================================================
// OBJETS GLOBAUX
// ============================================================
TFT_eSPI tft = TFT_eSPI();
MAX30105 particleSensor;
HX711_ADC scale(HX711_DT, HX711_SCK);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// ✅ COMMUNICATION UART AVEC ESP32 ESCLAVE RFID
HardwareSerial RFIDSerial(2); // UART2

// ============================================================
// STRUCTURES DE DONNÉES AMÉLIORÉES ✅
// ============================================================
struct Etudiant {
  int id;
  char uid[20];
  char nom[30];
  char prenom[30];
  char classe[15];
  bool actif;
};

struct DonneesBiometriques {
  int bpm = 0;
  int bpmStable = 0;
  int spo2 = 0;
  int spo2Stable = 0;
  float poids = 0;
  float temperature = 0;
  float temperatureStable = 0;
  float temperatureAmbiante = 0;  // 🌡️ Température ambiante pour logs
  bool donneesValides = false;
  unsigned long timestamp;
};

struct EtatCapteurs {
  bool doigtPresent = false;
  bool poidsStable = false;
  bool temperatureStable = false;
  bool hx711Actif = false;
  bool max30102Actif = false;
  bool mlx90614Actif = false;
  bool nouvelleLecturePoids = false;
  byte ledBright;
  long lastIR;
};

struct Configuration {
  float calFactor = 23.23f;
  float seuilPoidsDetection = 5.0;
  int seuilBpmMin = 40;
  int seuilBpmMax = 200;
  int seuilIrDoigt = 15000;
  unsigned long delaiMesurePoids = 5000;
  unsigned long delaiAffichageResultats = 8000;
};

struct WiFiStatus {
  bool connected = false;
  bool enabled = false;
  int signalStrength = 0;
  unsigned long lastCheck = 0;
};

struct MesureEnAttente {
  char uid[20];
  char nom[30];
  char prenom[30];
  char classe[15];
  int bpm, spo2;
  float temp, poids;
  unsigned long timestamp;
  int tentatives;
  bool actif;
};

// ============================================================
// RÉSULTATS OCULAIRES - STRUCTURE ✅
// ============================================================
struct ResultatsOculaires {
  char oeil_gauche[30] = "";        // Ex: "Sain", "Cataracte", etc.
  float oeil_gauche_conf = 0.0;    // Confiance 0-1
  char oeil_droit[30] = "";         // Ex: "Sain", "Cataracte", etc.
  float oeil_droit_conf = 0.0;     // Confiance 0-1
  int alerte = 0;                   // 1 si anomalie détectée
  bool disponibles = false;         // Résultats reçus du serveur
  unsigned long timestamp = 0;
};

// ============================================================
// VARIABLES GLOBALES OPTIMISÉES ✅
// ============================================================
enum Etat {
  BADGE,
  BIENVENUE,
  RYTHME_CARDIAQUE,
  TEMPERATURE,
  POIDS,
  AFFICHAGE
};
Etat etatActuel = BADGE;

// ⭐⭐ SUPPRIMER le cache EEPROM
Etudiant etudActuel;  // Un seul étudiant à la fois

// Variables inchangées...
DonneesBiometriques donnees;
EtatCapteurs etatCapteurs;
Configuration config;
WiFiStatus wifiStat;
MesureEnAttente queue[MAX_QUEUE_SIZE];
int queueSize = 0;

// ⭐⭐ SESSION RASPBERRY API - EYE DISEASE DETECTION
char sessionRaspberryId[50] = "";      // ID session retourné par Raspberry
unsigned long debutCaptureyeux = 0;    // Timestamp début capture
bool captureOeuxEnCours = false;       // Flag capture en cours
ResultatsOculaires resultatOeux;       // Résultats finaux oculaires

// Time management
unsigned long tempsDebutEtat = 0;
unsigned long dernierUpdateAffichage = 0;

// Pulse sensor variables - AMÉLIORÉ
const byte RATE_SIZE = 12;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;
unsigned long dernierAffichageIR = 0;

// 🫀 VARIABLES POUR ALGORITHME PULSEOX OPTIMISÉ
float spo2Buffer[SPO2_BUFFER_SIZE];  // Historique SpO2 (8 dernières mesures)
int spo2BufferIndex = 0;            // Index du buffer
float irDC = 0, irAC = 0;           // Composantes DC/AC du signal IR
float redDC = 0, redAC = 0;         // Composantes DC/AC du signal RED
float dernierIRValue = 0, dernierRedValue = 0;  // Dernières valeurs pour delta
int spo2Counter = 0;                // Compteur pour décimation

// Variables améliorées pour stabilité BPM
int compteurMesuresStables = 0;
int dernierBPMValide = 0;

// 🫀 VARIABLES POUR ALGORITHME BEAT FINDER PRO
float bpmBuffer[BPM_BUFFER_SIZE] = {0};  // Historique 6 valeurs BPM (dernière 10s)
int bpmBufferIndex = 0;                  // Index circulaire du buffer
float kalmanState = 0;                   // État estimé du filtre Kalman (BPM filtré)
float kalmanCovariance = 1.0;            // Incertitude du Kalman (covariance)
long lastBeatTime = 0;                   // Timestamp du dernier beat détecté (ms)
float bpmVariance = 0;                   // Variance des BPM récentes (pour adaptatif)
int beatPatternCount = 0;                // Nombre de beats valides consécutifs
float adaptativeTolerance = 10.0;        // TOLERANCE_BPM adaptatif (initialisation)
long detectionStartTime = 0;             // Timestamp début détection rapide (0.5-1s)
unsigned long debutMesureStable = 0;
bool mesureEnCours = false;

// Variables pour la gestion du poids améliorée
unsigned long derniereLecturePoids = 0;
const unsigned long INTERVALLE_LECTURE_POIDS = 100;
float historiquePoids[10];
int indexHistoriquePoids = 0;
unsigned long debutStabilisation = 0;
bool stabilisationEnCours = false;

// 🏋️ VARIABLES POUR ALGORITHME SMART WEIGHT OPTIMIZER
float weightBuffer[WEIGHT_VARIANCE_BUFFER_SIZE] = {0};  // Buffer variance 5 valeurs
int weightBufferIndex = 0;                              // Index circulaire
float kalmanWeightState = 0;                            // État Kalman poids
float kalmanWeightCovariance = 1.0;                     // Incertitude Kalman
long lastWeightReading = 0;                             // Timestamp dernière lecture
float weightVariance = 0;                               // Variance poids
float lastWeightValue = 0;                              // Poids précédent (détection mouvement)
bool movementDetected = false;                          // Flag mouvement
float measurementConfidence = 0;                        // Confiance 0-100%
int stableReadingCount = 0;                             // Lectures stables consécutives
long weightDetectionStartTime = 0;                      // Timer détection rapide

// Variables pour température
unsigned long derniereLectureTemperature = 0;
const unsigned long INTERVALLE_LECTURE_TEMP = 300;
float historiqueTemperature[5];
int indexHistoriqueTemperature = 0;
unsigned long debutStabilisationTemp = 0;
bool stabilisationTempEnCours = false;

// RFID et communication
unsigned long tRFID = 0;
String inBuf = "";
bool modeAdd = false, modeWiFiConfig = false;
int etapeAdd = 0, etapeWiFi = 0;
char newUID[20], newNom[30], newPrenom[30], newClasse[15];
char tempSSID[32], tempPass[64], tempURL[128];

// ============================================================
// FORWARD DECLARATIONS - BEAT FINDER PRO
void reinitialiserBufferSpO2();
float filtreKalmanBPM(float bpmMesu);
bool estBeatValide(long timeSinceLast);
float calculerToleranceAdaptative();
void reinitialiserBPMAlgorithme();

// FORWARD DECLARATIONS - SMART WEIGHT OPTIMIZER
float filtreKalmanPoids(float poidsMesu);
bool detecterMouvement(float poidsActuel);
float calculerConfiance();
void reinitialiserAlgorithmePoids();

// FONCTIONS JSON - ADAPTÉES POUR L'API ARDUINO ✅
// ============================================================
String creerJSONRecherche(String uid_rfid) {
  DynamicJsonDocument doc(200);
  doc["uid_rfid"] = uid_rfid;
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

String creerJSONMesures(int id_utilisateur, float poids, float temperature, int rythme_cardiaque, int spo2) {
  DynamicJsonDocument doc(300);
  doc["id_utilisateur"] = id_utilisateur;
  doc["poids"] = poids;
  doc["temperature"] = temperature;
  doc["rythme_cardiaque"] = rythme_cardiaque;
  doc["spo2"] = spo2;
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

// ============================================================
// DÉCLARATIONS FONCTIONS
// ============================================================
void beep(int freq, int duration);
void beepOK();
void reinitialiserBufferSpO2();  // Forward declaration for SpO2 algorithm

// ============================================================
// COMMUNICATION UART AVEC ESP32 ESCLAVE RFID
// ============================================================
void initRFIDSerial() {
  RFIDSerial.begin(115200, SERIAL_8N1, UART_RFID_RX, UART_RFID_TX);
  Serial.println("[UART] ✅ Communication RFID initialisée (RX=" + String(UART_RFID_RX) + ", TX=" + String(UART_RFID_TX) + ")");
}

String readRFIDSerial() {
  if (RFIDSerial.available()) {
    String message = RFIDSerial.readStringUntil('\n');
    message.trim();

    if (message.startsWith("UID:")) {
      String uid = message.substring(4);
      Serial.println("[RFID] 📨 UID reçu: " + uid);
      return uid;
    }
    else if (message == "ESCLAVE_READY") {
      Serial.println("[RFID] ✅ ESP32 esclave prêt");
    }
    else if (message.startsWith("RFID_VERSION:")) {
      Serial.println("[RFID] " + message);
    }
    else if (message.startsWith("ERREUR:")) {
      Serial.println("[RFID] ❌ " + message);
    }
    else if (message == "SANTE:OK") {
      static unsigned long lastHealth = 0;
      if (millis() - lastHealth > 25000) {
        Serial.println("[RFID] 💚 ESP32 esclave en bonne santé");
        lastHealth = millis();
      }
    }
  }
  return "";
}

bool sendCommand(const String& command, unsigned long timeout = 2000) {
  Serial.println("[UART] 📤 Envoi commande: " + command);
  RFIDSerial.println(command);

  unsigned long start = millis();
  while (millis() - start < timeout) {
    if (RFIDSerial.available()) {
      String response = RFIDSerial.readStringUntil('\n');
      response.trim();
      Serial.println("[UART] 📥 Réponse: " + response);
      return true;
    }
    delay(10);
  }
  Serial.println("[UART] ❌ Timeout - Pas de réponse");
  return false;
}

// ============================================================
// RFID - VERSION UART AVEC ESP32 ESCLAVE
// ============================================================
bool readBadge(char* buf) {
  if (millis() - tRFID < DELAI_ANTI_REBOND_RFID) return false;

  String uid = readRFIDSerial();
  if (uid.length() > 0) {
    strncpy(buf, uid.c_str(), 19);
    buf[19] = 0;
    tRFID = millis();
    beepBadgeDetected();
    return true;
  }
  return false;
}

void initRFID() {
  Serial.println("\n[RFID] === INITIALISATION COMMUNICATION ESP32 ESCLAVE ===");

  initRFIDSerial();
  delay(1000);

  Serial.println("[RFID] ⏳ Attente de l'ESP32 esclave...");
  unsigned long start = millis();
  bool esclavePret = false;

  while (millis() - start < 10000) {
    String message = readRFIDSerial();
    if (message == "ESCLAVE_READY") {
      esclavePret = true;
      break;
    }
    delay(100);
  }

  if (esclavePret) {
    Serial.println("[RFID] ✅ ESP32 esclave connecté et opérationnel");
    beepOK();
  } else {
    Serial.println("[RFID] ⚠️ ESP32 esclave non détecté - Mode offline");
    beepWarning();
  }

  Serial.println();
}

// ============================================================
// FONCTIONS D'INITIALISATION AVEC DEBUG
// ============================================================
void initTFT() {
  Serial.println("\n[TFT] === INITIALISATION ÉCRAN 480x320 ===");

  if (TFT_BL >= 0) {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BL_ON);
    Serial.println("[TFT] ✅ Rétroéclairage activé sur GPIO " + String(TFT_BL));
  } else {
    Serial.println("[TFT] ⚠️ LED connecté directement à 3.3V (toujours allumé)");
  }
  delay(200);

  tft.init();
  Serial.println("[TFT] ✅ Init TFT OK");
  delay(100);

  tft.setRotation(3);
  Serial.println("[TFT] ✅ Rotation 3 (paysage 480x320)");
  delay(100);

  pinMode(TOUCH_CS, OUTPUT);
  digitalWrite(TOUCH_CS, HIGH);

  tft.fillScreen(BACKGROUND_COLOR);
  Serial.println("[TFT] ✅ Initialization terminée - Résolution 480x320\n");
}

void initI2C() {
  Serial.println("[I2C] Initialization bus I2C (SDA=21, SCL=22)...");
  Wire.begin(21, 22);
  Wire.setClock(400000);
  Serial.println("[I2C] ✅ Bus I2C OK (400kHz)");
}

void initBuzzer() {
  Serial.println("[BUZZER] Initialization buzzer...");
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("[BUZZER] ✅ Buzzer OK sur GPIO " + String(BUZZER_PIN));
}

void initConfig() {
  Serial.println("[CONFIG] Configuration système...");

  // ✅ Utilisez directement le calFactor en kg (PAS de multiplication par 1000)
  config.calFactor = 23.23f;  // Valeur directe pour kg
  config.seuilPoidsDetection = 5.0;  // ✅ Réduit à 2 kg pour meilleure détection
  config.seuilBpmMin = 40;
  config.seuilBpmMax = 200;
  config.seuilIrDoigt = 15000;
  config.delaiMesurePoids = 5000;
  config.delaiAffichageResultats = 8000;

  Serial.println("[CONFIG] ✅ Configuration par défaut chargée");
  Serial.println("[CONFIG] CalFactor: " + String(config.calFactor));
}

// ============================================================
// BUZZER - MÉLODIES ET SONS
// ============================================================
void beep(int freq, int duration) {
  if (freq > 0) {
    tone(BUZZER_PIN, freq, duration);
  }
  delay(duration);
  noTone(BUZZER_PIN);
}

void beepOK() {
  beep(NOTE_C5, 100);
  delay(50);
  beep(NOTE_E5, 100);
}

void beepError() {
  beep(NOTE_E4, 150);
  delay(50);
  beep(NOTE_C4, 150);
}

void beepWarning() {
  beep(NOTE_G4, 100);
  delay(50);
  beep(NOTE_G4, 100);
}

void beepBadgeDetected() {
  beep(NOTE_G5, 80);
  delay(30);
  beep(NOTE_C5, 80);
}

void beepMesureComplete() {
  beep(NOTE_C5, 100);
  delay(30);
  beep(NOTE_E5, 100);
  delay(30);
  beep(NOTE_G5, 150);
}

void melodieStartup() {
  int melody[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5 };
  int durations[] = { 150, 150, 150, 300 };
  for (int i = 0; i < 4; i++) {
    beep(melody[i], durations[i]);
    delay(50);
  }
}

void melodieSuccess() {
  int melody[] = { NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5 };
  int durations[] = { 100, 100, 100, 100, 200 };
  for (int i = 0; i < 5; i++) {
    beep(melody[i], durations[i]);
    delay(30);
  }
}

void melodieAlert() {
  for (int i = 0; i < 3; i++) {
    beep(NOTE_A4, 100);
    delay(100);
  }
}

// ============================================================
// UTILITAIRES
// ============================================================
String tronc(const String& s, int max) {
  return (s.length() <= max) ? s : s.substring(0, max - 3) + "...";
}

void clear(int x, int y, int w, int h) {
  tft.fillRect(x, y, w, h, BACKGROUND_COLOR);
}

bool timeout() {
  return (millis() - tempsDebutEtat > TIMEOUT_ETAT);
}

// ============================================================
// WIFI
// ============================================================
bool initWiFi() {
  if (!WIFI_ENABLED) {
    Serial.println("[WIFI] WiFi désactivé");
    return false;
  }

  Serial.println("[WIFI] Connexion à: " + String(WIFI_SSID));
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  unsigned long start = millis();
  int dots = 0;
  while (WiFi.status() != WL_CONNECTED && millis() - start < WIFI_TIMEOUT) {
    delay(200);  // ✅ Réduit de 500 à 200ms
    yield();     // ✅ CRUCIAL: Laisser le watchdog respirer
    Serial.print(".");
    dots++;
    if (dots > 20) {
      Serial.println();
      dots = 0;
    }
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    wifiStat.connected = true;
    wifiStat.enabled = true;
    wifiStat.signalStrength = WiFi.RSSI();
    Serial.println("[WIFI] ✅ Connecté - IP: " + WiFi.localIP().toString());
    beepOK();
    delay(500);  // Petit délai avant audio
    envoyerSignalAudio(AUDIO_BADGE);  // 📢 001.mp3 après WiFi
    return true;
  } else {
    wifiStat.connected = false;
    Serial.println("[WIFI] ⚠️ Échec connexion - Mode offline");
    beepWarning();
    return false;
  }
}

// ============================================================
// FONCTION : ENVOYER SIGNAL AUDIO AU PC FEDORA
// ============================================================
bool envoyerSignalAudio(AudioCode code) {
  if (!wifiStat.connected) {
    Serial.println("[AUDIO] ⚠️ WiFi non connecté - audio ignoré");
    return false;
  }

  HTTPClient http;
  String url = "http://" + String(PC_AUDIO_IP) + ":" + String(PC_AUDIO_PORT) + "/play_audio";

  Serial.println("[AUDIO] 📢 Envoi signal audio " + String(code) + " au PC Fedora");

  http.begin(url);
  http.setTimeout(2000);  // Timeout 2 secondes
  http.addHeader("Content-Type", "application/json");

  String jsonPayload = "{\"audio_code\":" + String(code) + "}";

  int httpCode = http.POST(jsonPayload);

  if (httpCode == 200) {
    Serial.println("[AUDIO] ✅ Signal reçu par le PC");
    http.end();
    return true;
  } else {
    Serial.println("[AUDIO] ❌ Échec (code: " + String(httpCode) + ")");
    http.end();
    return false;
  }
}

void checkWiFi() {
  if (!wifiStat.enabled) return;
  if (millis() - wifiStat.lastCheck > 30000) {
    if (WiFi.status() == WL_CONNECTED) {
      wifiStat.connected = true;
      wifiStat.signalStrength = WiFi.RSSI();
    } else {
      wifiStat.connected = false;
      Serial.println("[WIFI] ⚠️ WiFi perdu - Reconnexion...");
      initWiFi();
    }
    wifiStat.lastCheck = millis();
  }
}

// ============================================================
// SERVEUR - FONCTIONS MODIFIÉES ✅
// ============================================================
bool rechercherEtudiantServeur(const char* uid, Etudiant* res) {
  if (!wifiStat.connected) return false;

  HTTPClient http;

  String url = API_ARDUINO + "?action=rechercher_etudiant";
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(SERVER_TIMEOUT);

  String jsonData = creerJSONRecherche(String(uid));
  Serial.println("[SERVER] 📤 Recherche: " + jsonData);

  int code = http.POST(jsonData);
  Serial.println("[SERVER] 📥 Code HTTP: " + String(code));

  if (code == 200) {
    String payload = http.getString();
    payload.trim();
    Serial.println("[SERVER] 📄 Réponse: " + payload);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload);

    if (!error && doc["success"] == true) {
      // Récupérer les données du serveur
      res->id = doc["etudiant"]["id_utilisateur"].as<int>();

      strncpy(res->uid, doc["etudiant"]["rfid_card"].as<String>().c_str(), 19);
      res->uid[19] = 0;
      strncpy(res->nom, doc["etudiant"]["nom"].as<String>().c_str(), 29);
      res->nom[29] = 0;
      strncpy(res->prenom, doc["etudiant"]["prenom"].as<String>().c_str(), 29);
      res->prenom[29] = 0;
      strncpy(res->classe, "Non spécifié", 14); // Ou récupérer depuis le serveur si disponible
      res->classe[14] = 0;
      res->actif = true;

      Serial.println("[SERVER] ✅ Étudiant trouvé - ID: " + String(res->id) +
                    ", Nom: " + String(res->prenom) + " " + String(res->nom) +
                    ", UID: " + String(res->uid));

      // ⭐⭐ PLUS d'appel à addEtud() - pas de sauvegarde EEPROM

      http.end();
      return true;
    } else {
      Serial.println("[SERVER] ❌ Erreur parsing JSON ou success=false");
      if (error) {
        Serial.println("[SERVER] ❌ Erreur JSON: " + String(error.c_str()));
      }
    }
  } else {
    Serial.println("[SERVER] ❌ Erreur HTTP: " + String(code));
  }
  http.end();
  return false;
}

// ============================================================
// RASPBERRY PI API - DÉTECTION MALADIES DES YEUX ✅
// ============================================================

// Fonction 1: POST /api/capture - Lance la capture + analyse
bool lancerCaptureOeuxRaspberry(int patient_id) {
  if (!wifiStat.connected) {
    Serial.println("[RASPBERRY] ❌ WiFi déconnecté");
    return false;
  }

  HTTPClient http;
  String url = String("http://") + String(RASPBERRY_IP) + ":" + String(RASPBERRY_PORT) + "/api/capture";

  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(SERVER_TIMEOUT);

  // Créer JSON: {"patient_id": 123}
  DynamicJsonDocument doc(128);
  doc["patient_id"] = patient_id;
  String jsonData;
  serializeJson(doc, jsonData);

  Serial.println("[RASPBERRY] 📤 POST /api/capture");
  Serial.println("[RASPBERRY] Données: " + jsonData);

  int code = http.POST(jsonData);
  Serial.println("[RASPBERRY] 📥 Code HTTP: " + String(code));

  if (code == 202 || code == 200) {
    String response = http.getString();
    response.trim();
    Serial.println("[RASPBERRY] 📄 Réponse: " + response);

    DynamicJsonDocument respDoc(256);
    DeserializationError err = deserializeJson(respDoc, response);

    if (!err && respDoc["status"] == "processing") {
      // Récupérer la session_id
      String sessionId = respDoc["session_id"].as<String>();
      sessionId.toCharArray(sessionRaspberryId, sizeof(sessionRaspberryId));

      captureOeuxEnCours = true;
      debutCaptureyeux = millis();

      Serial.println("[RASPBERRY] ✅ Capture lancée - Session ID: " + String(sessionRaspberryId));
      beepOK();
      http.end();
      return true;
    } else {
      Serial.println("[RASPBERRY] ❌ Réponse inattendue");
    }
  } else {
    Serial.println("[RASPBERRY] ❌ Erreur HTTP: " + String(code));
  }

  http.end();
  beepError();
  return false;
}

// Fonction 2: GET /api/results/<session_id> - Récupère les résultats
bool obtenirResultatsOeuxRaspberry(char* sessionId) {
  if (!wifiStat.connected) {
    return false;
  }

  HTTPClient http;
  String url = String("http://") + String(RASPBERRY_IP) + ":" + String(RASPBERRY_PORT) + "/api/results/" + String(sessionId);

  http.begin(url);
  http.setTimeout(SERVER_TIMEOUT);

  Serial.println("[RASPBERRY] 🔄 GET /api/results/" + String(sessionId));

  int code = http.GET();
  Serial.println("[RASPBERRY] 📥 Code HTTP: " + String(code));

  if (code == 200) {
    String response = http.getString();
    response.trim();

    DynamicJsonDocument respDoc(512);
    DeserializationError err = deserializeJson(respDoc, response);

    if (!err) {
      String status = respDoc["status"].as<String>();

      // [NEW] Signal de capture terminee - faire un bip pour signaler a l'utilisateur
      if (respDoc.containsKey("capture_done") && respDoc["capture_done"].as<bool>() == true) {
        Serial.println("[RASPBERRY] [CAPTURE DONE] Bip! L'utilisateur peut retirer ses yeux");
        beepOK();  // Bip sonore pour indiquer fin de capture
      }

      if (status == "completed") {
        // Résultats disponibles!
        if (respDoc.containsKey("oeil_1")) {
          respDoc["oeil_1"]["classe"].as<String>().toCharArray(resultatOeux.oeil_gauche, sizeof(resultatOeux.oeil_gauche));
          // Parser le pourcentage: "94.83%" -> 94.83
          String confStr = respDoc["oeil_1"]["confiance"].as<String>();
          confStr.replace("%", ""); // Enlever le %
          resultatOeux.oeil_gauche_conf = confStr.toFloat() / 100.0;  // Convertir en 0-1
        }

        if (respDoc.containsKey("oeil_2")) {
          respDoc["oeil_2"]["classe"].as<String>().toCharArray(resultatOeux.oeil_droit, sizeof(resultatOeux.oeil_droit));
          // Parser le pourcentage: "89.40%" -> 89.40
          String confStr = respDoc["oeil_2"]["confiance"].as<String>();
          confStr.replace("%", ""); // Enlever le %
          resultatOeux.oeil_droit_conf = confStr.toFloat() / 100.0;  // Convertir en 0-1
        }

        resultatOeux.alerte = respDoc["alerte"] ? 1 : 0;
        resultatOeux.disponibles = true;
        resultatOeux.timestamp = millis();

        Serial.println("[RASPBERRY] [OK] Resultats recus!");
        Serial.println("[RASPBERRY] OD: " + String(resultatOeux.oeil_droit) + " (" + String(resultatOeux.oeil_droit_conf, 2) + ")");
        Serial.println("[RASPBERRY] OG: " + String(resultatOeux.oeil_gauche) + " (" + String(resultatOeux.oeil_gauche_conf, 2) + ")");

        http.end();
        return true;
      } else if (status == "processing") {
        Serial.println("[RASPBERRY] [INFO] Analyse en cours...");
        // Continuer à attendre
      } else if (status == "error") {
        Serial.println("[RASPBERRY] [ERROR] Erreur lors de l'analyse");
        captureOeuxEnCours = false;
      }
    }
  } else if (code == 404) {
    Serial.println("[RASPBERRY] ❌ Session non trouvée");
    captureOeuxEnCours = false;
  }

  http.end();
  return false;
}

bool envoyerMesuresServeur(const char* uid, const char* nom, const char* prenom,
                           const char* classe, int bpm, int spo2, float temp, float poids,
                           const char* oeil_gauche = "", float oeil_gauche_conf = 0.0,
                           const char* oeil_droit = "", float oeil_droit_conf = 0.0, int alerte = 0) {
  if (!wifiStat.connected) return false;

  // ⭐⭐ VÉRIFICATION CRITIQUE de l'ID utilisateur
  if (etudActuel.id <= 0) {
    Serial.println("[SERVER] ❌ ERREUR CRITIQUE: id_utilisateur invalide (" + String(etudActuel.id) + ")");
    Serial.println("[SERVER] Impossible d'envoyer les mesures - ID manquant");
    return false;
  }

  HTTPClient http;

  String url = ADD_MESURE_URL + "?json=1";
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  http.setTimeout(SERVER_TIMEOUT);

  // Construire les données POST
  String postData = "id_utilisateur=" + String(etudActuel.id);
  postData += "&temperature=" + String(temp, 1);
  postData += "&rythme_cardiaque=" + String(bpm);
  postData += "&poids=" + String(poids, 1);
  postData += "&spo2=" + String(spo2);

  // ⭐⭐ AJOUTER les champs oculaires
  if (strlen(oeil_gauche) > 0) {
    postData += "&oeil_gauche=" + String(oeil_gauche);
    postData += "&oeil_gauche_conf=" + String(oeil_gauche_conf, 3);
  }
  if (strlen(oeil_droit) > 0) {
    postData += "&oeil_droit=" + String(oeil_droit);
    postData += "&oeil_droit_conf=" + String(oeil_droit_conf, 3);
  }
  postData += "&alerte=" + String(alerte);
  postData += "&submit=1";

  Serial.println("[SERVER] 📤 POST vers add_mesure.php");
  Serial.println("[SERVER] ID Utilisateur: " + String(etudActuel.id));
  Serial.println("[SERVER] Données: " + postData);

  int code = http.POST(postData);
  Serial.println("[SERVER] 📥 Code HTTP: " + String(code));

  if (code == 200) {
    String response = http.getString();
    response.trim();

    Serial.println("[SERVER] 📄 Réponse complète: " + response);

    // Essayer de parser la réponse JSON
    DynamicJsonDocument respDoc(512);
    DeserializationError err = deserializeJson(respDoc, response);

    if (!err) {
      bool succes = (respDoc["success"] == true);
      if (succes) {
        Serial.println("[SERVER] ✅ Mesures enregistrées avec succès!");
        Serial.println("[SERVER] ID Mesure: " + String(respDoc["id_mesure"].as<String>()));
        beepOK();
        http.end();
        return true;
      } else {
        const char* msg = respDoc["message"] | "Erreur serveur";
        Serial.println(String("[SERVER] ❌ Erreur serveur: ") + String(msg));
        http.end();
        beepError();
        return false;
      }
    } else {
      // Fallback: vérifier dans le HTML
      if (response.indexOf("Mesure ajoutée") >= 0 || response.indexOf("succès") >= 0) {
        Serial.println("[SERVER] ✅ Mesures enregistrées (détection texte)");
        beepOK();
        http.end();
        return true;
      } else {
        Serial.println("[SERVER] ❌ Réponse inattendue du serveur");
        Serial.println("[SERVER] Début réponse: " + response.substring(0, 200));
        http.end();  // ✅ CRUCIAL: Libérer la ressource WiFi
      }
    }
  } else {
    Serial.println("[SERVER] ❌ Erreur HTTP: " + String(code));
  }

  http.end();
  beepError();
  return false;
}

// ============================================================
// FONCTION DE RECHERCHE ÉTUDIANT - UNIQUEMENT SERVEUR
// ============================================================
bool findEtud(const char* uid, Etudiant* res) {
  // ⭐⭐ UNIQUEMENT recherche serveur - plus de cache EEPROM

  if (wifiStat.connected) {
    Serial.println("[SERVER] 🔍 Recherche sur le serveur pour UID: " + String(uid));
    return rechercherEtudiantServeur(uid, res);
  } else {
    Serial.println("[SERVER] ❌ WiFi déconnecté - Impossible de rechercher l'étudiant");
    return false;
  }
}

// ============================================================
// QUEUE
// ============================================================
void ajouterQueue(const char* uid, const char* nom, const char* prenom,
                  const char* classe, int bpm, int spo2, float temp, float poids) {
  if (queueSize >= MAX_QUEUE_SIZE) {
    for (int i = 0; i < queueSize - 1; i++) queue[i] = queue[i + 1];
    queueSize--;
  }

  strncpy(queue[queueSize].uid, uid, 19);
  queue[queueSize].uid[19] = 0;
  strncpy(queue[queueSize].nom, nom, 29);
  queue[queueSize].nom[29] = 0;
  strncpy(queue[queueSize].prenom, prenom, 29);
  queue[queueSize].prenom[29] = 0;
  strncpy(queue[queueSize].classe, classe, 14);
  queue[queueSize].classe[14] = 0;
  queue[queueSize].bpm = bpm;
  queue[queueSize].spo2 = spo2;
  queue[queueSize].temp = temp;
  queue[queueSize].poids = poids;
  queue[queueSize].timestamp = millis();
  queue[queueSize].tentatives = 0;
  queue[queueSize].actif = true;
  queueSize++;
  Serial.println("[QUEUE] Ajout (" + String(queueSize) + ")");
}

void traiterQueue() {
  if (!wifiStat.connected || queueSize == 0) return;

  // ✅ AMÉLIORATION: Traiter UNE SEULE mesure par appel (non-bloquant)
  static int indexQueue = 0;

  if (indexQueue >= queueSize) {
    indexQueue = 0;
    return;
  }

  if (!queue[indexQueue].actif) {
    indexQueue++;
    return;
  }

  bool success = envoyerMesuresServeur(
    queue[indexQueue].uid, queue[indexQueue].nom, queue[indexQueue].prenom, queue[indexQueue].classe,
    queue[indexQueue].bpm, queue[indexQueue].spo2, queue[indexQueue].temp, queue[indexQueue].poids);

  if (success) {
    queue[indexQueue].actif = false;
    for (int j = indexQueue; j < queueSize - 1; j++) queue[j] = queue[j + 1];
    queueSize--;
    indexQueue = 0;  // Recommencer depuis le début
    Serial.println("[QUEUE] ✅ Une mesure envoyée - Queue: " + String(queueSize) + " restantes");
  } else {
    queue[indexQueue].tentatives++;
    if (queue[indexQueue].tentatives >= MAX_RETRY_SEND) {
      queue[indexQueue].actif = false;
      for (int j = indexQueue; j < queueSize - 1; j++) queue[j] = queue[j + 1];
      queueSize--;
      indexQueue = 0;
      Serial.println("[QUEUE] ❌ Mesure abandonnée après " + String(MAX_RETRY_SEND) + " tentatives");
    } else {
      indexQueue++;
      Serial.println("[QUEUE] ⚠️ Tentative " + String(queue[indexQueue].tentatives) + " échouée");
    }
  }

  yield();  // ✅ Laisser respirer le watchdog
}

// ============================================================
// GESTION ÉNERGÉTIQUE DES CAPTEURS - VERSION AMÉLIORÉE ✅
// ============================================================
void activerCapteurCardiaque() {
  if (!etatCapteurs.max30102Actif) {
    Serial.println("Activation capteur cardiaque...");

    Wire.begin();
    Wire.setClock(400000);

    if (particleSensor.begin(Wire, I2C_SPEED_FAST)) {
      // Configuration optimisée pour stabilité
      particleSensor.setup(0x5F, 4, 2, 400, 411, 4096);
      particleSensor.setPulseAmplitudeRed(0x1F);
      particleSensor.setPulseAmplitudeIR(0x1F);
      etatCapteurs.max30102Actif = true;
      etatCapteurs.ledBright = 0x1F;
      
      // 🎯 Réinitialiser les buffers SpO2 pour nouvelles mesures
      reinitialiserBufferSpO2();
      
      Serial.println("Capteur cardiaque active - Mode stable");
    } else {
      Serial.println("ERREUR: Capteur cardiaque non detecte!");
    }
  }
}

void desactiverCapteurCardiaque() {
  if (etatCapteurs.max30102Actif) {
    particleSensor.shutDown();
    etatCapteurs.max30102Actif = false;
    etatCapteurs.doigtPresent = false;
  }
}

void activerCapteurTemperature() {
  if (!etatCapteurs.mlx90614Actif) {
    Serial.println("Activation capteur temperature...");

    Wire.begin();
    Wire.setClock(100000);

    if (mlx.begin()) {
      etatCapteurs.mlx90614Actif = true;
      Serial.println("Capteur temperature active");
    } else {
      Serial.println("ERREUR: Capteur temperature non detecte!");
    }
  }
}

void desactiverCapteurTemperature() {
  if (etatCapteurs.mlx90614Actif) {
    etatCapteurs.mlx90614Actif = false;
  }
}

void activerCapteurPoids() {
  if (!etatCapteurs.hx711Actif) {
    Serial.println("Activation capteur poids...");

    scale.begin();
    unsigned long stabilizingtime = 1500;
    boolean _tare = true;

    scale.start(stabilizingtime, _tare);

    if (scale.getTareTimeoutFlag() || scale.getSignalTimeoutFlag()) {
      Serial.println("ERREUR: Capteur poids non detecte!");
      return;
    }

    // ✅ CORRECTION : Utilisez directement calFactor (déjà en kg)
    scale.setCalFactor(config.calFactor);

    Serial.println("[POIDS] Calibration: " + String(config.calFactor));

    etatCapteurs.hx711Actif = true;
    Serial.println("Capteur poids active");
  }
}

void desactiverCapteurPoids() {
  if (etatCapteurs.hx711Actif) {
    scale.powerDown();
    etatCapteurs.hx711Actif = false;
    etatCapteurs.nouvelleLecturePoids = false;
  }
}

void desactiverTousCapteurs() {
  desactiverCapteurCardiaque();
  desactiverCapteurTemperature();
  desactiverCapteurPoids();
}

// ============================================================
// FONCTIONS D'AFFICHAGE OPTIMISÉES POUR 480x320 ✅
// ============================================================
void dessinerHeader(const String& titre) {
  tft.fillRect(0, 0, SCREEN_W, 60, PRIMARY_COLOR);
  tft.setTextColor(BACKGROUND_COLOR, PRIMARY_COLOR);
  tft.drawCentreString(tronc(titre, 25), SCREEN_W / 2, 15, 4);

  if (wifiStat.enabled) {
    int x = SCREEN_W - 40, y = 10;
    if (wifiStat.connected) {
      tft.fillCircle(x, y + 20, 4, SUCCESS_COLOR);
      tft.fillCircle(x, y + 15, 3, SUCCESS_COLOR);
    } else {
      tft.drawCircle(x, y + 20, 4, ERROR_COLOR);
    }
  }
}

void afficherStatut(const String& texte, uint16_t couleur) {
  clear(0, 180, SCREEN_W, 30);
  tft.setTextColor(couleur, BACKGROUND_COLOR);
  tft.drawCentreString(tronc(texte, 45), SCREEN_W / 2, 185, 2);
}

void afficherValeurIR(long irValue) {
  clear(350, 270, 120, 20);
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawString("IR: " + String(irValue), 350, 270, 1);
}

void afficherBPM(int valeurBPM) {
  clear(80, 80, 150, 60);
  uint16_t couleur = TEXT_COLOR;
  if (valeurBPM == 0) {
    couleur = WARNING_COLOR;
  } else if (valeurBPM >= 60 && valeurBPM <= 100) {
    couleur = SUCCESS_COLOR;
  } else if (valeurBPM > 100 && valeurBPM <= 120) {
    couleur = SECONDARY_COLOR;
  } else {
    couleur = ERROR_COLOR;
  }

  tft.setTextColor(couleur, BACKGROUND_COLOR);
  String texte = (valeurBPM > 0) ? String(valeurBPM) : "--";
  tft.drawCentreString(texte, 155, 80, 8);

  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("BPM", 155, 140, 2);
}

void afficherSpO2(int valeurSpO2) {
  clear(250, 80, 150, 60);
  uint16_t couleur = TEXT_COLOR;
  if (valeurSpO2 == 0) {
    couleur = WARNING_COLOR;
  } else if (valeurSpO2 >= 95) {
    couleur = SUCCESS_COLOR;
  } else if (valeurSpO2 >= 90) {
    couleur = SECONDARY_COLOR;
  } else {
    couleur = ERROR_COLOR;
  }

  tft.setTextColor(couleur, BACKGROUND_COLOR);
  String texte = (valeurSpO2 > 0) ? String(valeurSpO2) + "%" : "--%";
  tft.drawCentreString(texte, 325, 80, 8);

  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("SpO2", 325, 140, 2);
}

void afficherPoids(float valeurPoids) {
  clear(150, 160, 180, 60);

  // ✅ CORRECTION : Affichage cohérent en kg
  tft.setTextColor(valeurPoids > 0 ? PRIMARY_COLOR : WARNING_COLOR, BACKGROUND_COLOR);
  String texte = (valeurPoids > 0) ? String(valeurPoids, 1) + " kg" : "--.- kg";
  tft.drawCentreString(texte, 240, 160, 6);
}

void afficherTemperature(float valeurTemperature) {
  clear(150, 160, 180, 60);

  uint16_t couleur;
  if (valeurTemperature >= TEMPERATURE_FIEVRE) {
    couleur = TEMP_HIGH_COLOR;
  } else if (valeurTemperature >= TEMPERATURE_FIEVRE_LEGERE) {
    couleur = TEMP_WARNING_COLOR;
  } else if (valeurTemperature >= TEMPERATURE_NORMALE_MIN && valeurTemperature <= TEMPERATURE_NORMALE_MAX) {
    couleur = TEMP_NORMAL_COLOR;
  } else {
    couleur = WARNING_COLOR;
  }

  tft.setTextColor(couleur, BACKGROUND_COLOR);
  String texte = (valeurTemperature > 0) ? String(valeurTemperature, 1) + " °C" : "--.- °C";
  tft.drawCentreString(texte, 240, 160, 6);
}

void afficherQualiteSignal(float qualite) {
  int x = 50, y = 220, largeur = 150, hauteur = 10;

  clear(x, y-20, 250, 30);
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawString("Signal:", x, y-20, 1);

  tft.drawRect(x, y, largeur, hauteur, TEXT_COLOR);

  uint16_t couleur = ERROR_COLOR;
  if (qualite > 75) couleur = SUCCESS_COLOR;
  else if (qualite > 50) couleur = SECONDARY_COLOR;
  else if (qualite > 25) couleur = WARNING_COLOR;

  int largeurBarre = (largeur - 2) * qualite / 100.0;
  if (largeurBarre > 0) {
    tft.fillRect(x + 1, y + 1, largeurBarre, hauteur - 2, couleur);
  }

  tft.setTextColor(couleur, BACKGROUND_COLOR);
  tft.drawString(String((int)qualite) + "%", x + largeur + 10, y-20, 1);
}

void afficherProgressionStabilite(int compteur, int total) {
  clear(50, 240, 200, 20);
  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("Stability: " + String(compteur) + "/" + String(total), 50, 240, 1);
}

// ============================================================
// ÉCRAN BIENVENUE PERSONNALISÉ - NOUVEAU ✅
// ============================================================
void afficherEcranBienvenueEtudiant() {
  tft.fillScreen(BACKGROUND_COLOR);

  // Cadre décoratif
  tft.drawRoundRect(20, 20, SCREEN_W - 40, SCREEN_H - 40, 15, PRIMARY_COLOR);
  tft.drawRoundRect(22, 22, SCREEN_W - 44, SCREEN_H - 44, 13, HIGHLIGHT_COLOR);

  // Titre principal
  tft.setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("WELCOME", SCREEN_W / 2, 50, 6);

  // Message de salutation
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("Hello", SCREEN_W / 2, 110, 4);

  // Prénom en grand
  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString(tronc(String(etudActuel.prenom), 20), SCREEN_W / 2, 150, 8);

  // Nom
  tft.setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString(tronc(String(etudActuel.nom), 20), SCREEN_W / 2, 200, 4);

  // Classe
  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString(tronc(String(etudActuel.classe), 18), SCREEN_W / 2, 240, 2);

  // Message de préparation
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("Preparation of measurements...", SCREEN_W / 2, 270, 2);

  // Animation de chargement
  int barX = 80, barY = 290, barW = SCREEN_W - 160, barH = 8;
  tft.drawRect(barX, barY, barW, barH, TEXT_COLOR);

  for (int i = 0; i <= 100; i += 5) {
    int fillW = (barW - 2) * i / 100;
    tft.fillRect(barX + 1, barY + 1, fillW, barH - 2, PRIMARY_COLOR);

    if (i % 25 == 0) {
      beep(NOTE_C4 + (i / 25) * 100, 50);
    }

    delay(60);
  }

  beepOK();
}

void afficherEcranBienvenue() {
  tft.fillScreen(BACKGROUND_COLOR);

  tft.setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("MEDICAL SYSTEM", SCREEN_W / 2, 80, 6);
  tft.drawCentreString("BIOMETRIC", SCREEN_W / 2, 140, 6);

  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("BPM + SpO2 + WEIGHT + TEMP", SCREEN_W / 2, 190, 2);

  tft.drawCentreString("Initialization...", SCREEN_W / 2, 240, 2);

  int barX = (SCREEN_W - 300) / 2;
  int barY = 270;
  for(int i = 0; i <= 100; i += 10) {
    tft.drawRect(barX, barY, 300, 12, TEXT_COLOR);
    tft.fillRect(barX + 1, barY + 1, (298 * i) / 100, 10, PRIMARY_COLOR);
    delay(50);
  }
}

// ============================================================
// GESTION POIDS AMÉLIORÉE ✅
// ============================================================
float lirePoidsFiltre() {
  static unsigned long dernierUpdate = 0;
  static float poidsFiltre = 0;
  static bool initialise = false;

  if (millis() - dernierUpdate >= INTERVALLE_LECTURE_POIDS) {
    if (scale.update()) {
      float nouveauPoids = scale.getData();

      // ✅✅✅ SEULE MODIFICATION NÉCESSAIRE : Conversion grammes → kilogrammes
      nouveauPoids = nouveauPoids / 1000.0;

      // ✅ Initialization du filtre
      if (!initialise) {
        poidsFiltre = nouveauPoids;
        initialise = true;
      } else {
        const float alpha = 0.7;
        poidsFiltre = alpha * nouveauPoids + (1 - alpha) * poidsFiltre;
      }

      dernierUpdate = millis();
      etatCapteurs.nouvelleLecturePoids = true;

      Serial.println("[POIDS] Brut: " + String(nouveauPoids, 2) + " kg | Filtre: " + String(poidsFiltre, 2) + " kg");

      return poidsFiltre;
    }
  }

  return poidsFiltre;
}
bool verifierStabilitePoids(float poidsActuel) {
  // Ajouter à l'historique
  historiquePoids[indexHistoriquePoids] = poidsActuel;
  indexHistoriquePoids = (indexHistoriquePoids + 1) % NOMBRE_LECTURES_STABILITE;

  // ✅ CORRECTION : Ignorer les valeurs à 0 (non initialisées)
  float valeurs[NOMBRE_LECTURES_STABILITE];
  int nbValeursValides = 0;

  for (int i = 0; i < NOMBRE_LECTURES_STABILITE; i++) {
    if (historiquePoids[i] > 0.1) {  // Ignorer les valeurs proches de 0
      valeurs[nbValeursValides++] = historiquePoids[i];
    }
  }

  // ✅ Attendre d'avoir suffisamment de valeurs
  if (nbValeursValides < NOMBRE_LECTURES_STABILITE) {
    Serial.println("[STABILITE] En attente de " + String(NOMBRE_LECTURES_STABILITE - nbValeursValides) + " lectures");
    return false;
  }

  // Calculer min/max sur les valeurs valides
  float minPoids = valeurs[0];
  float maxPoids = valeurs[0];

  for (int i = 1; i < nbValeursValides; i++) {
    if (valeurs[i] < minPoids) minPoids = valeurs[i];
    if (valeurs[i] > maxPoids) maxPoids = valeurs[i];
  }

  float variation = maxPoids - minPoids;

  Serial.println("[STABILITE] Min: " + String(minPoids, 2) + " | Max: " + String(maxPoids, 2) +
  " | Variation: " + String(variation, 2) + " kg");

  // ✅ Vérifier stabilité ET poids suffisant
  bool stable = (variation <= VARIATION_MAX_STABILITE) && (poidsActuel > config.seuilPoidsDetection);

  if (stable) {
    if (!stabilisationEnCours) {
      stabilisationEnCours = true;
      debutStabilisation = millis();
      Serial.println("[STABILITE] ✅ Début stabilisation");
    }

    unsigned long dureeStabilisation = millis() - debutStabilisation;
    bool stabilisationComplete = (dureeStabilisation >= TEMPS_STABILITE_REQUIS);

    if (stabilisationComplete) {
      Serial.println("[STABILITE] ✅✅ STABILISATION COMPLETE (" + String(dureeStabilisation) + " ms)");
    }

    return stabilisationComplete;
  } else {
    if (stabilisationEnCours) {
      Serial.println("[STABILITE] ❌ Stabilisation perdue");
    }
    stabilisationEnCours = false;
    debutStabilisation = 0;
    return false;
  }
}

// ============================================================
// GESTION TEMPÉRATURE AMÉLIORÉE ✅
// ============================================================
// ============================================================
// COMPENSATION DE TEMPÉRATURE MLX90614 - ALGORITHME OPTIMISÉ
// ============================================================
float calculerFacteurK(float tempAmbiente) {
  // Adaptation dynamique du facteur K selon la température ambiante
  if (tempAmbiente < 18.0) {
    return 0.18;  // Compensation forte (pièce froide)
  } else if (tempAmbiente <= 25.0) {
    // Interpolation linéaire entre 18°C et 25°C
    return 0.18 - (tempAmbiente - 18.0) * (0.18 - 0.12) / 7.0;
  } else {
    return 0.06;  // Compensation faible (pièce chaude)
  }
}

float compenserTemperatureCorps(float tObj, float tAmb) {
  // Formule de compensation : T_body = T_obj + K(T_amb) * (T_obj - T_amb) + OFFSET
  float k = calculerFacteurK(tAmb);
  float tBody = tObj + k * (tObj - tAmb) + 0.8;  // +0.8°C offset clinique
  
  Serial.print("[TEMP] Brute: "); Serial.print(tObj);
  Serial.print(" | Amb: "); Serial.print(tAmb);
  Serial.print(" | K: "); Serial.print(k, 3);
  Serial.print(" | Compensation: "); Serial.println(tBody);
  
  return tBody;
}


float lireTemperatureFiltree() {
  static unsigned long dernierUpdate = 0;
  static float temperatureFiltree = 0;

  if (millis() - dernierUpdate >= INTERVALLE_LECTURE_TEMP) {
    if (etatCapteurs.mlx90614Actif) {
      float tObj = mlx.readObjectTempC();     // Température surface (front)
      float tAmb = mlx.readAmbientTempC();    // Température ambiante
      
      // ✅ Appliquer la compensation optimisée
      float tCompensee = compenserTemperatureCorps(tObj, tAmb);

      // ✅ Filtre exponentiel sur température compensée
      const float alpha = 0.5;
      temperatureFiltree = alpha * tCompensee + (1 - alpha) * temperatureFiltree;

      donnees.temperatureAmbiante = tAmb;  // Stocker pour logs
      dernierUpdate = millis();
      return temperatureFiltree;
    }
  }

  return temperatureFiltree;
}

bool verifierStabiliteTemperature(float temperatureActuelle) {
  historiqueTemperature[indexHistoriqueTemperature] = temperatureActuelle;
  indexHistoriqueTemperature = (indexHistoriqueTemperature + 1) % 5;

  float minTemp = historiqueTemperature[0];
  float maxTemp = historiqueTemperature[0];

  for (int i = 1; i < 5; i++) {
    if (historiqueTemperature[i] < minTemp) minTemp = historiqueTemperature[i];
    if (historiqueTemperature[i] > maxTemp) maxTemp = historiqueTemperature[i];
  }

  float variation = maxTemp - minTemp;
  bool stable = (variation <= 0.15);  // 🎯 Seuil ajusté pour temp compensée

  if (stable) {
    if (!stabilisationTempEnCours) {
      stabilisationTempEnCours = true;
      debutStabilisationTemp = millis();
    }

    return (millis() - debutStabilisationTemp >= 1500);
  } else {
    stabilisationTempEnCours = false;
    debutStabilisationTemp = 0;
    return false;
  }
}

// ============================================================
// ALGORITHME PULSEOX OPTIMISÉ ✅ (Professionnel MAX30102)
// ============================================================

// Filtrer DC (moyenne = baseline)
float filtrerDC(float nouvelleMesure, float valeurActuelle, float alpha) {
  return alpha * valeurActuelle + (1 - alpha) * nouvelleMesure;
}

// Calculer AC (variation autour de DC)
float calculerAC(float mesure, float dc) {
  return mesure - dc;
}

// Fonction principale : Calcul SpO2 optimisé
void calculerSpO2(long irValue, long redValue) {
  if (irValue < config.seuilIrDoigt) {
    donnees.spo2 = 0;
    return;
  }

  // 🎯 ÉTAPE 1 : Filtrer les composantes DC/AC pour IR et RED
  irDC = filtrerDC((float)irValue, irDC, SPO2_DC_ALPHA);
  redDC = filtrerDC((float)redValue, redDC, SPO2_DC_ALPHA);
  
  irAC = calculerAC((float)irValue, irDC);
  redAC = calculerAC((float)redValue, redDC);

  // 🎯 ÉTAPE 2 : Calculer le ratio (AC/DC)
  // Éviter division par zéro
  if (irDC == 0 || redDC == 0) {
    donnees.spo2 = 95;  // Valeur par défaut si capteur inactive
    return;
  }

  float ratioAC = (abs(redAC) / redDC) / (abs(irAC) / irDC);
  
  // 🎯 ÉTAPE 3 : Appliquer la formule polynomiale (ordre 2)
  // SpO2 = A + B × (Ratio)²
  float spo2_value = SPO2_COEFF_A + SPO2_COEFF_B * (ratioAC * ratioAC);

  // 🎯 ÉTAPE 4 : Clamp entre 70-100%
  if (spo2_value < 70) spo2_value = 70;
  if (spo2_value > 100) spo2_value = 100;

  // 🎯 ÉTAPE 5 : Ajouter à l'historique (moyenning sur 8 mesures)
  spo2Buffer[spo2BufferIndex] = spo2_value;
  spo2BufferIndex = (spo2BufferIndex + 1) % SPO2_BUFFER_SIZE;

  // 🎯 ÉTAPE 6 : Calculer la moyenne lissée
  float moyenneSpO2 = 0;
  for (int i = 0; i < SPO2_BUFFER_SIZE; i++) {
    moyenneSpO2 += spo2Buffer[i];
  }
  moyenneSpO2 /= SPO2_BUFFER_SIZE;

  donnees.spo2 = (int)(moyenneSpO2 + 0.5);  // Arrondir

  // 🔍 DEBUG : Afficher les détails en Serial
  if (spo2Counter++ % 10 == 0) {  // Log tous les 10 calculs
    Serial.print("[SPO2] IR: "); Serial.print(irValue);
    Serial.print(" | RED: "); Serial.print(redValue);
    Serial.print(" | DC_IR: "); Serial.print(irDC, 0);
    Serial.print(" | DC_RED: "); Serial.print(redDC, 0);
    Serial.print(" | Ratio: "); Serial.print(ratioAC, 3);
    Serial.print(" | SpO2_raw: "); Serial.print(spo2_value, 1);
    Serial.print(" | SpO2_avg: "); Serial.print(moyenneSpO2, 1);
    Serial.print(" | Final: "); Serial.println(donnees.spo2);
  }
}

// Réinitialiser le buffer SpO2 (utile après changement d'état)
void reinitialiserBufferSpO2() {
  for (int i = 0; i < SPO2_BUFFER_SIZE; i++) {
    spo2Buffer[i] = 95;  // Valeur neutre
  }
  spo2BufferIndex = 0;
  irDC = 0;
  irAC = 0;
  redDC = 0;
  redAC = 0;
  spo2Counter = 0;
}

// ============================================================
// GESTION DES ÉTATS AVEC ACTIVATION CAPTEURS ✅
// ============================================================
void changerEtat(Etat nouvelEtat) {
  Serial.println("\n[ETAT] ========================================");
  Serial.println("[ETAT] Changement d'état demandé");
  Serial.println("[ETAT] État actuel: " + String(etatActuel));
  Serial.println("[ETAT] Nouvel état: " + String(nouvelEtat));
  Serial.println("[ETAT] ========================================");

  desactiverTousCapteurs();

  etatActuel = nouvelEtat;
  tempsDebutEtat = millis();
  dernierUpdateAffichage = 0;

  switch (etatActuel) {
    case BADGE:
      setupBadge();
      break;

    case BIENVENUE:
      setupBienvenue();
      // Audio envoyé directement dans setupBienvenue()
      break;

    case RYTHME_CARDIAQUE:
      setupRythmeCardiaque();
      envoyerSignalAudio(AUDIO_RYTHME);  // 📢 003.mp3
      break;

    case TEMPERATURE:
      setupTemperature();
      envoyerSignalAudio(AUDIO_TEMPERATURE);  // 📢 004.mp3
      break;

    case POIDS:
      setupPoids();
      envoyerSignalAudio(AUDIO_POIDS);  // 📢 005.mp3
      break;

    case AFFICHAGE:
      setupAffichage();
      envoyerSignalAudio(AUDIO_FIN);  // 📢 006.mp3
      break;

    default:
      break;
  }
}

// ============================================================
// ÉTAT BADGE - RFID
// ============================================================
void setupBadge() {
  Serial.println("\n[BADGE] ===== DEBUT SETUP BADGE 480x320 =====");

  // 🎯 BUG FIX: Réinitialiser l'utilisateur et les données précédents
  memset(&etudActuel, 0, sizeof(Etudiant));
  donnees.bpm = 0;
  donnees.spo2 = 0;
  donnees.poids = 0;
  donnees.temperature = 0;
  donnees.bpmStable = 0;
  donnees.spo2Stable = 0;
  donnees.temperatureStable = 0;
  Serial.println("[BADGE] ✅ Utilisateur et données réinitialisées - En attente nouveau badge");

  tft.fillScreen(BACKGROUND_COLOR);

  // Cadre principal
  tft.drawRoundRect(10, 10, SCREEN_W - 20, SCREEN_H - 20, 20, PRIMARY_COLOR);

  tft.setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("MEDICAL ROBOT", SCREEN_W / 2, 60, 6);
  tft.drawCentreString("SCHOOL", SCREEN_W / 2, 120, 6);

  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("Present your RFID card", SCREEN_W / 2, 180, 2);

  // Section informations système
  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("=== SYSTEM STATUS ===", SCREEN_W / 2, 210, 1);

  if (wifiStat.enabled) {
    String wifiTxt = wifiStat.connected ? "WiFi: Connecte" : "WiFi: Hors ligne";
    uint16_t wifiCol = wifiStat.connected ? SUCCESS_COLOR : WARNING_COLOR;
    tft.setTextColor(wifiCol, BACKGROUND_COLOR);
    tft.drawCentreString(wifiTxt, SCREEN_W / 2, 230, 1);
  }

  if (queueSize > 0) {
    tft.setTextColor(WARNING_COLOR, BACKGROUND_COLOR);
    tft.drawCentreString("En attente: " + String(queueSize) + " mesure(s)", SCREEN_W / 2, 250, 1);
  }

  // Icône badge animée
  int x = SCREEN_W / 2 - 25, y = 270;
  tft.drawRoundRect(x, y, 50, 35, 8, PRIMARY_COLOR);
  tft.fillRoundRect(x + 10, y + 8, 30, 20, 5, PRIMARY_COLOR);

  Serial.println("[BADGE] ✅✅✅ ECRAN BADGE 480x320 COMPLET ✅✅✅");
}

void loopBadge() {
  char uid[20];
  if (readBadge(uid)) {
    Serial.println("[RFID] Badge détecté: " + String(uid));

    // Réinitialiser etudActuel avant la recherche
    memset(&etudActuel, 0, sizeof(Etudiant));

    if (findEtud(uid, &etudActuel)) {
      Serial.println("[RFID] ✅ Étudiant trouvé: " + String(etudActuel.prenom) + " " + String(etudActuel.nom));
      Serial.println("[RFID] ID Étudiant: " + String(etudActuel.id));
      Serial.println("[RFID] UID: " + String(etudActuel.uid));

      // Vérification critique
      if (etudActuel.id == 0) {
        Serial.println("[RFID] ⚠️ ATTENTION: ID étudiant est 0 !");
      }

      changerEtat(BIENVENUE);
    } else {
      Serial.println("[RFID] ❌ Étudiant inconnu");
      tft.fillScreen(BACKGROUND_COLOR);
      dessinerHeader("UNKNOWN BADGE");
      tft.setTextColor(ERROR_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("Not registered", SCREEN_W / 2, 120, 4);
      tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("UID: " + tronc(String(uid), 20), SCREEN_W / 2, 170, 2);
      tft.drawCentreString("Contact administration", SCREEN_W / 2, 210, 1);
      beepError();
      delay(3000);
      changerEtat(BADGE);
    }
  }

  static unsigned long lastQueue = 0;
  if (millis() - lastQueue > 5000) {
    traiterQueue();
    lastQueue = millis();
  }
  checkWiFi();

  static unsigned long tAnim = 0;
  static int f = 0;
  if (millis() - tAnim > 800) {
    int x = SCREEN_W / 2 - 25, y = 270;
    clear(x, y, 50, 40);
    tft.drawRoundRect(x, y, 50, 35, 8, PRIMARY_COLOR);
    if (f % 2 == 0) tft.fillRoundRect(x + 10, y + 8, 30, 20, 5, PRIMARY_COLOR);
    f++;
    tAnim = millis();
  }
}

// ============================================================
// FONCTIONS D'AFFICHAGE BIENVENUE ✅ NOUVELLES
// ============================================================

// Affichage "Rapprochez votre visage" avec countdown NON-BLOQUANT
void afficherRapprochezVisage() {
  tft.fillScreen(BACKGROUND_COLOR);
  dessinerHeader("EYE ANALYSIS");

  // Texte principal
  tft.setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("BRING YOUR FACE CLOSER", SCREEN_W / 2, 80, 4);
  tft.drawCentreString("IN FRONT OF THE CAMERA", SCREEN_W / 2, 130, 4);

  // Dessiner cadre visage stylisé
  int cx = SCREEN_W / 2;
  int cy = 200;
  tft.drawRoundRect(cx - 90, cy - 110, 180, 220, 15, HIGHLIGHT_COLOR);
  tft.fillCircle(cx - 40, cy - 40, 15, PRIMARY_COLOR);   // Œil gauche
  tft.fillCircle(cx + 40, cy - 40, 15, PRIMARY_COLOR);   // Œil droit
  tft.drawLine(cx - 20, cy + 10, cx + 20, cy + 10, PRIMARY_COLOR); // Nez
  tft.drawArc(cx, cy + 40, 30, 25, 200, 340, PRIMARY_COLOR, BACKGROUND_COLOR); // Bouche

  // Compte à rebours 3...2...1 avec yield()
  tft.setTextColor(WARNING_COLOR, BACKGROUND_COLOR);
  for (int i = 3; i > 0; i--) {
    tft.fillRect(cx - 50, 280, 100, 40, BACKGROUND_COLOR); // Effacer
    tft.drawCentreString(String(i), cx, 285, 6);
    // ✅ RETIRÉ: beep(NOTE_C4, 100); - Seul l'audio MP3 joue

    // ✅ Remplacer delay(1000) par boucle + yield()
    unsigned long debut = millis();
    while (millis() - debut < 900) {
      yield();  // Laisser respirer le watchdog
      delay(50);
    }
  }

  // ✅ RETIRÉ: beep(NOTE_G5, 150); - Bip final supprimé pour audio MP3
}

// Affichage "Capture en cours" avec barre de progression NON-BLOQUANTE
void afficherCaptureEnCours() {
  tft.fillScreen(BACKGROUND_COLOR);
  dessinerHeader("CAPTURE IN PROGRESS");

  tft.setTextColor(WARNING_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("📸 CAPTURE EN COURS", SCREEN_W / 2, 100, 4);

  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("Stay still", SCREEN_W / 2, 160, 2);
  tft.drawCentreString("Don't move", SCREEN_W / 2, 190, 2);

  // Barre de progression (3 secondes = 100 * 30ms) avec yield()
  int barX = 90, barY = 240, barW = 300, barH = 20;
  tft.drawRect(barX, barY, barW, barH, TEXT_COLOR);

  for (int i = 0; i <= 100; i += 2) {
    int fillW = (barW - 2) * i / 100;
    tft.fillRect(barX + 1, barY + 1, fillW, barH - 2, PRIMARY_COLOR);

    // Afficher pourcentage
    tft.fillRect(SCREEN_W / 2 - 40, barY + 30, 80, 30, BACKGROUND_COLOR);
    tft.setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
    tft.drawCentreString(String(i) + "%", SCREEN_W / 2, barY + 35, 4);

    // ✅ Remplacer delay(30) par delay(30) + yield()
    yield();
    delay(30); // Total: 100 * 30ms = 3 secondes
  }

  // ✅ RETIRÉ: beep(NOTE_E5, 100); - Seul l'audio MP3 joue
}

// ============================================================
// ÉTAT BIENVENUE PERSONNALISÉ ✅ REFACTORISÉ
// ============================================================
void setupBienvenue() {
  // ÉTAPE 1 : Afficher "Bonjour M. DUPONT" (déjà avec animation)
  afficherEcranBienvenueEtudiant();

  // ÉTAPE 2 : Bip sonore puis audio 002 (YEUX) - puis attendre 7 secondes
  // "Identification réussie. Première étape : analyse oculaire..."
  beepMesureComplete();  // 🔔 Bip sonore de bienvenue
  delay(300);
  envoyerSignalAudio(AUDIO_YEUX);  // 📢 002.mp3 (7 secondes)

  // ⭐ ATTENDRE 7 SECONDES pour que l'utilisateur écoute l'audio
  Serial.println("[BIENVENUE] ⏳ Attente de 7 secondes pour l'audio 002...");
  unsigned long debut = millis();
  while (millis() - debut < 7000) {  // 7000ms = 7 secondes
    yield();  // Laisser respirer le watchdog
    delay(100);
  }
  Serial.println("[BIENVENUE] ✅ Audio 002 terminé - Passage au countdown");

  // ÉTAPE 3 : Afficher "RAPPROCHEZ VOTRE VISAGE" (3 secondes + countdown)
  afficherRapprochezVisage();

  // ÉTAPE 4 : Afficher "CAPTURE EN COURS" (3 secondes + barre)
  afficherCaptureEnCours();

  // ÉTAPE 5 : Lancer capture Raspberry
  if (wifiStat.connected && etudActuel.id > 0) {
    if (lancerCaptureOeuxRaspberry(etudActuel.id)) {
      // ✅ Succès - capture lancée
      Serial.println("[BIENVENUE] ✅ Capture Raspberry lancée");
      // captureOeuxEnCours et debutCaptureyeux déjà définis par lancerCaptureOeuxRaspberry()
    } else {
      // ❌ Échec - Raspberry inaccessible
      Serial.println("[BIENVENUE] ❌ Impossible de contacter la Raspberry");
      resultatOeux.disponibles = false;
      captureOeuxEnCours = false;

      tft.fillScreen(BACKGROUND_COLOR);
      dessinerHeader("NETWORK ERROR");
      tft.setTextColor(ERROR_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("Raspberry inaccessible", SCREEN_W / 2, 150, 3);
      tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("Rest of the examination", SCREEN_W / 2, 220, 2);
      beepError();
      delay(2000);
    }
  } else {
    // ❌ Pas de WiFi ou ID invalide
    Serial.println("[BIENVENUE] ⚠️  WiFi ou ID invalide");
    resultatOeux.disponibles = false;
    captureOeuxEnCours = false;
  }
}

void loopBienvenue() {
  // ⭐ SI capture lancée, faire UNIQUEMENT le polling
  if (captureOeuxEnCours) {
    unsigned long tempsEcoule = millis() - debutCaptureyeux;

    // Afficher statut toutes les secondes
    static unsigned long dernierAffichage = 0;
    if (millis() - dernierAffichage > 1000) {
      dernierAffichage = millis();

      int secondesEcoulees = tempsEcoule / 1000;
      tft.fillRect(0, 270, SCREEN_W, 40, BACKGROUND_COLOR);
      tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("Analyse en cours... " + String(secondesEcoulees) + "s",
                           SCREEN_W / 2, 280, 2);
    }

    // POLLING toutes les 2 secondes
    static unsigned long dernierPolling = 0;
    if (millis() - dernierPolling > 2000) {
      dernierPolling = millis();

      if (obtenirResultatsOeuxRaspberry(sessionRaspberryId)) {
        // ✅ Résultats reçus!
        Serial.println("[BIENVENUE] ✅ Résultats IA reçus");
        captureOeuxEnCours = false;

        // ❌ NE PAS AFFICHER LES RÉSULTATS ICI
        // Les résultats seront affichés dans setupAffichage() avec les autres mesures
        beepOK();
        delay(500); // Bref feedback
        changerEtat(RYTHME_CARDIAQUE);
        return;
      }
    }

    // Timeout après 30 secondes
    if (tempsEcoule > 30000) {
      Serial.println("[BIENVENUE] ❌ Timeout capture oculaire");
      captureOeuxEnCours = false;
      resultatOeux.disponibles = false;

      tft.fillScreen(BACKGROUND_COLOR);
      dessinerHeader("TIMEOUT");
      tft.setTextColor(ERROR_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("Analysis impossible", SCREEN_W / 2, 150, 3);
      tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("Rest of the examination...", SCREEN_W / 2, 220, 2);

      beepError();
      delay(2000);
      changerEtat(RYTHME_CARDIAQUE);
    }
  } else {
    // ⭐ Si pas de capture lancée (erreur initiale), passer directement
    if (millis() - tempsDebutEtat > 1000) {
      Serial.println("[BIENVENUE] Capture non lancée - Suite examen");
      changerEtat(RYTHME_CARDIAQUE);
    }
  }
}

// ============================================================
// ÉTAT RYTHME CARDIAQUE - VERSION AMÉLIORÉE ✅
// ============================================================
// ================================================
// 🫀 ALGORITHME BEAT FINDER PRO - IMPLÉMENTATION
// ================================================

// Filtre Kalman pour lissage BPM temps réel
float filtreKalmanBPM(float bpmMesu) {
  // Prédiction
  float prediction = kalmanState;
  float covariancePred = kalmanCovariance + KALMAN_PROCESS_NOISE;
  
  // Correction (Kalman gain)
  float kalmanGain = covariancePred / (covariancePred + KALMAN_MEASUREMENT_NOISE);
  
  // Mise à jour état
  kalmanState = prediction + kalmanGain * (bpmMesu - prediction);
  
  // Mise à jour covariance
  kalmanCovariance = (1.0 - kalmanGain) * covariancePred;
  
  return kalmanState;
}

// Validation d'un beat selon pattern physiologique
bool estBeatValide(long timeSinceLast) {
  // Validation physiologie: 40-300 BPM = 200-1500ms entre beats
  if (timeSinceLast < BEAT_PATTERN_MIN || timeSinceLast > BEAT_PATTERN_MAX) {
    beatPatternCount = 0;  // Reset pattern
    return false;
  }
  
  beatPatternCount++;
  return true;
}

// Tolerance adaptatif basé variance - PLUS STRICT
float calculerToleranceAdaptative() {
  // Commence strict, se relâche si variance monte (faux positifs)
  // L'idée: forcer la stabilisation au lieu d'accepter n'importe quoi
  
  if (bpmVariance < 2.0) {
    return 2.0;  // ±1-2 BPM - ultra stable (très bon signal)
  } else if (bpmVariance < 4.0) {
    return 3.0;  // ±3 BPM - très stable
  } else if (bpmVariance < 8.0) {
    return 5.0;  // ±5 BPM - stable
  } else if (bpmVariance < 15.0) {
    return 8.0;  // ±8 BPM - apprentissage
  } else {
    return 12.0;  // ±12 BPM - max (signal très bruité)
  }
}

// Réinitialiser algorithme BPM
void reinitialiserBPMAlgorithme() {
  for(int i = 0; i < BPM_BUFFER_SIZE; i++) {
    bpmBuffer[i] = 0;
  }
  bpmBufferIndex = 0;
  kalmanState = 0;
  kalmanCovariance = 1.0;
  lastBeatTime = 0;
  bpmVariance = 0;
  beatPatternCount = 0;
  adaptativeTolerance = 10.0;
  detectionStartTime = 0;
  
  Serial.println("[BPM] Algorithme réinitialisé");
}

void setupRythmeCardiaque() {
  activerCapteurCardiaque();

  tft.fillScreen(BACKGROUND_COLOR);
  dessinerHeader("HEART RATE MEASUREMENT");

  // Zone BPM - Gauche
  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("HEARTBEATS", 120, 50, 2);
  tft.drawCentreString("PER MINUTE", 120, 70, 2);

  // Zone SpO2 - Droite
  tft.drawCentreString("OXYGENATION", 360, 50, 2);
  tft.drawCentreString("OF THE BLOOD", 360, 70, 2);

  afficherBPM(0);
  afficherSpO2(0);
  afficherStatut("Place your finger on the sensor", WARNING_COLOR);

  for(byte i = 0; i < RATE_SIZE; i++) rates[i] = 0;
  rateSpot = 0;
  lastBeat = 0;
  donnees.bpm = 0;
  donnees.spo2 = 0;
  etatCapteurs.doigtPresent = false;
  compteurMesuresStables = 0;
  dernierBPMValide = 0;
  mesureEnCours = false;
  debutMesureStable = 0;
  reinitialiserBPMAlgorithme();  // Initialiser Beat Finder Pro
}

void loopRythmeCardiaque() {
  if (!etatCapteurs.max30102Actif) {
    afficherStatut("Heart rate sensor unavailable", ERROR_COLOR);
    donnees.bpmStable = 0;
    donnees.spo2Stable = 0;
    delay(2000);
    desactiverCapteurCardiaque();
    changerEtat(TEMPERATURE);
    return;
  }

  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();

  if (millis() - dernierAffichageIR > 1000) {
    Serial.println("[HR] IR: " + String(irValue) + ", RED: " + String(redValue));
    afficherValeurIR(irValue);
    dernierAffichageIR = millis();
  }

  if (irValue > config.seuilIrDoigt) {
    // ✅ DOIGT DÉTECTÉ
    if (!etatCapteurs.doigtPresent) {
      etatCapteurs.doigtPresent = true;
      afficherStatut("Finger detected - Fast analysis (0.5-1s)...", SUCCESS_COLOR);
      lastBeatTime = millis();
      debutMesureStable = millis();
      detectionStartTime = millis();
      mesureEnCours = true;
      reinitialiserBPMAlgorithme();
      Serial.println("[BPM] 🟢 Doigt détecté - Démarrage détection rapide");
    }

    float qualiteSignal = min(100.0, (float)(irValue - config.seuilIrDoigt) / 30000.0 * 100.0);

    // 🫀 BEAT FINDER PRO - Détection beat avec pattern
    if (checkForBeat(irValue) == true) {
      long timeSinceLast = millis() - lastBeatTime;
      lastBeatTime = millis();

      // Calculer BPM brut
      int rawBPM = 60000 / timeSinceLast;
      
      // Valider selon pattern physiologique
      if (estBeatValide(timeSinceLast)) {
        // ✅ Beat valide
        
        // Appliquer filtre Kalman
        float filteredBPM = filtreKalmanBPM(rawBPM);
        
        // Ajouter au buffer circulaire
        bpmBuffer[bpmBufferIndex] = filteredBPM;
        bpmBufferIndex = (bpmBufferIndex + 1) % BPM_BUFFER_SIZE;
        
        // Calculer variance buffer
        float sum = 0, sumSq = 0;
        int validCount = 0;
        for (int i = 0; i < BPM_BUFFER_SIZE; i++) {
          if (bpmBuffer[i] > 0) {
            sum += bpmBuffer[i];
            sumSq += bpmBuffer[i] * bpmBuffer[i];
            validCount++;
          }
        }
        
        if (validCount > 0) {
          float mean = sum / validCount;
          bpmVariance = (sumSq / validCount) - (mean * mean);
          donnees.bpm = (int)mean;
          
          // Mettre à jour tolerance adaptatif
          adaptativeTolerance = calculerToleranceAdaptative();
        }
        
        // Calculer SpO2
        calculerSpO2(irValue, redValue);
        
        // 🔒 GESTION STABILITÉ - STRICT
        if (dernierBPMValide == 0) {
          dernierBPMValide = donnees.bpm;
          compteurMesuresStables = 1;
        } else {
          // TRÈS STRICT: doit être dans tolérance ET variance acceptée
          if (abs(donnees.bpm - dernierBPMValide) <= adaptativeTolerance && 
              bpmVariance <= 25.0) {  // Variance max 25 pour compter comme stable
            compteurMesuresStables++;
            Serial.println("[BPM] ✓ Stable #" + String(compteurMesuresStables) + " (delta:" + 
                          String(abs(donnees.bpm - dernierBPMValide)) + " Var:" + String((int)bpmVariance) + ")");
          } else {
            if (compteurMesuresStables > 0) {
              Serial.println("[BPM] ✗ Instable - Reset (delta:" + String(abs(donnees.bpm - dernierBPMValide)) + 
                            " Var:" + String((int)bpmVariance) + ")");
            }
            compteurMesuresStables = max(0, compteurMesuresStables - 1);
          }
          dernierBPMValide = donnees.bpm;
        }
        
        // Debug
        Serial.println("[BPM] Raw:" + String(rawBPM) + " Filt:" + String((int)filteredBPM) + 
                      " Moy:" + String(donnees.bpm) + " Tol:" + String((int)adaptativeTolerance) + 
                      " Var:" + String((int)bpmVariance));
        
        // 🚀 DÉTECTION RAPIDE SÉCURISÉE - Forcer stabilisation réelle
        unsigned long tempsMesure = millis() - debutMesureStable;
        
        // ⚠️ CRITÈRES ÉQUILIBRÉS pour détection stable + réactive:
        // Mode rapide: MINIMUM 2s + 4 beats stables + variance < 25 BPM²
        // Mode standard: 3.5s + 2 mesures stables + variance < 9 BPM²
        
        bool fastModeValid = (tempsMesure >= 2000 && beatPatternCount >= 4 && bpmVariance < 25.0) &&
                            (compteurMesuresStables >= 2);
        
        bool standardModeValid = (tempsMesure >= TEMPS_MIN_MESURE_BPM && 
                                compteurMesuresStables >= NOMBRE_MESURES_STABLES_REQUIS &&
                                bpmVariance < 9.0);
        
        if (fastModeValid || standardModeValid) {
          donnees.bpmStable = donnees.bpm;
          donnees.spo2Stable = donnees.spo2;
          donnees.donneesValides = true;

          long timeToDetection = millis() - detectionStartTime;
          Serial.println("[BPM] ✅ DÉTECTÉ EN " + String(timeToDetection) + "ms - BPM:" + 
                        String(donnees.bpmStable) + " SpO2:" + String(donnees.spo2Stable) + "% Var:" + String((int)bpmVariance));
          
          afficherStatut("✓ Measurements complete (" + String(timeToDetection) + "ms)", SUCCESS_COLOR);
          beepOK();
          delay(800);
          desactiverCapteurCardiaque();
          changerEtat(TEMPERATURE);
          return;
        }
      } else {
        // ❌ Beat rejeté (hors pattern physiologique)
        Serial.println("[BPM] ❌ Beat rejeté (pattern invalide): delta=" + String(timeSinceLast) + "ms");
      }
    }

    // Affichage mis à jour tous les 500ms
    if (millis() - dernierUpdateAffichage > 500) {
      afficherBPM(donnees.bpm);
      afficherSpO2(donnees.spo2);
      afficherQualiteSignal(qualiteSignal);
      
      long elapsed = millis() - detectionStartTime;
      String status = "BPM:" + String(donnees.bpm) + " | Stable:" + String(compteurMesuresStables) + 
                     "/5 | Pat:" + String(beatPatternCount) + "/4 | Var:" + String((int)bpmVariance);
      afficherStatut(status, PRIMARY_COLOR);
      
      afficherProgressionStabilite(compteurMesuresStables, 5);
      dernierUpdateAffichage = millis();
    }
  } else {
    // ❌ DOIGT RETIRÉ
    if (etatCapteurs.doigtPresent) {
      etatCapteurs.doigtPresent = false;
      donnees.bpm = 0;
      donnees.spo2 = 0;
      compteurMesuresStables = 0;
      beatPatternCount = 0;
      mesureEnCours = false;
      afficherStatut("Finger removed - Replace finger", WARNING_COLOR);
      afficherBPM(0);
      afficherSpO2(0);
      afficherQualiteSignal(0);
      Serial.println("[BPM] 🔴 Doigt retiré");
    }
  }
}

// ============================================================
// ÉTAT TEMPÉRATURE - VERSION AMÉLIORÉE ✅
// ============================================================
void setupTemperature() {
  activerCapteurTemperature();

  tft.fillScreen(BACKGROUND_COLOR);
  dessinerHeader("TEMPERATURE MEASUREMENT");

  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("BODY TEMPERATURE", SCREEN_W / 2, 100, 2);

  afficherTemperature(0.0);
  afficherStatut("Bring your forehead close to the sensor", PRIMARY_COLOR);

  for (int i = 0; i < 5; i++) {
    historiqueTemperature[i] = 0;
  }
  indexHistoriqueTemperature = 0;
  stabilisationTempEnCours = false;
  debutStabilisationTemp = 0;
}

void loopTemperature() {
  if (!etatCapteurs.mlx90614Actif) {
    afficherStatut("Temperature sensor unavailable", ERROR_COLOR);
    donnees.temperatureStable = 0;  // Marquer comme non disponible
    delay(2000);
    desactiverCapteurTemperature();
    changerEtat(POIDS);  // Continuer vers la mesure suivante
    return;
  }

  float temperatureBrute = lireTemperatureFiltree();

  if (temperatureBrute > 25) {
    donnees.temperature = temperatureBrute;

    bool stable = verifierStabiliteTemperature(donnees.temperature);

    afficherTemperature(donnees.temperature);

    if (stable) {
      donnees.temperatureStable = donnees.temperature;
      afficherStatut("✓ Temperature stable !", SUCCESS_COLOR);
      beepOK();  // ✅ BIP pour indiquer que la mesure est prise
      delay(500);
      desactiverCapteurTemperature();
      changerEtat(POIDS);
      return;
    } else {
      if (!stabilisationTempEnCours) {
        afficherStatut("Waiting for stabilization...", WARNING_COLOR);
      } else {
        int msRestantes = 1500 - (millis() - debutStabilisationTemp);
        afficherStatut("Stable... " + String(msRestantes/100.0, 1) + "s", SUCCESS_COLOR);
      }
    }
  } else {
    afficherStatut("Bring your forehead close to the sensor", WARNING_COLOR);
  }

  delay(100);
}

// ============================================================
// ÉTAT POIDS - VERSION AMÉLIORÉE ✅
// ================================================
// 🏋️ ALGORITHME SMART WEIGHT OPTIMIZER - IMPLÉMENTATION
// ================================================

// Filtre Kalman pour lissage poids temps réel
float filtreKalmanPoids(float poidsMesu) {
  // Prédiction
  float prediction = kalmanWeightState;
  float covariancePred = kalmanWeightCovariance + KALMAN_WEIGHT_PROCESS;
  
  // Correction (Kalman gain)
  float kalmanGain = covariancePred / (covariancePred + KALMAN_WEIGHT_MEASUREMENT);
  
  // Mise à jour état
  kalmanWeightState = prediction + kalmanGain * (poidsMesu - prediction);
  
  // Mise à jour covariance
  kalmanWeightCovariance = (1.0 - kalmanGain) * covariancePred;
  
  return kalmanWeightState;
}

// Détection de mouvement patient
bool detecterMouvement(float poidsActuel) {
  if (lastWeightValue == 0) {
    lastWeightValue = poidsActuel;
    return false;  // Première lecture
  }
  
  float delta = abs(poidsActuel - lastWeightValue);
  lastWeightValue = poidsActuel;
  
  if (delta > MOVEMENT_DETECTION_THRESHOLD) {
    movementDetected = true;
    stableReadingCount = 0;  // Reset stabilité
    return true;
  }
  
  movementDetected = false;
  return false;
}

// Calcul confiance mesure (0-100%)
float calculerConfiance() {
  float conf = 0;
  
  // Pas de mouvement: +50%
  if (!movementDetected) {
    conf += 50.0;
  } else {
    conf += 10.0;
  }
  
  // Variance basse: +30%
  if (weightVariance < 0.05) {
    conf += 30.0;
  } else if (weightVariance < 0.1) {
    conf += 20.0;
  } else if (weightVariance < 0.2) {
    conf += 10.0;
  }
  
  // Pattern valide: +20%
  if (stableReadingCount >= 3) {
    conf += 20.0;
  } else if (stableReadingCount >= 1) {
    conf += 10.0;
  }
  
  return min(100.0f, conf);
}

// Réinitialiser algorithme poids
void reinitialiserAlgorithmePoids() {
  for(int i = 0; i < WEIGHT_VARIANCE_BUFFER_SIZE; i++) {
    weightBuffer[i] = 0;
  }
  weightBufferIndex = 0;
  kalmanWeightState = 0;
  kalmanWeightCovariance = 1.0;
  lastWeightReading = 0;
  weightVariance = 0;
  lastWeightValue = 0;
  movementDetected = false;
  measurementConfidence = 0;
  stableReadingCount = 0;
  weightDetectionStartTime = 0;
  
  Serial.println("[POIDS] Algorithme réinitialisé");
}

// ============================================================
// ÉTAT POIDS - VERSION AMÉLIORÉE ✅
// ============================================================
void setupPoids() {
  activerCapteurPoids();

  // ✅ Réinitialisation complète
  for (int i = 0; i < NOMBRE_LECTURES_STABILITE; i++) {
    historiquePoids[i] = 0;
  }
  indexHistoriquePoids = 0;
  stabilisationEnCours = false;
  debutStabilisation = 0;
  reinitialiserAlgorithmePoids();  // Initialiser Smart Weight Optimizer
  
  // 🎯 BUG FIX: Réinitialiser la donnée de poids avant nouvelle mesure
  donnees.poids = 0;
  weightDetectionStartTime = millis();  // Reset timer détection rapide

  // ✅ Faire quelques lectures initiales pour "amorcer" le filtre
  Serial.println("[POIDS] Amorcage du filtre...");
  for (int i = 0; i < 5; i++) {
    lirePoidsFiltre();
    delay(200);
  }

  tft.fillScreen(BACKGROUND_COLOR);
  dessinerHeader("WEIGHT MEASUREMENT");

  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("BODY WEIGHT", SCREEN_W / 2, 100, 2);

  afficherPoids(0.0);
  afficherStatut("Get on the scale and stay still", PRIMARY_COLOR);

  Serial.println("[POIDS] ✅ Setup terminé - En attente de mesure");
}

void loopPoids() {
  if (!etatCapteurs.hx711Actif) {
    afficherStatut("Weight sensor unavailable", ERROR_COLOR);
    donnees.poids = 0;
    delay(2000);
    desactiverCapteurPoids();
    changerEtat(AFFICHAGE);
    return;
  }

  float poidsBrut = lirePoidsFiltre();

  if (etatCapteurs.nouvelleLecturePoids) {
    etatCapteurs.nouvelleLecturePoids = false;

    // 🏋️ SMART WEIGHT OPTIMIZER - Détection et filtrage
    
    // 1. Détecter mouvement
    bool movement = detecterMouvement(poidsBrut);
    
    if (movement) {
      Serial.println("[POIDS] ⚠️ Mouvement détecté (delta: " + String(abs(poidsBrut - lastWeightValue), 2) + "kg)");
      stableReadingCount = 0;
      afficherStatut("Please stay still on the scale!", WARNING_COLOR);
    }
    
    // 2. Appliquer filtre Kalman
    float filteredWeight = filtreKalmanPoids(poidsBrut);
    
    // 3. Ajouter au buffer circulaire
    weightBuffer[weightBufferIndex] = filteredWeight;
    weightBufferIndex = (weightBufferIndex + 1) % WEIGHT_VARIANCE_BUFFER_SIZE;
    
    // 4. Calculer variance
    float sum = 0, sumSq = 0;
    int validCount = 0;
    for (int i = 0; i < WEIGHT_VARIANCE_BUFFER_SIZE; i++) {
      if (weightBuffer[i] > 0.1) {
        sum += weightBuffer[i];
        sumSq += weightBuffer[i] * weightBuffer[i];
        validCount++;
      }
    }
    
    if (validCount > 0) {
      float mean = sum / validCount;
      weightVariance = (sumSq / validCount) - (mean * mean);
      donnees.poids = mean;
    }
    
    // 5. Calculer confiance
    measurementConfidence = calculerConfiance();
    
    // 6. Vérifier stabilité
    bool stable = verifierStabilitePoids(donnees.poids);
    
    afficherPoids(donnees.poids);
    
    if (!movement && stable) {
      stableReadingCount++;
    } else {
      stableReadingCount = max(0, stableReadingCount - 1);
    }
    
    // Debug
    Serial.println("[POIDS] Raw:" + String(poidsBrut, 2) + "kg Filt:" + String(filteredWeight, 2) + 
                  "kg Var:" + String(weightVariance, 4) + " Conf:" + String((int)measurementConfidence) + "% Mov:" + (movement ? "YES" : "NO"));
    
    // 🚀 DÉTECTION RAPIDE 1-2s
    unsigned long tempsMesure = millis() - weightDetectionStartTime;
    
    // Mode rapide: 1s + 3 lectures stables OU 2s + 2 lectures + pas mouvement
    bool fastModeValid = (tempsMesure >= WEIGHT_FAST_DETECTION_TIME && stableReadingCount >= 3 && !movement) ||
                        (tempsMesure >= 2000 && stableReadingCount >= 2 && !movement && measurementConfidence >= 80.0);
    
    // Mode standard: ancien système (fallback)
    bool standardModeValid = stable && measurementConfidence >= 70.0;
    
    if (fastModeValid || standardModeValid) {
      long timeToDetection = millis() - weightDetectionStartTime;
      Serial.println("[POIDS] ✅ DÉTECTÉ EN " + String(timeToDetection) + "ms - Poids:" + 
                    String(donnees.poids, 2) + "kg Conf:" + String((int)measurementConfidence) + "%");
      
      afficherStatut("✓ Weight: " + String(donnees.poids, 1) + "kg (" + String((int)measurementConfidence) + "%)", SUCCESS_COLOR);
      beepMesureComplete();
      delay(1000);
      desactiverCapteurPoids();
      changerEtat(AFFICHAGE);
      return;
    } else {
      // Affichage progressif
      if (donnees.poids < config.seuilPoidsDetection) {
        afficherStatut("Get on the scale (" + String(donnees.poids, 1) + " kg)", WARNING_COLOR);
      } else if (!movement) {
        String statusText = "Stable: " + String(donnees.poids, 1) + "kg | " + 
                           String(stableReadingCount) + "/3 | Conf:" + String((int)measurementConfidence) + "%";
        afficherStatut(statusText, PRIMARY_COLOR);
      } else {
        afficherStatut("Please stay still... (" + String(donnees.poids, 1) + " kg)", SECONDARY_COLOR);
      }
    }
  }

  delay(50);
}

// ============================================================
// ÉTAT AFFICHAGE RÉSULTATS - VERSION AMÉLIORÉE ✅
// ============================================================
void setupAffichage() {
  desactiverTousCapteurs();

  donnees.timestamp = millis();
  bool sent = false;

  // ⭐⭐ AJOUTER DU DÉBOGAGE CRITIQUE
  Serial.println("[AFFICHAGE] === TENTATIVE ENVOI MESURES ===");
  Serial.println("[AFFICHAGE] ID Étudiant: " + String(etudActuel.id));
  Serial.println("[AFFICHAGE] Nom: " + String(etudActuel.prenom) + " " + String(etudActuel.nom));
  Serial.println("[AFFICHAGE] BPM: " + String(donnees.bpmStable));
  Serial.println("[AFFICHAGE] SpO2: " + String(donnees.spo2Stable));
  Serial.println("[AFFICHAGE] Temp: " + String(donnees.temperatureStable, 1));
  Serial.println("[AFFICHAGE] Poids: " + String(donnees.poids, 1));

  if (wifiStat.connected && etudActuel.id > 0) {
    sent = envoyerMesuresServeur(
      etudActuel.uid, etudActuel.nom, etudActuel.prenom, etudActuel.classe,
      donnees.bpmStable, donnees.spo2Stable, donnees.temperatureStable, donnees.poids,
      resultatOeux.oeil_gauche, resultatOeux.oeil_gauche_conf,
      resultatOeux.oeil_droit, resultatOeux.oeil_droit_conf,
      resultatOeux.alerte);
  } else {
    if (etudActuel.id <= 0) {
      Serial.println("[AFFICHAGE] ❌ ID étudiant invalide, ajout à la file d'attente");
    }
    if (!wifiStat.connected) {
      Serial.println("[AFFICHAGE] ⚠️ WiFi déconnecté, ajout à la file d'attente");
    }
  }

  if (!sent) {
    Serial.println("[AFFICHAGE] Ajout des mesures à la file d'attente");
    ajouterQueue(
      etudActuel.uid, etudActuel.nom, etudActuel.prenom, etudActuel.classe,
      donnees.bpmStable, donnees.spo2Stable, donnees.temperatureStable, donnees.poids);
  }

  tft.fillScreen(BACKGROUND_COLOR);
  dessinerHeader("MEASUREMENT SUMMARY");

  // Cadre principal
  tft.drawRoundRect(20, 60, SCREEN_W - 40, 200, 15, PRIMARY_COLOR);

  int yStart = 80;
  int ligneHauteur = 45;

  // Ligne 1: BPM et SpO2
  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawString("Rythme cardiaque:", 40, yStart, 2);
  tft.drawString("Saturation O2:", 260, yStart, 2);

  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  String bpmText = (donnees.bpmStable > 0) ? String(donnees.bpmStable) + " BPM" : "Non mesure";
  tft.drawString(bpmText, 40, yStart + 25, 4);

  String spo2Text = (donnees.spo2Stable > 0) ? String(donnees.spo2Stable) + " %" : "Non mesure";
  tft.drawString(spo2Text, 260, yStart + 25, 4);

  // Ligne 2: Température
  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawString("Temperature:", 40, yStart + ligneHauteur, 2);

  uint16_t tempColor = TEMP_NORMAL_COLOR;
  String tempText = "Non mesuree";
  if (donnees.temperatureStable > 0) {
    tempText = String(donnees.temperatureStable, 1) + " °C";
    if (donnees.temperatureStable >= TEMPERATURE_FIEVRE) tempColor = TEMP_HIGH_COLOR;
    else if (donnees.temperatureStable >= TEMPERATURE_FIEVRE_LEGERE) tempColor = TEMP_WARNING_COLOR;
    else tempColor = TEMP_NORMAL_COLOR;
  }
  tft.setTextColor(tempColor, BACKGROUND_COLOR);
  tft.drawString(tempText, 40, yStart + ligneHauteur + 25, 4);

  // Ligne 3: Poids
  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawString("Body weight:", 40, yStart + 2*ligneHauteur, 2);

  tft.setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
  String poidsText = (donnees.poids > 0) ? String(donnees.poids, 1) + " kg" : "Not measured";
  tft.drawString(poidsText, 40, yStart + 2*ligneHauteur + 25, 4);

  // ⭐⭐ AFFICHAGE RÉSULTATS OCULAIRES - SIMPLIFIÉ (une seule fois)
  int yEyes = yStart + 3*ligneHauteur;
  tft.drawLine(20, yEyes - 10, SCREEN_W - 20, yEyes - 10, SECONDARY_COLOR);

  tft.setTextColor(SECONDARY_COLOR, BACKGROUND_COLOR);
  tft.drawString("👁️ Yeux:", 30, yEyes, 2);

  if (resultatOeux.disponibles) {
    // Affichage compact: G: Sain (90%)  |  D: Cataracte (85%)
    uint16_t colorOG = (String(resultatOeux.oeil_gauche) == "Sain") ? SUCCESS_COLOR : ERROR_COLOR;
    uint16_t colorOD = (String(resultatOeux.oeil_droit) == "Sain") ? SUCCESS_COLOR : ERROR_COLOR;

    tft.setTextColor(colorOG, BACKGROUND_COLOR);
    tft.drawString("G: " + String(resultatOeux.oeil_gauche), 30, yEyes + 25, 2);
    tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    tft.drawString(String(resultatOeux.oeil_gauche_conf * 100, 0) + "%", 30, yEyes + 40, 1);

    tft.setTextColor(colorOD, BACKGROUND_COLOR);
    tft.drawString("D: " + String(resultatOeux.oeil_droit), 220, yEyes + 25, 2);
    tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
    tft.drawString(String(resultatOeux.oeil_droit_conf * 100, 0) + "%", 220, yEyes + 40, 1);
  } else {
    tft.setTextColor(WARNING_COLOR, BACKGROUND_COLOR);
    tft.drawString("Not available", 30, yEyes + 25, 2);
  }

  // ⭐⭐ ALERTE VISUELLE SI ANOMALIE DÉTECTÉE
  if (resultatOeux.disponibles && resultatOeux.alerte == 1) {
    // Affichage alerte en rouge sur toute la largeur
    tft.fillRect(0, 280, SCREEN_W, 35, ERROR_COLOR);
    tft.setTextColor(TEXT_COLOR, ERROR_COLOR);
    tft.drawCentreString("⚠️ ANOMALIE DÉTECTÉE", SCREEN_W / 2, 290, 2);

    // Bips d'alerte (triple bip)
    beep(NOTE_C5, 100);
    delay(100);
    beep(NOTE_C5, 100);
    delay(100);
    beep(NOTE_C5, 100);
  }

  // Statut envoi
  tft.setTextColor(sent ? SUCCESS_COLOR : WARNING_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString(sent ? "✓ Data sent to server" : "⚠️ Data awaiting synchronization", SCREEN_W / 2, 320, 1);

  Serial.println("\n=== SYNTHESE DES MESURES ===");
  Serial.println("Étudiant: " + String(etudActuel.prenom) + " " + String(etudActuel.nom));
  Serial.println("Classe: " + String(etudActuel.classe));
  Serial.println("BPM: " + String(donnees.bpmStable));
  Serial.println("SpO2: " + String(donnees.spo2Stable) + "%");
  Serial.println("Température: " + String(donnees.temperatureStable, 1) + "°C");
  Serial.println("Poids: " + String(donnees.poids, 1) + "kg");
  Serial.println("Oeil Gauche: " + String(resultatOeux.oeil_gauche) + " (" + String(resultatOeux.oeil_gauche_conf, 2) + ")");
  Serial.println("Oeil Droit: " + String(resultatOeux.oeil_droit) + " (" + String(resultatOeux.oeil_droit_conf, 2) + ")");
  Serial.println("Alerte: " + String(resultatOeux.alerte ? "OUI" : "NON"));
  Serial.println("Envoi: " + String(sent ? "Réussi" : "En attente"));
  Serial.println("=============================\n");
}

void loopAffichage() {
  unsigned long tempsEcoule = millis() - tempsDebutEtat;
  int secondesRestantes = (DELAI_AFFICHAGE - tempsEcoule) / 1000 + 1;
  if(secondesRestantes < 0) secondesRestantes = 0;

  clear(0, SCREEN_H - 25, SCREEN_W, 25);
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("New measurement in " + String(secondesRestantes) + " seconds", SCREEN_W / 2, SCREEN_H - 20, 1);

  if (tempsEcoule > DELAI_AFFICHAGE) {
    beep(NOTE_G4, 100);
    changerEtat(BADGE);
  }
}

// ============================================================
// COMMANDES SÉRIE
// ============================================================
void handleSerial() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n' || c == '\r') {
      if (inBuf.length() > 0) {
        processCmd(inBuf);
        inBuf = "";
      }
    } else inBuf += c;
  }
}

void processCmd(String cmd) {
  cmd.trim();
  String originalCmd = cmd;
  cmd.toLowerCase();

  if (cmd.startsWith("badge ")) {
    String uid = originalCmd.substring(6);
    uid.trim();
    if (uid.length() > 0) {
      Serial.println("Simuler badge: " + uid);

      // Réinitialiser etudActuel
      memset(&etudActuel, 0, sizeof(Etudiant));

      char buf[20];
      strncpy(buf, uid.c_str(), 19);
      buf[19] = 0;

      if (findEtud(buf, &etudActuel)) {
        Serial.println("✅ Étudiant trouvé: " + String(etudActuel.prenom) + " " + String(etudActuel.nom));
        Serial.println("📋 ID: " + String(etudActuel.id));
        changerEtat(BIENVENUE);
      } else {
        Serial.println("❌ Étudiant inconnu pour UID: " + uid);
      }
      return;
    }
  }

  // ⭐⭐ SUPPRIMER les commandes liées au cache
  if (cmd == "liste" || cmd == "l") {
    Serial.println("❌ Commande désactivée - Mode sans cache EEPROM");
    return;
  }

  if (cmd == "ajouter" || cmd == "add") {
    Serial.println("❌ Commande désactivée - Mode sans cache EEPROM");
    return;
  }

  if (cmd.startsWith("sup ")) {
    Serial.println("❌ Commande désactivée - Mode sans cache EEPROM");
    return;
  }

  if (cmd == "reset") {
    Serial.println("❌ Commande désactivée - Mode sans cache EEPROM");
    return;
  }

  // Garder les autres commandes utiles
  if (cmd == "wifi") {
    Serial.println("\n=== CONFIG WIFI ===");
    Serial.print("SSID: ");
    modeWiFiConfig = true;
    etapeWiFi = 1;
  }
  else if (cmd == "queue") {
    Serial.println("\n=== QUEUE (" + String(queueSize) + ") ===");
    for (int i = 0; i < queueSize; i++) {
      Serial.println(String(i + 1) + ". " + queue[i].prenom + " " + queue[i].nom);
    }
  }
  else if (cmd == "sync") {
    Serial.println("[SYNC] Force synchronisation...");
    traiterQueue();
  }
  else if (cmd == "poids" || cmd == "p") {
    Serial.println("\n=== TEST POIDS ===");

    activerCapteurPoids();

    Serial.println("Lecture de 10 échantillons...");
    for (int i = 0; i < 10; i++) {
      if (scale.update()) {
        float poids = scale.getData();
        Serial.println("Lecture " + String(i+1) + ": " + String(poids, 3) + " kg");
      }
      delay(500);
    }

    desactiverCapteurPoids();
    Serial.println("==================\n");
  }

  // ✅ NOUVELLE COMMANDE CALIBRATION
  else if (cmd.startsWith("cal ")) {
    String valStr = cmd.substring(4);
    float newCal = valStr.toFloat();

    if (newCal > 0) {
      config.calFactor = newCal;
      Serial.println("[CAL] Nouveau calFactor: " + String(config.calFactor));

      if (etatCapteurs.hx711Actif) {
        scale.setCalFactor(config.calFactor);
        Serial.println("[CAL] ✅ Appliqué au capteur");
      }
    } else {
      Serial.println("[CAL] ❌ Valeur invalide");
    }
  }

  // ✅ NOUVELLE COMMANDE TARE
  else if (cmd == "tare" || cmd == "t") {
    Serial.println("[TARE] Remise à zéro de la balance...");

    if (!etatCapteurs.hx711Actif) {
      activerCapteurPoids();
      delay(500);
    }

    scale.tareNoDelay();

    unsigned long start = millis();
    while (!scale.getTareStatus() && millis() - start < 5000) {
      scale.update();
      delay(10);
    }

    if (scale.getTareStatus()) {
      Serial.println("[TARE] ✅ Tare effectuée");
      beepOK();
    } else {
      Serial.println("[TARE] ❌ Échec tare");
      beepError();
    }
  }
  else if (cmd == "etat" || cmd == "e") {
    Serial.println("\n=== ÉTAT ===");
    Serial.println("WiFi: " + String(wifiStat.connected ? "OK" : "OFF"));
    Serial.println("Queue: " + String(queueSize));
    Serial.println("Heap: " + String(ESP.getFreeHeap()));
    Serial.println("============\n");
  }
  else if (cmd == "help" || cmd == "h") {
    Serial.println("\n=== COMMANDES ===");
    Serial.println("badge UID : Simuler badge");
    Serial.println("wifi      : Config WiFi");
    Serial.println("queue     : Voir queue");
    Serial.println("sync      : Force sync");
    Serial.println("etat      : État système");
    Serial.println("=================\n");
  }
  else Serial.println("'help' pour les commandes");
}

// ============================================================
// 🔥 ÉCRAN DÉMARRAGE - ADAPTÉ POUR 480x320 ✅
// ============================================================
void bootScreen() {
  Serial.println("\n[BOOT] ========================================");
  Serial.println("[BOOT] Affichage écran de démarrage 480x320");
  Serial.println("[BOOT] ========================================");

  if (TFT_BL >= 0) {
    digitalWrite(TFT_BL, TFT_BL_ON);
    Serial.println("[BOOT] ✅ Rétroéclairage vérifié");
  }

  tft.fillScreen(BACKGROUND_COLOR);
  delay(100);

  // Cadre décoratif
  tft.drawRoundRect(15, 15, SCREEN_W - 30, SCREEN_H - 30, 25, PRIMARY_COLOR);
  tft.drawRoundRect(17, 17, SCREEN_W - 34, SCREEN_H - 34, 23, HIGHLIGHT_COLOR);

  tft.setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("ROBOT MEDICAL", SCREEN_W / 2, 70, 6);
  tft.drawCentreString("SCOLAIRE", SCREEN_W / 2, 130, 6);
  Serial.println("[BOOT] Titre affiché");

  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("Intelligent biometric system", SCREEN_W / 2, 180, 2);
  tft.drawCentreString("v5.3 - ESP32 - 480x320", SCREEN_W / 2, 210, 1);
  Serial.println("[BOOT] Version affichée");

  delay(800);

  int bX = 90, bY = 250, bW = 300, bH = 12;
  tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("System initialization...", SCREEN_W / 2, bY - 25, 1);
  tft.drawRect(bX, bY, bW, bH, TEXT_COLOR);
  Serial.println("[BOOT] Barre de progression créée");

  for (int i = 0; i <= 100; i += 5) {
    int fillW = (bW - 2) * i / 100;
    tft.fillRect(bX + 1, bY + 1, fillW, bH - 2, PRIMARY_COLOR);

    if (i % 25 == 0 && i > 0) {
      beep(NOTE_C4 + (i / 25) * 100, 60);
    }

    delay(35);
  }

  Serial.println("[BOOT] Animation terminée");

  tft.setTextColor(SUCCESS_COLOR, BACKGROUND_COLOR);
  tft.drawCentreString("SYSTEM READY", SCREEN_W / 2, 290, 4);

  beep(NOTE_C5, 100);
  delay(50);
  beep(NOTE_E5, 100);
  delay(50);
  beep(NOTE_G5, 150);

  Serial.println("[BOOT] ✅ Écran de démarrage terminé");
  delay(800);

  tft.fillScreen(BACKGROUND_COLOR);
  Serial.println("[BOOT] Écran nettoyé - Prêt pour la suite");
}

// ============================================================
// 🔥 SETUP - ORDRE CRITIQUE CORRIGÉ 🔥
// ============================================================
void setup() {
  Serial.begin(115200);

  delay(2000);

  pinMode(BUZZER_PIN, OUTPUT);
  beep(NOTE_C5, 200);

  Serial.println("\n\n\n");
  Serial.println("========================================");
  Serial.println("   ROBOT MEDICAL SCOLAIRE v5.3");
  Serial.println("   MODE SANS CACHE EEPROM");
  Serial.println("========================================");
  Serial.println();

  delay(1000);

  Serial.println("[INIT] Étape 1/5 - Écran TFT 480x320...");
  initTFT();
  delay(300);

  Serial.println("[INIT] Étape 2/5 - Écran de démarrage...");
  bootScreen();

  Serial.println("[INIT] Étape 3/5 - Configuration...");
  initConfig();

  Serial.println("[INIT] Étape 4/5 - Bus I2C...");
  initI2C();
  delay(200);

  Serial.println("[INIT] Étape 5/5 - Communication ESP32 esclave...");
  initRFID();
  delay(200);

  Serial.println("[INIT] Étape 6/6 - Finalisation...");
  initBuzzer();

  Serial.println();
  Serial.println("[INIT] ========================================");
  Serial.println("[INIT] ✅ SYSTÈME SANS EEPROM INITIALISÉ");
  Serial.println("[INIT] ========================================");
  Serial.println();

  if (WIFI_ENABLED) {
    Serial.println("[INIT] Tentative connexion WiFi...");

    tft.fillScreen(BACKGROUND_COLOR);
    tft.setTextColor(PRIMARY_COLOR, BACKGROUND_COLOR);
    tft.drawCentreString("WiFi Connection...", SCREEN_W / 2, 140, 4);

    bool wifiOk = initWiFi();

    tft.fillScreen(BACKGROUND_COLOR);
    if (wifiOk) {
      tft.setTextColor(SUCCESS_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("WiFi connected", SCREEN_W / 2, 120, 4);
      tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString(WiFi.localIP().toString(), SCREEN_W / 2, 170, 2);
      Serial.println("[INIT] Affichage résultat WiFi OK");
    } else {
      tft.setTextColor(WARNING_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("WiFi hors ligne", SCREEN_W / 2, 140, 4);
      tft.setTextColor(TEXT_COLOR, BACKGROUND_COLOR);
      tft.drawCentreString("Local mode activated", SCREEN_W / 2, 190, 2);
      Serial.println("[INIT] Affichage résultat WiFi Offline");
    }
    delay(1500);

    tft.fillScreen(BACKGROUND_COLOR);
    Serial.println("[INIT] Écran WiFi nettoyé");
  }

  Serial.println("\n[INIT] ========================================");
  Serial.println("[INIT] DEMARRAGE APPLICATION 480x320");
  Serial.println("[INIT] ========================================");
  Serial.println("[INIT] Appel changerEtat(BADGE)...");

  changerEtat(BADGE);

  Serial.println("[INIT] Retour de changerEtat(BADGE)");
  Serial.println("[INIT] État actuel: " + String(etatActuel));
  Serial.println("========================================");
  Serial.println("   SYSTEME OPERATIONNEL 480x320");
  Serial.println("   En attente de badge RFID...");
  Serial.println("   Tapez 'help' pour les commandes");
  Serial.println("========================================\n");
}

// ============================================================
// LOOP PRINCIPAL
// ============================================================
void loop() {
  static unsigned long lastDebug = 0;
  if (millis() - lastDebug > 10000) {
    Serial.println("\n[LOOP] ===== DEBUG ETAT =====");
    Serial.println("[LOOP] État: " + String(etatActuel));
    Serial.println("[LOOP] Temps écoulé: " + String((millis() - tempsDebutEtat) / 1000) + "s");
    Serial.println("[LOOP] Heap libre: " + String(ESP.getFreeHeap()) + " bytes");

    if (etatActuel == BADGE) {
      sendCommand("PING");
    }

    Serial.println("[LOOP] ========================\n");
    lastDebug = millis();
  }

  handleSerial();

  switch (etatActuel) {
    case BADGE:               loopBadge(); break;
    case BIENVENUE:           loopBienvenue(); break;
    case RYTHME_CARDIAQUE:    loopRythmeCardiaque(); break;
    case TEMPERATURE:         loopTemperature(); break;
    case POIDS:               loopPoids(); break;
    case AFFICHAGE:           loopAffichage(); break;
    default:                  break;
  }

  delay(10);
}
