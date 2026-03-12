#!/usr/bin/env python3
"""
Serveur Audio Fedora - ESP32 ↔ PC avec Bluetooth
Reçoit les requêtes HTTP de l'ESP32 et joue l'audio sur le haut-parleur Bluetooth

Installation:
    pip3 install flask pygame
    
Démarrage:
    python3 audio_server_fedora.py
"""

from flask import Flask, request, jsonify
import pygame
import os
import threading
import time
from pathlib import Path

app = Flask(__name__)

# ============================================================
# CONFIGURATION
# ============================================================
AUDIO_DIR = os.path.expanduser("~/audio_robot_medical")  # ~/audio_robot_medical
VOLUME = 0.8  # Volume de lecture (0.0 à 1.0)

# ============================================================
# MAPPING DES CODES AUDIO VERS LES FICHIERS
# ============================================================
AUDIO_FILES = {
    1: "001.mp3",   # BADGE - "Bienvenue. Veuillez scanner votre carte..."
    2: "002.mp3",   # YEUX - "Identification réussie. Première étape: analyse oculaire..."
    3: "003.mp3",   # RYTHME - "Analyse terminée. Posez votre doigt sur le capteur..."
    4: "004.mp3",   # TEMP - "Rapprochez votre front du capteur de température..."
    5: "005.mp3",   # POIDS - "Dernière étape: le poids. Montez sur la balance..."
    6: "006.mp3"    # FIN - "Merci. Votre bilan complet est terminé..."
}

# ============================================================
# INITIALISATION PYGAME MIXER
# ============================================================
try:
    pygame.mixer.init()
    pygame.mixer.music.set_volume(VOLUME)
    print("[AUDIO] ✅ Mixer pygame initialisé")
except Exception as e:
    print(f"[AUDIO] ⚠️  Erreur init mixer: {e}")

lecture_en_cours = False
verrou_lecture = threading.Lock()

# ============================================================
# FONCTION DE LECTURE AUDIO
# ============================================================
def jouer_audio(code_audio):
    """
    Joue le fichier audio correspondant au code reçu
    """
    global lecture_en_cours
    
    if code_audio not in AUDIO_FILES:
        print(f"[ERREUR] Code audio inconnu: {code_audio}")
        return False
    
    fichier = os.path.join(AUDIO_DIR, AUDIO_FILES[code_audio])
    
    if not os.path.exists(fichier):
        print(f"[ERREUR] Fichier introuvable: {fichier}")
        print(f"[INFO] Fichiers disponibles: {os.listdir(AUDIO_DIR) if os.path.exists(AUDIO_DIR) else 'Dossier inexistant'}")
        return False
    
    try:
        with verrou_lecture:
            # Arrêter toute lecture en cours
            if pygame.mixer.music.get_busy():
                pygame.mixer.music.stop()
            
            # Charger et jouer le nouveau fichier
            pygame.mixer.music.load(fichier)
            pygame.mixer.music.play()
            lecture_en_cours = True
            
            print(f"[AUDIO] ▶️  Lecture: {AUDIO_FILES[code_audio]}")
        
        # Attendre la fin de la lecture (thread séparé)
        def attendre_fin():
            global lecture_en_cours
            while pygame.mixer.music.get_busy():
                time.sleep(0.1)
            lecture_en_cours = False
            print(f"[AUDIO] ✅ Lecture terminée: {AUDIO_FILES[code_audio]}")
        
        threading.Thread(target=attendre_fin, daemon=True).start()
        return True
        
    except Exception as e:
        print(f"[ERREUR] Impossible de jouer {fichier}: {e}")
        lecture_en_cours = False
        return False

# ============================================================
# ROUTE PRINCIPALE - RÉCEPTION SIGNAL ESP32
# ============================================================
@app.route('/play_audio', methods=['POST'])
def play_audio():
    """
    Endpoint pour recevoir les demandes de lecture audio de l'ESP32
    Format JSON attendu: {"audio_code": 1}
    """
    try:
        data = request.get_json()
        
        if not data or 'audio_code' not in data:
            return jsonify({
                'status': 'error',
                'message': 'Code audio manquant'
            }), 400
        
        code_audio = int(data['audio_code'])
        print(f"\n[REQUÊTE] 📨 Reçu de ESP32 - Code: {code_audio}")
        
        # Jouer l'audio
        if jouer_audio(code_audio):
            return jsonify({
                'status': 'success',
                'message': f'Audio {code_audio} en cours de lecture',
                'file': AUDIO_FILES.get(code_audio, 'unknown')
            }), 200
        else:
            return jsonify({
                'status': 'error',
                'message': 'Impossible de jouer l\'audio'
            }), 500
            
    except Exception as e:
        print(f"[ERREUR] {e}")
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500

