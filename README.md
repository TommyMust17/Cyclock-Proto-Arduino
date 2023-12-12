# Proto Arduino du boitier Cyc'lock

## Description

Le boitier a pour but de manipuler des serrures en fonction des messages qu'il reçoit via BLE. 

## Pré-Requis

- Une arduino uno wifi rev2
- Serrures et les relais associés
- Alimentation
- Moyen de communication bluetooth (NRF connect ou appli sur mesure)

## Instruction

Pour ouvrir une serrure, il faut se connecter en bluetooth à l'appareil. Il est ensuite nécessaire écrire 2210 dans la caractéristique "timeCharacteristic "F002". Ceci est une simulation de vérification horaire. Si vous ne voulez pas de cette étape, vous avez juste à changer la valeur de la variable "isTimeVerified" de 0 à 1. Pour ouvrir la serrure , il faut écrire dans la caractéristique "openCharacteristic" F001 la valeur correspondante :

  LOCK1_CODE = 13,
  LOCK2_CODE = 7,
  LOCK3_CODE = 10,
  LOCK4_CODE = 18.
  
Lorsque la serrure est s'ouvre,, la caracteristique timeCharacteristic prends la valeur 66, et prends la valeur 77 quand elle se ferme. Si le message écrit n'est pas bon la caractéristique prends la valeur 44. Les pins associé aux serrures sont modifiables dans LOCK_PINS[]. 

## Librairie à installer

- ArduinoBLE
