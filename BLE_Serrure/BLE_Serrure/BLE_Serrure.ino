// Inclusion des bibliothèques nécessaires
#include <ArduinoBLE.h>
#include "config.h"

// Initialisation des objets BLE
BLEService lockService(BLE_SERVICE_UUID);  // Service BLE
BLELongCharacteristic openCharacteristic(BLE_OPEN_UUID, BLERead | BLEWrite);  // Caractéristique pour l'ouverture des verrous
BLEIntCharacteristic timeCharacteristic(BLE_TIME_UUID, BLERead | BLEWrite);  // Caractéristique pour la gestion du temps
BLEIntCharacteristic infoCharacteristic(BLE_INFO_UUID, BLERead | BLENotify | BLEWrite);  // Caractéristique pour les informations supplémentaires

// Variables globales
unsigned long lockOpenTime[TOTAL_LOCKS] = { 0 };  // Temps d'ouverture pour chaque verrou
unsigned long startTime;  // Temps de début de la connexion
unsigned long currentMillis;  // Temps actuel en millisecondes
int isTimeVerified = 0;  // État de la vérification
int totalConnections = 0;  // Compteur de connexions
int totalOpenings = 0;  // Compteur d'ouvertures

// Prototypes de fonctions pour les gestionnaires d'événements
void onBLEConnect(BLEDevice central);
void onBLEDisconnect(BLEDevice central);
void onTimeWritten(BLEDevice central, BLECharacteristic characteristic);
void onOpenWritten(BLEDevice central, BLECharacteristic characteristic);
void onInfoWritten(BLEDevice central, BLECharacteristic characteristic);
void handleLockOpening(long receivedCode);

// Fonction setup() pour l'initialisation
void setup() {
  // Configuration des pins de verrouillage en mode sortie
  for (int i = 0; i < TOTAL_LOCKS; ++i) {
    pinMode(LOCK_PINS[i], OUTPUT);
  }

  // Initialisation de la communication série pour le débogage
  Serial.begin(9600);
  while (!Serial);  // Attendre que la communication série soit prête

  // Initialisation du BLE avec gestion des erreurs
  int bleInitializationAttempts = 0;
  while (!BLE.begin()) {
    Serial.println("Initialisation BLE échouée. Tentative de reconnexion...");
    bleInitializationAttempts++;
    if (bleInitializationAttempts >= 3) {
      Serial.println("Échec de l'initialisation du BLE après 3 tentatives. Arrêt du système.");
      while (1);  // Arrêt du programme
    }
    delay(2000);  // Attendre 2 secondes avant de réessayer
  }

  // Configuration du BLE
  BLE.setLocalName(DEVICE_NAME);
  BLE.setAdvertisedService(lockService);
  lockService.addCharacteristic(openCharacteristic);
  lockService.addCharacteristic(timeCharacteristic);
  lockService.addCharacteristic(infoCharacteristic);
  BLE.addService(lockService);

  // Configuration des gestionnaires d'événements
  BLE.setEventHandler(BLEConnected, onBLEConnect);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnect);
  timeCharacteristic.setEventHandler(BLEWritten, onTimeWritten);
  openCharacteristic.setEventHandler(BLEWritten, onOpenWritten);
  infoCharacteristic.setEventHandler(BLEWritten, onInfoWritten);

  // Démarrage de la publicité
  BLE.advertise();
  Serial.println("En attente de connexion...");

  runTests();
}

// Fonction loop() pour la logique principale
void loop() {
  // Mise à jour du temps actuel
  currentMillis = millis();

  // Gestion de la déconnexion en cas de dépassement du temps maximal
  if (BLE.connected()) {
    if (currentMillis - startTime >= MAX_CONNECTION_DURATION_MS) {
      BLE.disconnect();
      Serial.println("Temps de connexion dépassé. Déconnexion...");
    }
  }

  for(int i = 0; i < TOTAL_LOCKS; ++i) {
    closeLock(LOCK_PINS[i], lockOpenTime[i]);
  }
  // Sondage pour les événements BLE
  BLE.poll(BLE_POLL_INTERVAL_MS);
}

void openLock(int lockPin) {
  digitalWrite(lockPin, HIGH);
  openCharacteristic.writeValue(RETURN_LOCK_OPEN);
}

inline void closeLock(int lockPin, unsigned long &lockOpenTime) {
  if (currentMillis - lockOpenTime >= LOCK_CLOSE_DELAY_MS) {
    digitalWrite(lockPin, LOW);
    openCharacteristic.writeValue(RETURN_LOCK_CLOSE);
  }
}

// Fonction pour déchiffrer le code reçu
inline long decryptCode(long receivedCode, int d, int N) {
  long decryptionBase = 1;
  long decryptedValue = 0;
  int exponentCounter = d;

  // Calcul du déchiffrement
  while (exponentCounter != 0) {
    decryptionBase = decryptionBase * receivedCode;
    exponentCounter = exponentCounter - 1;
  }
  decryptedValue = decryptionBase % N;
  return decryptedValue;
}