# ============================================================
# ROUTE DE TEST
# ============================================================
@app.route('/test/<int:code>', methods=['GET'])
def test_audio(code):
    """
    Route de test pour vérifier les fichiers audio
    Utilisation: http://localhost:5000/test/1
    """
    print(f"\n[TEST] 🧪 Test du code audio {code}")
    if jouer_audio(code):
        return f"✅ Test réussi - Code {code}", 200
    else:
        return f"❌ Test échoué - Code {code}", 500

# ============================================================
# ROUTE DE STATUT
# ============================================================
@app.route('/status', methods=['GET'])
def status():
    """
    Vérifie l'état du serveur et des fichiers audio
    """
    fichiers_manquants = []
    fichiers_presents = []
    
    if not os.path.exists(AUDIO_DIR):
        return jsonify({
            'status': 'error',
            'message': f'Dossier {AUDIO_DIR} inexistant',
            'fichiers_presents': [],
            'fichiers_manquants': list(AUDIO_FILES.values())
        }), 500
    
    for code, fichier in AUDIO_FILES.items():
        chemin = os.path.join(AUDIO_DIR, fichier)
        if os.path.exists(chemin):
            fichiers_presents.append(fichier)
        else:
            fichiers_manquants.append(fichier)
    
    return jsonify({
        'status': 'online',
        'lecture_en_cours': lecture_en_cours,
        'fichiers_presents': fichiers_presents,
        'fichiers_manquants': fichiers_manquants,
        'volume': VOLUME,
        'audio_dir': AUDIO_DIR
    }), 200

# ============================================================
# ROUTE DE CONTRÔLE VOLUME
# ============================================================
@app.route('/volume/<float:level>', methods=['GET'])
def set_volume(level):
    """
    Ajuste le volume de lecture
    Utilisation: http://localhost:5000/volume/0.5
    """
    global VOLUME
    try:
        if 0.0 <= level <= 1.0:
            VOLUME = level
            pygame.mixer.music.set_volume(VOLUME)
            print(f"[AUDIO] 🔊 Volume ajusté à {VOLUME * 100}%")
            return jsonify({
                'status': 'success',
                'volume': VOLUME
            }), 200
        else:
            return jsonify({
                'status': 'error',
                'message': 'Volume doit être entre 0.0 et 1.0'
            }), 400
    except Exception as e:
        return jsonify({
            'status': 'error',
            'message': str(e)
        }), 500

# ============================================================
# MAIN
# ============================================================
if __name__ == '__main__':
    print("\n" + "="*70)
    print("   SERVEUR AUDIO FEDORA - Robot Médical")
    print("   Communication: ESP32 ↔ PC (via HTTP)")
    print("   Sortie: Haut-parleur Bluetooth")
    print("="*70)
    print(f"📁 Dossier audio: {AUDIO_DIR}")
    print(f"🔊 Volume: {VOLUME * 100}%")
    print(f"🌐 Port: 5000")
    print(f"💻 Adresse locale: http://localhost:5000")
    print("="*70 + "\n")
    
    # Créer le dossier audio s'il n'existe pas
    if not os.path.exists(AUDIO_DIR):
        print(f"⚠️  Dossier {AUDIO_DIR} inexistant")
        print(f"   Création du dossier...")
        os.makedirs(AUDIO_DIR, exist_ok=True)
        print(f"   ✅ Dossier créé: {AUDIO_DIR}")
    
    # Vérifier les fichiers audio
    print("\n📋 Vérification des fichiers audio:")
    for code, fichier in AUDIO_FILES.items():
        chemin = os.path.join(AUDIO_DIR, fichier)
        if os.path.exists(chemin):
            taille = os.path.getsize(chemin) / 1024 / 1024  # en MB
            print(f"   ✅ [{code}] {fichier} ({taille:.1f} MB)")
        else:
            print(f"   ❌ [{code}] {fichier} - MANQUANT")
    
    print("\n🚀 Démarrage du serveur...\n")
    
    # Lancer le serveur Flask
    app.run(
        host='0.0.0.0',  # Écoute sur toutes les interfaces
        port=5000,
        debug=False,
        threaded=True
    )
