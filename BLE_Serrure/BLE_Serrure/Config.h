// Config.h
#pragma once

// Constantes et paramètres de configuration
#define DEVICE_NAME "Cyclock_Proto"  // Nom du dispositif BLE
#define BLE_SERVICE_UUID "F000"  // UUID du service BLE
#define BLE_OPEN_UUID "F001"  // UUID de la caractéristique d'ouverture
#define BLE_TIME_UUID "F002"  // UUID de la caractéristique de temps
#define BLE_INFO_UUID "F003"  // UUID de la caractéristique d'information

const unsigned long MAX_CONNECTION_DURATION_MS = 10000000;  // Durée maximale de connexion en millisecondes
const unsigned long LOCK_CLOSE_DELAY_MS = 3000;  // Délai pour le verrouillage en millisecondes
const unsigned long BLE_POLL_INTERVAL_MS = 10000;  // Délai pour le sondage BLE en millisecondes

const int RSA_MODULUS = 30;  // Modulus pour le déchiffrement
const int RSA_DECRYPTION_EXPONENT = 3;  // Exposant pour le déchiffrement
const int LOCK_PINS[] = { 13, 12, 15, 16 };  // Pins utilisés pour les verrous
const int TOTAL_LOCKS = sizeof(LOCK_PINS) / sizeof(LOCK_PINS[0]);  // Nombre total de verrous
const int INFO_VALUE = 5;  // Valeur pour la caractéristique d'information

// Constantes pour les codes de verrou
enum MessBle {
  LOCK1_CODE = 13,
  LOCK2_CODE = 7,
  LOCK3_CODE = 10,
  LOCK4_CODE = 18,
  RETURN_LOCK_OPEN = 1000,
  RETURN_LOCK_CLOSE = 2000,
  RETURN_WRONG_MESS = 404
};
const int LOCK_CODES[] = {LOCK1_CODE, LOCK2_CODE, LOCK3_CODE, LOCK4_CODE};  // Tableau des codes de verrou

// Constantes pour la référence temporelle
enum TimeRef {
  HOUR_REFERENCE = 22,
  MINUTE_REFERENCE = 10
};



