#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
SERVEUR FLASK - API POUR DÉTECTION OCULAIRE
Raspberry Pi - Port 5000
Reçoit patient_id via POST /api/capture
Lance la capture + détection d'yeux en arrière-plan
Retourne les résultats via GET /api/results/<session_id>
"""

import os
import sys
import json
import uuid
import threading
import time
from datetime import datetime
from flask import Flask, request, jsonify
import cv2
import numpy as np

# Pour les audios locaux - PYDUB (meilleure qualité que pygame)
try:
    import os
    # Définir le chemin ffprobe pour pydub (systemd n'utilise pas le PATH complet)
    os.environ['PATH'] = '/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin'
    
    # Force ALSA à utiliser le device jackpour l'audio (carte 2 = headphones)
    os.environ['ALSA_CARD'] = 'Headphones'
    
    from pydub import AudioSegment
    from pydub.playback import play
    AUDIO_AVAILABLE = True
except ImportError:
    AUDIO_AVAILABLE = False
    print("[AUDIO] [WARNING] pydub non installe - audios desactives")
    print("[AUDIO] [INFO] Installer: pip install pydub simpleaudio")

# Configuration environment variable pour MindSpore
os.environ['PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION'] = 'python'

# Imports pour le système de détection oculaire
import mindspore as ms
from mindspore import load_checkpoint, load_param_into_net
import mindspore.nn as nn
import mindspore.ops as ops

# ============================================================================
# CONFIGURATION
# ============================================================================
app = Flask(__name__)
app.config['JSON_SORT_KEYS'] = False

# Port d'écoute
PORT = 5000
DEBUG = False  # Désactiver debug pour éviter que le modèle se charge 2x

# Configuration audios locaux (Raspberry Pi) - SYNCHRONISÉ avec ESP32
AUDIO_DIR = "/home/mera/mera_detection/audio_robot_medical"
AUDIO_FILES = {
    1: "001.mp3",  # AUDIO_BADGE - Badge
    2: "002.mp3",  # AUDIO_YEUX - Yeux / Analyse oculaire en cours
    3: "003.mp3",  # AUDIO_RYTHME - Rythme
    4: "004.mp3",  # AUDIO_TEMPERATURE - Température
    5: "005.mp3",  # AUDIO_POIDS - Poids
    6: "006.mp3",  # AUDIO_FIN - Fin
    7: "007.mp3"   # CAPTURE_DONE - Photo prise (réutiliser 002 pour l'instant)
}

# Cache audio en mémoire (pré-chargé au démarrage)
AUDIO_CACHE = {}

# Classes de maladies
CLASSES = {
    0: "Sain",
    1: "Cataracte",
    2: "Conjunctivite",
    3: "Jaunisse",
    4: "Pterygion",
    5: "Blepharite",
    6: "Uveitis"
}

# Dictionnaire global pour gérer les sessions de traitement
processing_sessions = {}
sessions_lock = threading.Lock()

# ============================================================================
# IMPORT DES MODULES EXISTANTS
# ============================================================================
sys.path.insert(0, '/home/mera/mera_detection')

try:
    # Import spécial pour Untitled-1.py (avec tiret)
    import importlib.util
    spec = importlib.util.spec_from_file_location("untitled_module", "/home/mera/mera_detection/Untitled-1.py")
    untitled_module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(untitled_module)
    
    EyeDetector = untitled_module.EyeDetector
    detect_eyes_hybrid = untitled_module.detect_eyes_hybrid
    
    print("[FLASK] [OK] Modules de detection oculaire charges")
except Exception as e:
    print(f"[FLASK] [ERROR] Erreur import modules: {e}")
    import traceback
    traceback.print_exc()
    EyeDetector = None
    detect_eyes_hybrid = None

# Charger le modèle de détection d'yeux
try:
    detector = EyeDetector("eye_disease.ckpt")
    print("[FLASK] [OK] Modele Eye Disease charge")
except Exception as e:
    print(f"[FLASK] [ERROR] Erreur chargement modele: {e}")
    import traceback
    traceback.print_exc()
    print("[FLASK] [FATAL] Impossible de continuer sans le modèle!")
    sys.exit(1)
    print(f"[FLASK] [ERROR] Erreur chargement modele: {e}")
    detector = None

# ============================================================================
# FONCTIONS UTILITAIRES
# ============================================================================

def preload_audio_cache():
    """
    Pré-charge tous les fichiers audio en mémoire au démarrage
    Permet une lecture INSTANTANÉE sans délai de décodage
    """
    global AUDIO_CACHE
    
    if not AUDIO_AVAILABLE:
        print("[AUDIO] [WARNING] pydub non disponible - cache non charge")
        return False
    
    print("[AUDIO] [INFO] Pre-chargement des audios en memoire...")
    
    loaded_count = 0
    for code, filename in AUDIO_FILES.items():
        audio_path = os.path.join(AUDIO_DIR, filename)
        
        if not os.path.exists(audio_path):
            print(f"[AUDIO] [ERROR] Fichier non trouve: {audio_path}")
            continue
        
        try:
            audio = AudioSegment.from_mp3(audio_path)
            AUDIO_CACHE[code] = audio
            print(f"[AUDIO] [OK] Cache: Code {code} - {filename} ({len(audio)}ms)")
            loaded_count += 1
        except Exception as e:
            print(f"[AUDIO] [ERROR] Impossible de charger {filename}: {e}")
    
    print(f"[AUDIO] [OK] {loaded_count}/{len(AUDIO_FILES)} audios charges en cache\n")
    return loaded_count == len(AUDIO_FILES)

def play_audio_local(audio_code):
    """
    Joue un fichier audio localement sur la Raspberry Pi
    Utilise pydub + simpleaudio pour une meilleure qualité audio (pas de crisse)
    Utilise le cache en mémoire pour une lecture INSTANTANÉE
    """
    if not AUDIO_AVAILABLE:
        print(f"[AUDIO] [WARNING] pydub non disponible - audio code {audio_code} ignore")
        return False
    
    if audio_code not in AUDIO_FILES:
        print(f"[AUDIO] [ERROR] Code audio inconnu: {audio_code}")
        return False
    
    # Vérifier si l'audio est en cache
    if audio_code not in AUDIO_CACHE:
        print(f"[AUDIO] [ERROR] Audio {audio_code} pas en cache - pas charge au demarrage")
        return False
    
    try:
        audio = AUDIO_CACHE[audio_code]
        print(f"[AUDIO] [OK] Lecture INSTANT (cache): {AUDIO_FILES[audio_code]}")
        
        # Jouer l'audio en thread séparé pour pas bloquer
        def play_in_thread():
            try:
                play(audio)
                print(f"[AUDIO] [OK] Lecture terminee: {AUDIO_FILES[audio_code]}")
            except Exception as e:
                print(f"[AUDIO] [ERROR] Erreur lecture: {e}")
        
        threading.Thread(target=play_in_thread, daemon=True).start()
        return True
        
    except Exception as e:
        print(f"[AUDIO] [ERROR] Erreur lecture {AUDIO_FILES[audio_code]}: {e}")
        return False

# ============================================================================
# FONCTIONS DE TRAITEMENT
# ============================================================================

def process_eye_capture(session_id, patient_id):
    """
    Fonction exécutée en thread séparé:
    1. Capture photo via rpicam-jpeg
    2. Détecte les 2 yeux
    3. Fait l'inférence IA sur chaque oeil
    4. Sauvegarde résultats dans processing_sessions
    """
    try:
        timestamp_start = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        with sessions_lock:
            processing_sessions[session_id]["status"] = "capturing"
            processing_sessions[session_id]["timestamp_start"] = datetime.now().isoformat()
        
        print(f"[CAPTURE] [{timestamp_start}] ========== DEBUT SESSION {session_id} ==========")
        print(f"[CAPTURE] [{timestamp_start}] Patient: {patient_id}")
        print(f"[CAPTURE] [{timestamp_start}] Démarrage capture et analyse...")
        print(f"[CAPTURE] [CAM] Debut capture pour patient {patient_id}, session {session_id}")
        
        # Étape 1: Capture photo via rpicam-jpeg
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        output_file = f"/home/mera/mera_detection/resultat/capture_{session_id}_{timestamp}.jpg"
        
        try:
            import subprocess
            subprocess.run(
                [
                    'rpicam-jpeg',
                    '-o', output_file,
                    '--width', '2560',
                    '--height', '1920',
                    '--quality', '90',
                    '--brightness', '0.3',
                    '--contrast', '1.5',
                    '--saturation', '1.2',
                    '--awb', 'auto',
                    '--metering', 'centre',
                    '--shutter', '20000',
                    '--gain', '2.5',
                    '--preview', '0'  # Désactiver preview X/EGL (ralentit tout)
                ],
                check=True,
                timeout=30
            )
            print(f"[CAPTURE] [OK] Photo capturee: {output_file}")
            
            # Signal capture terminee pour ESP32 + jouer audio local
            with sessions_lock:
                processing_sessions[session_id]["capture_done"] = True
            print(f"[CAPTURE] [INFO] Signal capture_done envoye a ESP32 pour bip")
            
            # Jouer audio local sur Raspberry (thread séparé pour pas bloquer)
            # Code 7 = CAPTURE_DONE (photo prise, l'utilisateur peut retirer ses yeux)
            threading.Thread(
                target=lambda: play_audio_local(7),
                daemon=True
            ).start()
            
        except Exception as e:
            print(f"[CAPTURE] [ERROR] Erreur capture: {e}")
            with sessions_lock:
                processing_sessions[session_id]["status"] = "error"
                processing_sessions[session_id]["error"] = f"Capture failed: {str(e)}"
            return
        
        # Étape 2: Charger et analyser la photo
        if not os.path.exists(output_file):
            print(f"[CAPTURE] [ERROR] Fichier photo non trouve")
            with sessions_lock:
                processing_sessions[session_id]["status"] = "error"
                processing_sessions[session_id]["error"] = "Photo file not found"
            return
        
        captured_frame = cv2.imread(output_file)
        if captured_frame is None:
            print(f"[CAPTURE] [ERROR] Impossible de charger la photo")
            with sessions_lock:
                processing_sessions[session_id]["status"] = "error"
                processing_sessions[session_id]["error"] = "Failed to load photo"
            return
        
        print(f"[CAPTURE] [INFO] Photo chargee, detection des yeux...")
        print(f"[CAPTURE] [DEBUG] Taille photo: {captured_frame.shape}")
        
        # Étape 3: Détecter les yeux
        try:
            eyes = detect_eyes_hybrid(captured_frame)
            
            if not eyes or len(eyes) < 2:
                print(f"[CAPTURE] [WARNING] {len(eyes) if eyes else 0} oeil(s) detecte(s)")
                print(f"[CAPTURE] [DEBUG] Checking image quality... Shape: {captured_frame.shape}, Min: {captured_frame.min()}, Max: {captured_frame.max()}")
                with sessions_lock:
                    processing_sessions[session_id]["status"] = "error"
                    processing_sessions[session_id]["error"] = f"NO_EYES_DETECTED: {len(eyes) if eyes else 0} eyes found"
                return
            
            print(f"[CAPTURE] [OK] {len(eyes)} oeil(s) detecte(s)")
        except Exception as e:
            print(f"[CAPTURE] [ERROR] Erreur detection yeux: {e}")
            with sessions_lock:
                processing_sessions[session_id]["status"] = "error"
                processing_sessions[session_id]["error"] = f"Eye detection failed: {str(e)}"
            return
        
        # Étape 4: Analyser chaque oeil avec le modèle IA
        print(f"[CAPTURE] [INFO] Inference IA sur les yeux...")
        
        predictions = []
        for idx, eye_info in enumerate(eyes[:2]):  # Analyser les 2 premiers yeux
            try:
                ex, ey, ew, eh = eye_info['rect']
                eye_crop = captured_frame[ey:ey+eh, ex:ex+ew]
                
                if eye_crop.shape[0] < 15 or eye_crop.shape[1] < 15:
                    print(f"[CAPTURE] [WARNING] Oeil {idx+1} trop petit")
                    continue
                
                pred_class, confidence, probs = detector.predict(eye_crop)
                class_name = CLASSES[pred_class]
                
                predictions.append({
                    'eye': idx + 1,
                    'class': class_name,
                    'confidence': float(confidence),
                    'pred_class': int(pred_class)
                })
                
                print(f"[CAPTURE] [OK] Oeil {idx+1}: {class_name} ({confidence*100:.1f}%)")
            
            except Exception as e:
                print(f"[CAPTURE] [ERROR] Erreur analyse oeil {idx+1}: {e}")
        
        if len(predictions) < 2:
            print(f"[CAPTURE] [ERROR] Impossible d'analyser les 2 yeux")
            with sessions_lock:
                processing_sessions[session_id]["status"] = "error"
                processing_sessions[session_id]["error"] = f"Failed to analyze {2 - len(predictions)} eye(s)"
            return
        
        # Étape 5: Déterminer s'il y a une alerte
        alerte = 0
        for pred in predictions:
            if pred['class'] != "Sain":
                alerte = 1
                break
        
        # Étape 6: Sauvegarder les résultats au format attendu par ESP32
        with sessions_lock:
            processing_sessions[session_id]["status"] = "completed"
            processing_sessions[session_id]["oeil_1"] = {
                "classe": predictions[0]['class'],
                "confiance": f"{predictions[0]['confidence']*100:.2f}%"
            }
            processing_sessions[session_id]["oeil_2"] = {
                "classe": predictions[1]['class'],
                "confiance": f"{predictions[1]['confidence']*100:.2f}%"
            }
            processing_sessions[session_id]["alerte"] = (alerte == 1)
            processing_sessions[session_id]["timestamp_end"] = datetime.now().isoformat()
            processing_sessions[session_id]["image_path"] = output_file
        
        timestamp_end = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        print(f"[CAPTURE] [{timestamp_end}] ========== FIN SESSION {session_id} ==========")
        print(f"[CAPTURE] [OK] Oeil Gauche: {predictions[0]['class']} ({predictions[0]['confidence']*100:.1f}%)")
        print(f"[CAPTURE] [OK] Oeil Droit: {predictions[1]['class']} ({predictions[1]['confidence']*100:.1f}%)")
        print(f"[CAPTURE] [OK] Alerte: {'OUI' if alerte else 'NON'}")
        print(f"[CAPTURE] [{timestamp_end}] Nouvelle session disponible pour récupération")
    
    except Exception as e:
        print(f"[CAPTURE] [ERROR] Erreur non geree: {e}")
        import traceback
        traceback.print_exc()
        with sessions_lock:
            processing_sessions[session_id]["status"] = "error"
            processing_sessions[session_id]["error"] = f"Unhandled error: {str(e)}"

# ============================================================================
# ROUTES API
# ============================================================================

@app.route('/api/capture', methods=['POST'])
def api_capture():
    """
    POST /api/capture
    Reçoit: {"patient_id": 123}
    Retourne immédiatement: {"status": "processing", "session_id": "sess_XXX"}
    Lance le traitement en thread séparé
    """
    try:
        data = request.get_json()
        
        if not data or 'patient_id' not in data:
            return jsonify({"status": "error", "error": "Missing patient_id"}), 400
        
        patient_id = data['patient_id']
        timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
        
        # Vérifier que le modèle est chargé
        if detector is None:
            return jsonify({"status": "error", "error": "Model not loaded"}), 500
        
        # Créer une NOUVELLE session UNIQUE - JAMAIS réutiliser les anciennes!
        session_id = f"sess_{uuid.uuid4().hex[:12]}"
        
        print(f"[API] [{timestamp}] POST /api/capture - Patient: {patient_id} - NEW Session: {session_id}")
        
        with sessions_lock:
            processing_sessions[session_id] = {
                "status": "queued",
                "patient_id": patient_id,
                "timestamp_created": datetime.now().isoformat(),
                "error": None,
                "uuid": uuid.uuid4().hex  # ID unique pour chaque capture
            }
        
        print(f"[API] [INFO] === NOUVELLE CAPTURE ===")
        print(f"[API] Session ID: {session_id}")
        print(f"[API] Patient ID: {patient_id}")
        print(f"[API] Timestamp: {datetime.now().isoformat()}")
        print(f"[API] Nombre de sessions actives: {len(processing_sessions)}")
        
        # Lancer le traitement en thread séparé
        thread = threading.Thread(
            target=process_eye_capture,
            args=(session_id, patient_id),
            daemon=True
        )
        thread.start()
        
        print(f"[API] Thread lancé pour {session_id}")
        
        # Retourner immédiatement
        return jsonify({
            "status": "processing",
            "session_id": session_id,
            "patient_id": patient_id,
            "message": "Capture en cours..."
        }), 202
    
    except Exception as e:
        print(f"[API] [ERROR] Erreur /api/capture: {e}")
        import traceback
        traceback.print_exc()
        return jsonify({"status": "error", "error": str(e)}), 500


@app.route('/api/results/<session_id>', methods=['GET'])
def api_results(session_id):
    """
    GET /api/results/<session_id>
    Retourne (format ESP32-compatible):
    - Si en cours: {"status": "processing"}
    - Si terminé: {"status": "completed", "oeil_1": {"classe": "Sain", "confiance": 0.95}, "oeil_2": {...}, "alerte": true/false}
    - Si erreur: {"status": "error", "error": "..."}
    """
    try:
        with sessions_lock:
            if session_id not in processing_sessions:
                print(f"[API] [ERROR] Session not found: {session_id}")
                return jsonify({
                    "status": "error",
                    "error": "Session not found"
                }), 404
            
            session_data = processing_sessions[session_id].copy()
        
        response = {"status": session_data["status"]}
        
        # Inclure le flag de capture terminee pour que ESP32 fasse un bip
        if session_data.get("capture_done"):
            response["capture_done"] = True
        
        if session_data["status"] == "completed":
            response["oeil_1"] = session_data.get("oeil_1")
            response["oeil_2"] = session_data.get("oeil_2")
            response["alerte"] = session_data.get("alerte", False)
        elif session_data["status"] == "error":
            response["error"] = session_data.get("error", "Unknown error")
        
        print(f"[API] [INFO] GET /api/results/{session_id} - Status: {session_data['status']}")
        
        return jsonify(response), 200
    
    except Exception as e:
        print(f"[API] [ERROR] Erreur /api/results: {e}")
        return jsonify({"status": "error", "error": str(e)}), 500


@app.route('/health', methods=['GET'])
def health_check():
    """
    GET /health
    Vérification simple que le serveur fonctionne
    """
    return jsonify({
        "status": "ok",
        "model_loaded": detector is not None,
        "sessions_count": len(processing_sessions)
    }), 200


@app.route('/play_audio/<int:audio_code>', methods=['GET'])
def play_audio_get(audio_code):
    """
    GET /play_audio/<code>
    Joue un fichier audio localement sur Raspberry Pi
    Codes synchronisés avec ESP32:
    - 1: AUDIO_BADGE (001.mp3)
    - 2: AUDIO_YEUX (002.mp3)
    - 3: AUDIO_RYTHME (003.mp3)
    - 4: AUDIO_TEMPERATURE (004.mp3)
    - 5: AUDIO_POIDS (005.mp3)
    - 6: AUDIO_FIN (006.mp3)
    """
    if play_audio_local(audio_code):
        return jsonify({
            "status": "ok",
            "audio_code": audio_code,
            "file": AUDIO_FILES.get(audio_code, "unknown")
        }), 200
    else:
        return jsonify({
            "status": "error",
            "message": f"Audio code {audio_code} non disponible"
        }), 400


@app.route('/play_audio', methods=['POST'])
def play_audio_post():
    """
    POST /play_audio
    Joue un fichier audio localement sur Raspberry Pi
    Format JSON (synchronisé avec ESP32): {"audio_code": 1}
    Codes disponibles: 1-6 (voir AUDIO_FILES)
    """
    try:
        data = request.get_json()
        
        if not data or 'audio_code' not in data:
            return jsonify({
                "status": "error",
                "message": "audio_code manquant"
            }), 400
        
        audio_code = int(data['audio_code'])
        
        if play_audio_local(audio_code):
            return jsonify({
                "status": "ok",
                "audio_code": audio_code,
                "file": AUDIO_FILES.get(audio_code, "unknown")
            }), 200
        else:
            return jsonify({
                "status": "error",
                "message": f"Audio code {audio_code} non disponible"
            }), 400
    
    except Exception as e:
        return jsonify({
            "status": "error",
            "message": str(e)
        }), 500


@app.route('/api/sessions', methods=['GET'])
def list_sessions():
    """
    GET /api/sessions
    Liste toutes les sessions (pour débogage)
    """
    with sessions_lock:
        sessions_list = []
        for session_id, data in processing_sessions.items():
            sessions_list.append({
                "session_id": session_id,
                "status": data.get("status"),
                "patient_id": data.get("patient_id"),
                "created": data.get("timestamp_created")
            })
    
    return jsonify({
        "total": len(sessions_list),
        "sessions": sessions_list
    }), 200


# ============================================================================
# NETTOYAGE DES ANCIENNES SESSIONS
# ============================================================================

def cleanup_old_sessions():
    """
    Nettoie les sessions terminées après 1 heure
    """
    while True:
        time.sleep(300)  # Vérification toutes les 5 minutes
        
        try:
            with sessions_lock:
                now = datetime.now()
                sessions_to_delete = []
                
                for session_id, data in processing_sessions.items():
                    if data.get("status") in ["completed", "error"]:
                        timestamp_str = data.get("timestamp_end") or data.get("timestamp_created")
                        if timestamp_str:
                            timestamp = datetime.fromisoformat(timestamp_str)
                            if (now - timestamp).total_seconds() > 3600:  # 1 heure
                                sessions_to_delete.append(session_id)
                
                for session_id in sessions_to_delete:
                    del processing_sessions[session_id]
                    print(f"[CLEANUP] [OK] Session {session_id} supprimee")
        
        except Exception as e:
            print(f"[CLEANUP] [ERROR] Erreur: {e}")


# ============================================================================
# DÉMARRAGE DU SERVEUR
# ============================================================================

if __name__ == '__main__':
    print("\n" + "="*70)
    print("SERVEUR FLASK - DETECTION OCULAIRE + AUDIO")
    print("="*70)
    print(f"Port: {PORT}")
    print(f"Modele charge: {'[OK]' if detector else '[ERROR]'}")
    print(f"Audio (pydub): {'[OK]' if AUDIO_AVAILABLE else '[DESACTIVES]'}")
    print("="*70 + "\n")
    
    # Créer le dossier resultat s'il n'existe pas
    os.makedirs('/home/mera/mera_detection/resultat', exist_ok=True)
    
    # Vérifier que le dossier audio existe
    if AUDIO_AVAILABLE and not os.path.exists(AUDIO_DIR):
        print(f"[AUDIO] [WARNING] Dossier audio non trouve: {AUDIO_DIR}")
    
    # Pré-charger les audios en mémoire (IMPORTANT pour latence zéro)
    if AUDIO_AVAILABLE:
        preload_audio_cache()
    
    # Lancer le thread de nettoyage des sessions
    cleanup_thread = threading.Thread(target=cleanup_old_sessions, daemon=True)
    cleanup_thread.start()
    
    # Démarrer le serveur Flask
    try:
        app.run(host='0.0.0.0', port=PORT, debug=DEBUG, use_reloader=False, threaded=True)
    except KeyboardInterrupt:
        print("\n[SERVER] [OK] Arret du serveur")
        sys.exit(0)
