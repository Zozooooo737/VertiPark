# VertiPark — Parking vertical automatisé et connecté

> Prototype de parking intelligent à nacelles rotatives, piloté par un ESP32 et supervisé depuis une plateforme web PHP/MySQL.
> Projet BIN1 · ESILV · 2025-2026 · Équipe BIC-100

![ESP32](https://img.shields.io/badge/ESP32-NodeMCU-E7352C?logo=espressif&logoColor=white)
![PlatformIO](https://img.shields.io/badge/PlatformIO-firmware-F5822A?logo=platformio&logoColor=white)
![PHP](https://img.shields.io/badge/PHP-natif-777BB4?logo=php&logoColor=white)
![MySQL](https://img.shields.io/badge/MySQL-InnoDB-4479A1?logo=mysql&logoColor=white)
![SolidWorks](https://img.shields.io/badge/SolidWorks-CAO-D52B1E)
![KiCad](https://img.shields.io/badge/KiCad-schéma-314CB0?logo=kicad&logoColor=white)

<!-- Ajoutez ici une photo de la maquette : ![Maquette VertiPark](docs/maquette.jpg) -->

---

## Sommaire

- [Le projet](#le-projet)
- [Fonctionnalités](#fonctionnalités)
- [Architecture](#architecture)
- [Matériel](#matériel)
- [Structure du dépôt](#structure-du-dépôt)
- [Installation](#installation)
- [API REST](#api-rest)
- [Communication ESP32 ↔ serveur](#communication-esp32--serveur)
- [Base de données](#base-de-données)
- [Limites et perspectives](#limites-et-perspectives)
- [Équipe](#équipe)

---

## Le projet

En ville, chercher une place fait perdre du temps, augmente le trafic et les émissions de CO₂. **VertiPark** propose une réponse à petite échelle : un parking **vertical** qui empile les véhicules sur des nacelles entraînées par une chaîne, pour réduire l'emprise au sol.

La maquette réunit trois sous-systèmes :

| Sous-système | Contenu |
|---|---|
| **Mécanique** | Structure en bois découpée au laser, nacelles à liaison pivot, transmission par chaîne à maillons modulaires et pignon central, barrière d'accès. Conçue sous SolidWorks. |
| **Électronique** | ESP32 sur carte dédiée, capteurs de proximité multiplexés, LEDs d'état pilotées par registre à décalage, lecteur RFID, écran OLED, alimentation photovoltaïque. |
| **Informatique** | Firmware C++ (PlatformIO) + plateforme web PHP/MySQL avec API REST, tableau de bord temps réel, espace admin et système de crédits. |

---

## Fonctionnalités

- **Détection des véhicules** dans chaque nacelle (capteurs SEN0556), avec anti-rebond et validation temporelle.
- **LEDs d'état** sur chaque place : vert = libre, rouge = occupée, orange = réservée.
- **Barrière automatique** commandée par servomoteur.
- **Accès par badge RFID** : entrée, sortie et facturation automatiques.
- **Tableau de bord web temps réel** (rafraîchissement toutes les 2 s, sans recharger la page).
- **Réservation en ligne** d'une place libre en un clic, et ouverture de la barrière depuis le navigateur.
- **Système de crédits** en minutes : débit automatique à la sortie, notification si le solde est insuffisant.
- **Espace administrateur** : historique des stationnements, gestion des utilisateurs et des rôles, association d'un badge à un compte.
- **Énergie solaire** : panneau photovoltaïque pour une alimentation partielle.

---

## Architecture

```
┌──────────────────────┐        HTTP / JSON (Wi-Fi local)        ┌──────────────────────────┐
│        ESP32         │ ──── POST update_sensor.php ──────────▶ │   Serveur web (XAMPP)    │
│                      │ ──── POST rfid_check.php ─────────────▶ │                          │
│  capteurs · RFID     │ ──── GET  status.php ─────────────────▶ │   PHP  +  API REST       │
│  servos · LEDs · OLED│ ◀─── GET  action_parking.txt (polling)  │   MySQL (vertipark)      │
└──────────────────────┘                                         └────────────┬─────────────┘
                                                                              │ polling 2 s
                                                                   ┌──────────▼───────────┐
                                                                   │  Navigateur (client  │
                                                                   │  / administrateur)   │
                                                                   └──────────────────────┘
```

Le serveur web suit un découpage en trois couches :

- **Données** : MySQL, via `config/db.php` (instance PDO singleton, mode `ERRMODE_EXCEPTION`, requêtes préparées).
- **Métier** : un script PHP par opération dans `api/`, réponses JSON avec codes HTTP sémantiques (200, 201, 400, 404, 405, 409, 500).
- **Présentation** : pages PHP (`index.php`, `profil.php`, `admin.php`) + JavaScript pour la mise à jour dynamique.

Côté firmware, chaque composant est encapsulé dans sa propre classe (patron **Manager**, ex. `GateServoManager`), et tous les brochages sont centralisés dans `config.h`.

---

## Matériel

| Composant | Référence | Rôle |
|---|---|---|
| Microcontrôleur | ESP32 NodeMCU | Pilotage général et Wi-Fi |
| Servomoteur | FS90 | Barrière d'accès |
| Servomoteur 360° | FB5311M-360 | Rotation des nacelles |
| Capteur de proximité IR | SEN0556 | Présence des véhicules |
| Capteur à effet Hall | — | Position angulaire du mécanisme |
| Lecteur RFID | RC522 | Identification des usagers |
| Afficheur OLED | Grove 128×64 | Affichage local |
| Capteur temp./humidité | DHT22 | Surveillance de l'environnement |
| Panneau + module d'alimentation | DFR0579 | Énergie solaire |
| Buzzer piézo passif | — | Signaux sonores |
| Registre à décalage | 74HC595 | Commande des LEDs |
| Inverseurs | 74HC04 | Couplage LED rouge / verte |
| Multiplexeur | 74HC4051 | Lecture des capteurs |
| Transistors | 2N2222 | Commutation des LEDs (5 V) |

**Astuces électroniques :**
- 16 LEDs pilotées avec 3 broches grâce au **74HC595**. Comme une place est soit verte soit rouge, une **porte NOT (74HC04)** dérive l'une de l'autre, ce qui divise par deux le nombre de sorties nécessaires.
- Les LEDs sont alimentées en 5 V via des **transistors 2N2222**, car les GPIO de l'ESP32 sont en 3,3 V.
- Les capteurs passent par un **multiplexeur 74HC4051** pour économiser les GPIO.

---

## Structure du dépôt

<!-- À vérifier et ajuster selon l'organisation réelle du dépôt -->

```
VertiPark/
├── firmware/                 # Projet PlatformIO (ESP32)
│   ├── include/              # En-têtes : config.h, GateServoManager.h, ...
│   ├── src/                  # Sources : main.cpp, GateServoManager.cpp, ...
│   ├── lib/
│   ├── test/
│   └── platformio.ini
├── web/                      # Plateforme web (à placer dans htdocs/)
│   ├── config/db.php         # Connexion PDO
│   ├── api/                  # Endpoints REST
│   ├── index.php             # Tableau de bord
│   ├── profil.php            # Profil utilisateur
│   └── admin.php             # Administration
├── database/                 # Script SQL de la base vertipark
└── docs/                     # Rapport, cahier des charges, schémas, CAO
```

---

## Installation

### 1. Plateforme web

**Prérequis** : [XAMPP](https://www.apachefriends.org/) (Apache + MySQL + PHP).

1. Copier le dossier web dans `xampp/htdocs/vertipark/`.
2. Démarrer **Apache** et **MySQL** depuis le panneau XAMPP.
3. Dans phpMyAdmin, créer la base `vertipark` (encodage `utf8mb4`) et importer le script SQL.
4. Renseigner les identifiants MySQL dans `config/db.php`.
5. Ouvrir `http://localhost/vertipark/`.

Pour que l'ESP32 et les autres appareils accèdent au site, l'ordinateur qui héberge XAMPP doit être sur le **même réseau Wi-Fi**. Notez son adresse IP locale (`ipconfig` sous Windows).

### 2. Firmware ESP32

**Prérequis** : [VS Code](https://code.visualstudio.com/) + extension [PlatformIO](https://platformio.org/).

1. Ouvrir le dossier du firmware dans VS Code.
2. Dans la section de configuration, renseigner :
   - le SSID et le mot de passe du Wi-Fi ;
   - l'adresse IP du serveur et l'URL de l'API.
3. Vérifier les broches dans `config.h` si votre câblage diffère.
4. Brancher l'ESP32 en USB, puis **Build** et **Upload**.
5. Ouvrir le **moniteur série** pour suivre la connexion et les requêtes HTTP.

> **Attention :** ne publiez pas vos vrais identifiants Wi-Fi ni le mot de passe admin : gardez-les dans un fichier ignoré par Git.

---

## API REST

| Endpoint | Méthode | Appelé par | Fonction |
|---|---|---|---|
| `api/status.php` | GET | Frontend, ESP32 | État de toutes les places, trié par numéro |
| `api/update_sensor.php` | POST | ESP32 | Met à jour une place et ouvre/ferme la session de stationnement |
| `api/rfid_check.php` | POST | ESP32 | Identifie un badge, gère assignation, entrée et sortie avec facturation |
| `api/open_web.php` | POST | Client | Déclenche l'entrée ou la sortie depuis le site |
| `api/reserve.php` | POST | Client | Réserve une place libre (verrou `FOR UPDATE`) |
| `api/cancel_client.php` | POST | Client | Annule une réservation active |
| `api/buy_credits.php` | POST | Client | Ajoute des minutes au solde |
| `api/promote.php` · `api/demote.php` | POST | Admin | Change le rôle d'un utilisateur |
| `api/start_assign.php` | POST | Admin | Lance l'association d'un badge RFID à un compte |

Exemple de requête envoyée par l'ESP32 quand un véhicule arrive :

```http
POST /api/update_sensor.php
Content-Type: application/json

{ "numero": 2, "etat": "occupee" }
```

---

## Communication ESP32 ↔ serveur

L'ESP32 n'héberge pas de serveur HTTP : c'est toujours lui qui initie les échanges.

**ESP32 → serveur.** Une requête n'est envoyée que lorsqu'un capteur **change d'état** (détection de front, anti-rebond de 50 ms puis double vérification). Le serveur met à jour la place dans une transaction et ouvre ou clôt la session de stationnement.

**Serveur → ESP32.** Quand un utilisateur clique sur « Entrée » ou « Sortie » sur le site, `open_web.php` écrit un ordre dans `api/action_parking.txt` :

```
ENTREE:2    # descendre la nacelle n°2
SORTIE:2    # remonter la nacelle n°2
```

L'ESP32 interroge ce fichier régulièrement, exécute l'ordre puis l'efface. Si l'ESP32 est momentanément déconnecté, l'ordre attend la prochaine lecture.

**Cycle d'un badge RFID** (`rfid_check.php`) :

1. **Assignation** : si l'admin a lancé le mode assignation (`assignation.txt`), le badge est lié au compte en attente.
2. **Entrée** : badge reconnu, réservation active et solde positif → la nacelle réservée descend.
3. **Sortie** : durée calculée, minutes débitées (1 minute minimum), session clôturée → la nacelle remonte. Si le solde est insuffisant, la barrière reste fermée et l'utilisateur reçoit une notification.

---

## Base de données

Base `vertipark` (MySQL, InnoDB, `utf8mb4`) :

| Table | Rôle |
|---|---|
| `Utilisateur` | Comptes client/admin, badge RFID, solde en minutes |
| `Place` | État temps réel : `libre`, `occupee` ou `reservee` (contrainte `CHECK`) |
| `Réservation` | Réservations : `active`, `terminee` ou `annulee` |
| `Stationnement` | Historique horodaté des entrées et sorties |
| `Notification` | Alertes envoyées aux utilisateurs |
| `Véhicule` / `Appartenance` | Véhicules des clients (relation N:N) |

---

## Limites et perspectives

- **Mécanique** : remplacer le bois par un matériau plus résistant pour les pièces sous contrainte.
- **Communication** : passer aux **WebSockets** pour une communication en temps réel à la place du fichier-signal.
- **Paiement** : intégrer un vrai paiement en ligne.
- **Actionneurs** : ajouter un second moteur (bras de guidage, verrouillage des nacelles).
- **Sécurité** : l'accès admin par mot de passe en paramètre GET convient à une démo locale, mais pas à une mise en production.

---

## Équipe

Projet réalisé à l'**ESILV** (Projet BIN1, 2025-2026) par l'équipe **BIC-100** :

- **Enzo BOTTURA**
- **Alexandre KOUAKOU**
- **Lenny MURRAY**
- **Mattia SCIACCA**

Projet pédagogique. Les données RFID et de réservation utilisées pendant la démonstration sont fictives.
