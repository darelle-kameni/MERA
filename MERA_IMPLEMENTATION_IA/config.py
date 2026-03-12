#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
CONFIGURATION CENTRALILSÉE - FLASK SERVER
Modifiez ce fichier pour adapter à votre configuration
"""

# ============================================================================
# CONFIGURATION SERVEUR FLASK
# ============================================================================
FLASK_PORT = 5000
FLASK_DEBUG = True
FLASK_HOST = '0.0.0.0'  # Accessible sur tout le réseau

# ============================================================================
# CHEMINS FICHIERS
# ============================================================================
BASE_DIR = '/home/mera/mera_detection'
RESULTAT_DIR = f'{BASE_DIR}/resultat'
CHECKPOINT_FILE = f'{BASE_DIR}/eye_disease.ckpt'
LANDMARKER_FILE = f'{BASE_DIR}/face_landmarker.task'

# ============================================================================
# CONFIGURATION CAMÉRA
# ============================================================================
CAMERA_RESOLUTION_WIDTH = 2560      # 5MP: 2560x1920
CAMERA_RESOLUTION_HEIGHT = 1920
CAMERA_QUALITY = 90                  # JPEG quality 0-100
CAMERA_BRIGHTNESS = 0.3              # +30%
CAMERA_CONTRAST = 1.5                # 1.5x
CAMERA_SATURATION = 1.2              # 1.2x
CAMERA_SHUTTER = 20000               # Microsecondes
CAMERA_GAIN = 2.5                    # Gain du capteur
CAMERA_TIMEOUT = 10                  # Secondes

# ============================================================================
# CONFIGURATION IA
# ============================================================================
EYE_CLASSES = {
    0: "Sain",
    1: "Cataracte",
    2: "Conjunctivite",
    3: "Jaunisse",
    4: "Pterygion",
    5: "Blepharite",
    6: "Uveitis"
}

MIN_EYE_SIZE = 15                    # Pixels minimum pour un oeil
NUM_EYES_REQUIRED = 2                # Nombre d'yeux à détecter

# ============================================================================
# CONFIGURATION SESSIONS
# ============================================================================
SESSION_CLEANUP_INTERVAL = 300       # Secondes (5 minutes)
SESSION_EXPIRY_TIME = 3600           # Secondes (1 heure)
SESSION_TIMEOUT = 30                 # Secondes (timeout de traitement)

# ============================================================================
# CONFIGURATION RÉSEAU (TIMEOUTS)
# ============================================================================
HTTP_REQUEST_TIMEOUT = 5             # Secondes
HTTP_RETRY_ATTEMPTS = 3
HTTP_RETRY_DELAY = 1                 # Secondes

# ============================================================================
# CONFIGURATION LOGS
# ============================================================================
LOG_LEVEL = 'INFO'                   # DEBUG, INFO, WARNING, ERROR
LOG_FILE = f'{BASE_DIR}/flask_server.log'
LOG_TO_CONSOLE = True
LOG_TO_FILE = False

# ============================================================================
# CONFIGURATION ESP32 (À ADAPTER)
# ============================================================================
ESP32_IP = "192.168.43.97"           # ⚠️ À ADAPTER: IP de l'ESP32
ESP32_PORT = 8000

# ============================================================================
# CONFIGURATION SERVEUR WEB EXISTANT
# ============================================================================
WEB_SERVER_IP = "192.168.43.97"      # ⚠️ À ADAPTER: IP du serveur web
WEB_SERVER_PORT = 8000
WEB_SERVER_API_ARDUINO = "http://192.168.43.97:8000/backend/api_arduino.php"
WEB_SERVER_ADD_MESURE = "http://192.168.43.97:8000/backend/add_mesure.php"

# ============================================================================
# CONFIGURATION MINDSPORE
# ============================================================================
MINDSPORE_MODE = 'GRAPH_MODE'        # GRAPH_MODE ou PYNATIVE_MODE
MINDSPORE_DEVICE = 'CPU'             # CPU, GPU, ou Ascend
MINDSPORE_NUM_THREADS = 4

# ============================================================================
# FONCTION UTILITAIRE POUR CHARGER
# ============================================================================
def get_config():
    """Retourner la configuration sous forme de dictionnaire"""
    return {
        'flask': {
            'port': FLASK_PORT,
            'debug': FLASK_DEBUG,
            'host': FLASK_HOST
        },
        'paths': {
            'base': BASE_DIR,
            'resultat': RESULTAT_DIR,
            'checkpoint': CHECKPOINT_FILE,
            'landmarker': LANDMARKER_FILE
        },
        'camera': {
            'width': CAMERA_RESOLUTION_WIDTH,
            'height': CAMERA_RESOLUTION_HEIGHT,
            'quality': CAMERA_QUALITY,
            'brightness': CAMERA_BRIGHTNESS,
            'contrast': CAMERA_CONTRAST,
            'saturation': CAMERA_SATURATION,
            'shutter': CAMERA_SHUTTER,
            'gain': CAMERA_GAIN,
            'timeout': CAMERA_TIMEOUT
        },
        'ia': {
            'classes': EYE_CLASSES,
            'min_eye_size': MIN_EYE_SIZE,
            'num_eyes': NUM_EYES_REQUIRED
        },
        'sessions': {
            'cleanup_interval': SESSION_CLEANUP_INTERVAL,
            'expiry': SESSION_EXPIRY_TIME,
            'timeout': SESSION_TIMEOUT
        },
        'network': {
            'http_timeout': HTTP_REQUEST_TIMEOUT,
            'retry_attempts': HTTP_RETRY_ATTEMPTS,
            'retry_delay': HTTP_RETRY_DELAY,
            'esp32': f'http://{ESP32_IP}:{ESP32_PORT}',
            'web_server': f'http://{WEB_SERVER_IP}:{WEB_SERVER_PORT}'
        }
    }

if __name__ == '__main__':
    import json
    print("Configuration actuelle:")
    print(json.dumps(get_config(), indent=2))
