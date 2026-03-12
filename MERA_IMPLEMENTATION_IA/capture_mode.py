#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Mode capture avec rpicam-vid - Prendre une photo quand les yeux sont bien détectés
Solution compatible Python 3.11 (ne dépend pas de libcamera Python binding)
"""

import os
os.environ['PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION'] = 'python'

import cv2
import numpy as np
import subprocess
import time
import os
from datetime import datetime


def start_capture_mode(detector, detect_eyes_func, CLASSES, COLORS):
    """
    Lancer le mode capture - Avec sauvegarde dans resultat/
    """
    
    print(f"\n{'='*70}")
    print("[CAMERA] MODE CAPTURE - DÉTECTION D'ANOMALIES OCULAIRES")
    print(f"{'='*70}\n")
    
    # Créer le dossier resultat/ s'il n'existe pas
    results_dir = "resultat"
    if not os.path.exists(results_dir):
        os.makedirs(results_dir)
        print(f"[INFO] Dossier '{results_dir}/' créé\n")
    
    # Capturer une photo avec rpicam-jpeg optimisé pour 5MP avec meilleure lumière
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    output_file = os.path.join(results_dir, f"capture_{timestamp}.jpg")
    
    # Nettoyer les anciennes captures (garder seulement les 5 dernières)
    try:
        import glob
        capture_files = sorted(glob.glob(os.path.join(results_dir, "capture_*.jpg")))
        if len(capture_files) > 5:
            for old_file in capture_files[:-5]:
                try:
                    os.remove(old_file)
                    print(f"[INFO] Suppression de l'ancienne capture: {os.path.basename(old_file)}")
                except:
                    pass
    except:
        pass
    
    try:
        # Configuration optimisée pour 5MP avec meilleure lumière et contraste
        # Résolution: 2560x1920 (5MP) pour meilleure qualité
        subprocess.run(
            [
                'rpicam-jpeg',
                '-o', output_file,
                '--width', '2560',
                '--height', '1920',
                '--quality', '90',
                '--brightness', '0.3',      # Augmente la luminosité +30%
                '--contrast', '1.5',        # Augmente le contraste pour plus de clarté
                '--saturation', '1.2',      # Légère augmentation de saturation
                '--awb', 'auto',            # Balance blanche automatique
                '--metering', 'centre',     # Mesure d'exposition centrée
                '--shutter', '20000',       # Augmente le temps d'exposition (microsecondes)
                '--gain', '2.5'             # Augmente le gain pour meilleure luminosité
            ],
            check=True,
            timeout=10
        )
        print(f"[OK] Photo capturee (5MP - 2560x1920) dans {output_file}\n")
    except FileNotFoundError:
        print("[ERROR] rpicam-jpeg non trouve. Installation de libcamera...")
        return
    except subprocess.TimeoutExpired:
        print("[ERROR] Timeout lors de la capture. Verifiez la camera.")
        return
    except Exception as e:
        print(f"[ERROR] Erreur lors de la capture: {str(e)[:60]}")
        return

    # Charger et analyser la photo capturée
    if not os.path.exists(output_file):
        print("[ERROR] Impossible de trouver la photo capturee.")
        return
    
    captured_frame = cv2.imread(output_file)
    if captured_frame is None:
        print("[ERROR] Impossible de charger la photo capturee.")
        return

    print("\n[INFO] ANALYSE DE LA PHOTO")
    print(f"{'='*70}\n")

    # Appeler la fonction de détection des yeux avec seulement l'image
    try:
        eyes = detect_eyes_func(captured_frame)
        
        if not eyes or len(eyes) < 2:
            print("[WARNING] Impossible de detecter les 2 yeux sur la photo capturee")
            return
        
        print(f"[OK] {len(eyes)} oeil(s) detecte(s)\n")
        
        predictions = []
        
        # Analyser les 2 premiers yeux
        for idx, eye_info in enumerate(eyes[:2]):
            ex, ey, ew, eh = eye_info['rect']
            eye_crop = captured_frame[ey:ey+eh, ex:ex+ew]
            
            if eye_crop.shape[0] < 15 or eye_crop.shape[1] < 15:
                continue
            
            try:
                pred_class, confidence, _ = detector.predict(eye_crop)
                class_name = CLASSES[pred_class]
                predictions.append({
                    'eye': idx + 1,
                    'class': class_name,
                    'confidence': confidence,
                    'pred_class': pred_class
                })
                
                print(f"Oeil {idx + 1}: {class_name} ({confidence*100:.1f}%)")
            
            except Exception as e:
                print(f"[ERROR] Erreur analyse oeil {idx + 1}: {str(e)[:60]}")
        
        # Resume
        if predictions:
            print(f"\n{'='*70}")
            healthy = sum(1 for p in predictions if p['class'] == "Sain")
            anomalies = len(predictions) - healthy
            
            if anomalies == 0:
                result_text = "SAIN"
                print(f"[OK] RESULTAT: Les deux yeux sont SAINS")
            else:
                result_text = "ANOMALIE"
                print(f"[WARNING] RESULTAT: {anomalies} anomalie(s) detectee(s)")
                for p in predictions:
                    if p['class'] != "Sain":
                        print(f"   - {p['class']} ({p['confidence']*100:.1f}%)")
            
            print(f"{'='*70}\n")
            
            # Sauvegarder la photo dans resultat/
            timestamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
            output_filename = f"{timestamp}_{result_text}.jpg"
            output_path = os.path.join(results_dir, output_filename)
            cv2.imwrite(output_path, captured_frame)
            print(f"[OK] Photo enregistree: {output_path}\n")
            
            # Creer un rapport texte
            report_filename = f"{timestamp}_{result_text}.txt"
            report_path = os.path.join(results_dir, report_filename)
            
            with open(report_path, 'w') as f:
                f.write(f"RAPPORT D'ANALYSE - {timestamp}\n")
                f.write(f"{'='*70}\n\n")
                f.write(f"RESULTAT: {result_text}\n")
                f.write(f"Yeux detectes: {len(predictions)}\n\n")
                
                for p in predictions:
                    f.write(f"Oeil {p['eye']}: {p['class']} ({p['confidence']*100:.1f}%)\n")
                
                f.write(f"\n{'='*70}\n")
            
            print(f"[OK] Rapport enregistre: {report_path}\n")
        else:
            print("[WARNING] Aucun diagnostic n'a pu etre etabli.\n")
    
    except Exception as e:
        print(f"[ERROR] Erreur lors de l'analyse: {str(e)[:60]}\n")

    print(f"\n[INFO] Resultats sauvegardes dans le dossier '{results_dir}/'\n")


def analyze_captured_photo(frame, detector, detect_eyes_func, CLASSES, COLORS, results_dir):
    """Analyser la photo capturée et enregistrer dans resultat/"""
    
    # Générer le timestamp
    timestamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
    
    # Détecter les yeux sur la photo
    eyes = detect_eyes_func(frame)
    
    if not eyes or len(eyes) < 2:
        print("⚠️  Impossible de détecter les 2 yeux sur la photo capturée")
        return
    
    print(f"✅ {len(eyes)} yeux détectés\n")
    
    predictions = []
    
    for idx, eye_info in enumerate(eyes[:2]):  # Analyser les 2 premiers yeux
        ex, ey, ew, eh = eye_info['rect']
        eye_crop = frame[ey:ey+eh, ex:ex+ew]
        
        if eye_crop.shape[0] < 15 or eye_crop.shape[1] < 15:
            continue
        
        try:
            pred_class, confidence, _ = detector.predict(eye_crop)
            class_name = CLASSES[pred_class]
            predictions.append({
                'eye': idx + 1,
                'class': class_name,
                'confidence': confidence,
                'pred_class': pred_class
            })
            
            print(f"Œil {idx + 1}: {class_name} ({confidence*100:.1f}%)")
        
        except Exception as e:
            print(f"❌ Erreur analyse œil {idx + 1}: {e}")
    
    # Résumé
    if predictions:
        print(f"\n{'='*70}")
        healthy = sum(1 for p in predictions if p['class'] == "Sain")
        anomalies = len(predictions) - healthy
        
        if anomalies == 0:
            result_text = "SAIN"
            result_icon = "✅"
            print(f"{result_icon} RÉSULTAT: Les deux yeux sont SAINS")
        else:
            result_text = "ANOMALIE"
            result_icon = "⚠️ "
            print(f"{result_icon} RÉSULTAT: {anomalies} anomalie(s) détectée(s)")
            for p in predictions:
                if p['class'] != "Sain":
                    print(f"   - {p['class']} ({p['confidence']*100:.1f}%)")
        
        print(f"{'='*70}\n")
        
        # Sauvegarder la photo dans resultat/
        output_filename = f"{timestamp}_{result_text}.jpg"
        output_path = os.path.join(results_dir, output_filename)
        cv2.imwrite(output_path, frame)
        print(f"📁 Photo enregistrée: {output_path}\n")
        
        # Créer un rapport texte
        report_filename = f"{timestamp}_{result_text}.txt"
        report_path = os.path.join(results_dir, report_filename)
        
        with open(report_path, 'w') as f:
            f.write(f"RAPPORT D'ANALYSE - {timestamp}\n")
            f.write(f"{'='*70}\n\n")
            f.write(f"RÉSULTAT: {result_text}\n")
            f.write(f"Yeux détectés: {len(predictions)}\n\n")
            
            for p in predictions:
                f.write(f"Œil {p['eye']}: {p['class']} ({p['confidence']*100:.1f}%)\n")
            
            f.write(f"\n{'='*70}\n")
        
        print(f"📄 Rapport enregistré: {report_path}\n")