// Gestionnaire d'événement pour la connexion BLE
void onBLEConnect(BLEDevice central) {
  Serial.print("Connexion : ");
  Serial.println(central.address());
  startTime = currentMillis;  // Mise à jour du temps de début
  totalConnections++;  // Incrémentation du compteur de connexions
}

// Gestionnaire d'événement pour la déconnexion BLE
void onBLEDisconnect(BLEDevice central) {
  Serial.print("Déconnexion : ");
  Serial.println(central.address());
}

// Gestionnaire d'événement pour l'écriture de la caractéristique de temps
void onTimeWritten(BLEDevice central, BLECharacteristic characteristic) {
  uint32_t receivedTime = timeCharacteristic.value();
  int receivedHours = receivedTime / 100;
  int receivedMinutes = receivedTime % 100;

  // Vérification de la conformité de l'heure
  if (HOUR_REFERENCE == receivedHours && MINUTE_REFERENCE == receivedMinutes) {
    Serial.println("L'heure est conforme");
    isTimeVerified = 1;
  } else {
    Serial.println("Heure non conforme");
    isTimeVerified = 0;
    infoCharacteristic.writeValue(1);  // Écriture d'une valeur d'erreur
  }
}

// Gestionnaire d'événement pour l'écriture de la caractéristique d'ouverture
void onOpenWritten(BLEDevice central, BLECharacteristic characteristic) {
  long receivedCode = openCharacteristic.value();
  handleLockOpening(receivedCode);  // Gestion de l'ouverture du verrou
}

// Fonction pour gérer l'ouverture du verrou
void handleLockOpening(long receivedCode) {
  long decryptedCode = decryptCode(receivedCode, RSA_DECRYPTION_EXPONENT, RSA_MODULUS);

  // Vérification de la conformité du code
  if (isTimeVerified == 1) {
    for (int i = 0; i < TOTAL_LOCKS; ++i) {
      if (decryptedCode == LOCK_CODES[i]) {
        Serial.print("Ouverture de la Serrure N°");
        Serial.println(i + 1);
        lockOpenTime[i] = currentMillis;  // Mise à jour du temps d'ouverture
        openLock(LOCK_PINS[i]);  // Opération du verrou
        totalOpenings++;  // Incrémentation du compteur d'ouvertures
        return;
      }
    }
    Serial.println("Le message envoyé n'est pas bon");  // Message d'erreur
    openCharacteristic.writeValue(RETURN_WRONG_MESS);
  }
}

// Gestionnaire d'événement pour l'écriture de la caractéristique d'information
void onInfoWritten(BLEDevice central, BLECharacteristic characteristic) {
  if (infoCharacteristic.value() == INFO_VALUE) {
    Serial.print("Nombre de connexion : ");
    Serial.println(totalConnections);  // Affichage du nombre de connexions
    Serial.print("Nombre d'Ouverture : ");
    Serial.println(totalOpenings);  // Affichage du nombre d'ouvertures
  }
}

void runTests() {
  Serial.println("Début des tests...");

  // Test 1: Simuler une connexion et une déconnexion Bluetooth
  Serial.println("Test 1: Simulation de la connexion et de la déconnexion Bluetooth");
  BLEDevice dummyDevice;
  onBLEConnect(dummyDevice);
  onBLEDisconnect(dummyDevice);

  // Test 2: Tester l'écriture des caractéristiques
  Serial.println("Test 2: Tester l'écriture des caractéristiques");
  timeCharacteristic.writeValue(2210);  // Heure valide
  onTimeWritten(dummyDevice, timeCharacteristic);
  timeCharacteristic.writeValue(1234);  // Heure invalide
  onTimeWritten(dummyDevice, timeCharacteristic);

  // Test 3: Tester l'ouverture de la serrure
  Serial.println("Test 3: Tester l'ouverture de la serrure");
  long validCode = 13;  // Un code valide
  long invalidCode = 99;  // Un code invalide
  handleLockOpening(validCode);
  handleLockOpening(invalidCode);

  // Test 4: Tester la gestion du temps
  Serial.println("Test 4: Tester la gestion du temps");
  lockOpenTime[0] = millis() - (LOCK_CLOSE_DELAY_MS + 1);
  loop();  // Devrait fermer la serrure

  // Test 5: Tester les limites de temps de connexion
  Serial.println("Test 5: Tester les limites de temps de connexion");
  startTime = millis() - (MAX_CONNECTION_DURATION_MS + 1);
  loop();  // Devrait déconnecter

  // Test 6: Tester la fonction de déchiffrement
  Serial.println("Test 6: Tester la fonction de déchiffrement");
  long encrypted = 13;  // Un exemple de code chiffré
  long decrypted = decryptCode(encrypted, RSA_DECRYPTION_EXPONENT, RSA_MODULUS);
  if (decrypted == 13) {
    Serial.println("Déchiffrement réussi");
  } else {
    Serial.println("Échec du déchiffrement");
  }

  Serial.println("Fin des tests.");
}


