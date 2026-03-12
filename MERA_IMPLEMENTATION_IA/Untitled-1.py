#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Système Complet de Détection d'Anomalies Oculaires
Support: Images, Vidéos, Webcam, Batch Processing
"""

import os
import sys

# IMPORTANT: Définir la variable pour éviter les conflits protobuf/mindspore
os.environ['PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION'] = 'python'
# Importation compatible mediapipe (legacy ou tasks)
# Mediapipe FaceLandmarker (API tasks >=0.10)
from mediapipe.tasks.python.vision import FaceLandmarker, FaceLandmarkerOptions
from mediapipe.tasks.python.vision.face_landmarker import FaceLandmarkerResult
from mediapipe import Image, ImageFormat
from mediapipe.tasks.python import BaseOptions, vision

# Configurer OpenCV pour utiliser le backend X11 (xcb)
os.environ['QT_QPA_PLATFORM'] = 'xcb'

import cv2
import numpy as np
from PIL import Image as PILImage, ImageDraw, ImageFont
import mindspore as ms
from mindspore import context, Tensor
import time
import glob
from datetime import datetime
import json

# Configuration
context.set_context(mode=context.GRAPH_MODE, device_target="CPU")

# Paramètres
IMAGE_SIZE = 224
NUM_CLASSES = 7
CONFIDENCE_THRESHOLD = 0.60  # Seuil de confiance minimum

CLASSES = {
    0: "Sain",
    1: "Cataracte",
    2: "Conjunctivite",
    3: "Jaunisse",
    4: "Pterygion",
    5: "Blepharite",
    6: "Uveitis"
}

# Couleurs pour l'affichage (BGR pour OpenCV)
COLORS = {
    0: (0, 255, 0),      # Vert - Sain
    1: (255, 165, 0),    # Orange - Cataracte
    2: (0, 255, 255),    # Cyan - Conjunctivite
    3: (0, 255, 255),    # Jaune - Jaunisse
    4: (255, 0, 255),    # Magenta - Pterygion
    5: (128, 0, 128),    # Violet - Blepharite
    6: (0, 0, 255),      # Rouge - Uveitis
}

# Fonction pour afficher avec gestion des erreurs
def safe_print(message):
    """Afficher un message en gérant les erreurs d'encodage Unicode"""
    try:
        print(message.encode('utf-8').decode('utf-8'))  # Ensure proper encoding and decoding
    except UnicodeEncodeError:
        # Fallback: remplacer les caractères spéciaux
        import unicodedata
        safe_msg = ''.join(c if ord(c) < 128 else '?' for c in message)
        print(safe_msg)  # Use the built-in print function

def safe_imshow(window_name, image):
    """Afficher une image avec gestion des erreurs d'affichage"""
    try:
        cv2.imshow(window_name, image)
    except Exception as e:
        safe_print(f"WARNING: Cannot display window: {e}")
        return False
    return True

# Importer l'architecture
try:
    from train_fixed import EyeDiseaseNet
except ImportError:
    safe_print("ERROR: train_fixed.py not found!")
    safe_print("   Make sure the file is in the same directory.")
    sys.exit(1)


def calculate_dynamic_padding(eye_width, eye_height, image_width, image_height, object_area_ratio=None):
    """
    Calcule un padding dynamique basé sur la taille de l'objet détecté.
    
    Args:
        eye_width: Largeur du bounding box de l'œil
        eye_height: Hauteur du bounding box de l'œil
        image_width: Largeur totale de l'image
        image_height: Hauteur totale de l'image
        object_area_ratio: Ratio entre la surface de l'objet et celle de l'image (0-1)
        
    Returns:
        padding: Valeur de padding en pixels (dynamique)
    """
    # Stratégie 1: Basée sur la taille de l'objet
    object_size = max(eye_width, eye_height)
    
    # Stratégie 2: Si on a le ratio de surface, on l'utilise
    if object_area_ratio is not None:
        if object_area_ratio > 0.3:  # Gros plan (>30% de l'image)
            padding = int(object_size * 0.25)  # 25% de la taille de l'objet
        elif object_area_ratio > 0.1:  # Plan moyen (10-30%)
            padding = int(object_size * 0.35)  # 35% de la taille de l'objet
        elif object_area_ratio > 0.02:  # Plan éloigné (2-10%)
            padding = int(object_size * 0.5)   # 50% de la taille de l'objet
        else:  # Plan très éloigné (<2%)
            padding = int(object_size * 0.75)  # 75% de la taille de l'objet
    else:
        # Fallback: si pas de ratio, on estime basé sur la taille absolue
        if object_size > 150:  # Gros plan
            padding = int(object_size * 0.25)
        elif object_size > 80:  # Plan moyen
            padding = int(object_size * 0.35)
        elif object_size > 40:  # Plan éloigné
            padding = int(object_size * 0.5)
        else:  # Plan très éloigné
            padding = int(object_size * 0.75)
    
    # Limites de sécurité : padding entre 10 et 120 pixels
    padding = max(10, min(120, padding))
    
    return padding


def calculate_smart_dynamic_padding(eye_width, eye_height, image_width, image_height, 
                                   face_width=None, confidence=0.9, is_horizontal=True):
    """
    Calcule un padding ULTRA-intelligent avec plusieurs facteurs.
    
    Args:
        eye_width, eye_height: Dimensions du bounding box de l'œil
        image_width, image_height: Dimensions de l'image
        face_width: Largeur du visage détecté (pour estimer la distance)
        confidence: Confiance de détection (0-1)
        is_horizontal: True si l'œil est plus large que haut
        
    Returns:
        {
            'padding': padding_uniforme,
            'padding_h': padding horizontal spécifique,
            'padding_v': padding vertical spécifique,
            'asymmetric': True/False
        }
    """
    
    object_size = max(eye_width, eye_height)
    area_ratio = (eye_width * eye_height) / (image_width * image_height)
    
    # FACTEUR 1: Distance estimée via taille du visage
    if face_width is not None:
        # Ratio œil/visage indique la distance
        eye_face_ratio = eye_width / max(face_width, 1)
        
        if eye_face_ratio > 0.4:  # Très gros plan (proche)
            base_padding = int(object_size * 0.2)
        elif eye_face_ratio > 0.25:  # Gros plan
            base_padding = int(object_size * 0.3)
        elif eye_face_ratio > 0.15:  # Plan moyen
            base_padding = int(object_size * 0.4)
        elif eye_face_ratio > 0.08:  # Plan éloigné
            base_padding = int(object_size * 0.55)
        else:  # Très éloigné
            base_padding = int(object_size * 0.75)
    else:
        # Fallback sur area_ratio
        if area_ratio > 0.3:
            base_padding = int(object_size * 0.25)
        elif area_ratio > 0.1:
            base_padding = int(object_size * 0.35)
        elif area_ratio > 0.02:
            base_padding = int(object_size * 0.5)
        else:
            base_padding = int(object_size * 0.75)
    
    # FACTEUR 2: Ajuster selon la confiance
    # Moins confiant = plus de contexte
    confidence_factor = 1.0 + (1.0 - confidence) * 0.4  # +40% si confiance basse
    base_padding = int(base_padding * confidence_factor)
    
    # FACTEUR 3: Padding asymétrique intelligent
    # Plus de padding horizontal si œil étendu horizontalement
    if is_horizontal and eye_width > eye_height * 1.2:
        # Œil large : plus de contexte sur les côtés
        padding_h = int(base_padding * 1.3)
        padding_v = int(base_padding * 0.8)
        use_asymmetric = True
    elif not is_horizontal and eye_height > eye_width * 1.2:
        # Œil allongé verticalement (rare mais possible)
        padding_h = int(base_padding * 0.8)
        padding_v = int(base_padding * 1.2)
        use_asymmetric = True
    else:
        # Œil carré : padding uniforme
        padding_h = base_padding
        padding_v = base_padding
        use_asymmetric = False
    
    # Limites de sécurité
    base_padding = max(8, min(120, base_padding))
    padding_h = max(8, min(150, padding_h))
    padding_v = max(8, min(150, padding_v))
    
    return {
        'padding': base_padding,
        'padding_h': padding_h,
        'padding_v': padding_v,
        'asymmetric': use_asymmetric,
        'confidence_boost': confidence_factor
    }


def apply_padding_with_context(x, y, width, height, padding, image_width, image_height):
    """
    Applique le padding avec gestion intelligente des bordures.
    Retourne les coordonnées du rectangle paddé avec contexte.
    
    Args:
        x, y: Position du coin supérieur gauche
        width, height: Dimensions du bounding box original
        padding: Valeur de padding (int ou dict avec 'padding_h' et 'padding_v')
        image_width, image_height: Dimensions de l'image
        
    Returns:
        (x1, y1, x2, y2): Coordonnées paddées
    """
    # Gérer padding simple ou asymétrique
    if isinstance(padding, dict):
        pad_h = padding.get('padding_h', padding.get('padding', 20))
        pad_v = padding.get('padding_v', padding.get('padding', 20))
    else:
        pad_h = pad_v = padding
    
    # Appliquer le padding
    x1 = max(0, x - pad_h)
    y1 = max(0, y - pad_v)
    x2 = min(image_width, x + width + pad_h)
    y2 = min(image_height, y + height + pad_v)
    
    # Si on dépasse les limites, redistribuer le padding
    if x1 == 0 and x + width + pad_h > image_width:
        x2 = min(image_width, x2 + (pad_h - x))
    if y1 == 0 and y + height + pad_v > image_height:
        y2 = min(image_height, y2 + (pad_v - y))
    if x2 == image_width and x - pad_h < 0:
        x1 = max(0, x1 - (pad_h - (image_width - x - width)))
    if y2 == image_height and y - pad_v < 0:
        y1 = max(0, y1 - (pad_v - (image_height - y - height)))
    
    return (x1, y1, x2 - x1, y2 - y1)  # Retourne (x, y, w, h)


def detect_eyes_hybrid(image, detector_cascade=None):
    """
    Détection hybride des yeux : FaceLandmarker + Haar Cascades
    Retourne une liste de rectangles (x, y, w, h) pour les yeux détectés
    """
    h, w, _ = image.shape
    eyes = []
    
    # Étape 1 : Essayer FaceLandmarker avec seuils très bas
    try:
        model_path = "face_landmarker.task"
        if os.path.exists(model_path):
            options = FaceLandmarkerOptions(
                base_options=BaseOptions(model_asset_path=model_path),
                output_face_blendshapes=False,
                output_facial_transformation_matrixes=False,
                num_faces=1,
                min_face_detection_confidence=0.2,  # très tolérant
                min_face_presence_confidence=0.2,
                min_tracking_confidence=0.2
            )
            landmarker = FaceLandmarker.create_from_options(options)
            frame_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
            mp_image = Image(image_format=ImageFormat.SRGB, data=frame_rgb)
            result = landmarker.detect(mp_image)
            
            if result.face_landmarks:
                for landmarks in result.face_landmarks:
                    LEFT_EYE_IDX = [33, 133, 160, 159, 158, 157, 173, 246]
                    RIGHT_EYE_IDX = [362, 263, 387, 386, 385, 384, 398, 466]
                    
                    left_eye_pts = np.array([(int(landmarks[i].x * w), int(landmarks[i].y * h)) for i in LEFT_EYE_IDX])
                    right_eye_pts = np.array([(int(landmarks[i].x * w), int(landmarks[i].y * h)) for i in RIGHT_EYE_IDX])
                    
                    lx, ly, lw, lh = cv2.boundingRect(left_eye_pts)
                    rx, ry, rw, rh = cv2.boundingRect(right_eye_pts)
                    
                    # Calculer la taille du visage pour estimer la distance
                    face_pts = np.concatenate([left_eye_pts, right_eye_pts])
                    face_bbox = cv2.boundingRect(face_pts)
                    face_width = face_bbox[2]
                    
                    # Utiliser le padding SMART avec confiance élevée (MediaPipe = très bon)
                    left_padding_dict = calculate_smart_dynamic_padding(
                        lw, lh, w, h, 
                        face_width=face_width,
                        confidence=0.95,  # MediaPipe = confiance très haute
                        is_horizontal=True
                    )
                    right_padding_dict = calculate_smart_dynamic_padding(
                        rw, rh, w, h,
                        face_width=face_width,
                        confidence=0.95,
                        is_horizontal=True
                    )
                    
                    # Appliquer le padding avec gestion intelligente des bordures
                    left_rect = apply_padding_with_context(lx, ly, lw, lh, left_padding_dict, w, h)
                    right_rect = apply_padding_with_context(rx, ry, rw, rh, right_padding_dict, w, h)
                    
                    eyes.append({
                        'rect': left_rect,
                        'pts': left_eye_pts,
                        'type': 'mediapipe',
                        'padding': left_padding_dict,
                        'confidence': 0.95
                    })
                    eyes.append({
                        'rect': right_rect,
                        'pts': right_eye_pts,
                        'type': 'mediapipe',
                        'padding': right_padding_dict,
                        'confidence': 0.95
                    })
                    return eyes
    except Exception as e:
        pass
    
    # Étape 2 : Fallback Haar Cascades (très robuste)
    try:
        if detector_cascade is None:
            cascade_path = cv2.data.haarcascades + 'haarcascade_eye.xml'
            detector_cascade = cv2.CascadeClassifier(cascade_path)
        
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        # Détection avec paramètres optimisés
        detected_eyes = detector_cascade.detectMultiScale(
            gray,
            scaleFactor=1.1,
            minNeighbors=8,
            minSize=(30, 30),
            maxSize=(150, 150)
        )
        
        if len(detected_eyes) > 0:
            h, w, _ = image.shape
            for (x, y, w_eye, h_eye) in detected_eyes[:2]:  # Max 2 yeux
                # Haar Cascade = confiance modérée (moins fiable que MediaPipe)
                padding_dict = calculate_smart_dynamic_padding(
                    w_eye, h_eye, w, h,
                    face_width=None,  # Pas de visage détecté ici
                    confidence=0.75,  # Confiance moyenne pour Haar
                    is_horizontal=True
                )
                
                # Appliquer le padding avec gestion intelligente
                padded_rect = apply_padding_with_context(x, y, w_eye, h_eye, padding_dict, w, h)
                
                eyes.append({
                    'rect': padded_rect,
                    'pts': None,
                    'type': 'haar',
                    'padding': padding_dict,
                    'confidence': 0.75
                })
            return eyes
    except Exception as e:
        pass
    
    return eyes


class EyeDetector:
    """Classe pour la détection d'anomalies oculaires"""
    
    def __init__(self, model_path):
        """Initialiser le détecteur"""
        safe_print("Loading models...")
        
        if not os.path.exists(model_path):
            raise FileNotFoundError(f"Classification model not found: {model_path}")
            
        # Charger le modèle de classification
        self.network = EyeDiseaseNet(num_classes=NUM_CLASSES, pretrained=False)
        ms.load_checkpoint(model_path, self.network)
        self.network.set_train(False)
        safe_print("OK: Classification model loaded.")

        # Charger les modèles de détection d'yeux (Haar Cascades) - OPTIONNEL
        face_cascade_path = '/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml'
        eye_cascade_path = '/usr/share/opencv4/haarcascades/haarcascade_eye.xml'
        
        self.face_cascade = None
        self.eye_cascade = None
        
        if os.path.exists(face_cascade_path) and os.path.exists(eye_cascade_path):
            self.face_cascade = cv2.CascadeClassifier(face_cascade_path)
            self.eye_cascade = cv2.CascadeClassifier(eye_cascade_path)
            safe_print("OK: Haar Cascade models loaded.")
        else:
            safe_print("WARNING: Haar Cascades not found (fallback MediaPipe only)")
        
        safe_print("OK: Detector ready!\n")
    
    def preprocess_image(self, image):
        """Prétraiter l'image pour le modèle"""
        # Convertir en RGB si nécessaire
        if len(image.shape) == 2:
            image = cv2.cvtColor(image, cv2.COLOR_GRAY2RGB)
        elif image.shape[2] == 4:
            image = cv2.cvtColor(image, cv2.COLOR_BGRA2RGB)
        else:
            image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
        
        # Redimensionner
        img = cv2.resize(image, (IMAGE_SIZE, IMAGE_SIZE))
        img = img.astype(np.float32) / 255.0
        
        # Normalisation
        mean = np.array([0.485, 0.456, 0.406]).reshape((1, 1, 3))
        std = np.array([0.229, 0.224, 0.225]).reshape((1, 1, 3))
        img = (img - mean) / std
        
        # Transposer HWC -> CHW et ajouter batch
        img = img.transpose(2, 0, 1)
        img = np.expand_dims(img, axis=0)
        
        return img
    
    def predict(self, image):
        """Faire une prédiction sur une image"""
        # Prétraiter
        img_tensor = self.preprocess_image(image)
        tensor_img = Tensor(img_tensor, ms.float32)
        
        # Prédiction
        output = self.network(tensor_img)
        pred = output.asnumpy()[0]
        
        # Calculer les probabilités (softmax)
        probs = np.exp(pred) / np.sum(np.exp(pred))
        pred_class = np.argmax(probs)
        confidence = probs[pred_class]
        
        return pred_class, confidence, probs
    
    def draw_prediction(self, image, pred_class, confidence, probs):
        """Dessiner les prédictions sur l'image"""
        h, w = image.shape[:2]
        
        # Créer une copie
        img_display = image.copy()
        
        # Préparer le texte
        class_name = CLASSES[pred_class]
        color = COLORS[pred_class]
        
        # Zone d'information en haut
        overlay = img_display.copy()
        cv2.rectangle(overlay, (0, 0), (w, 120), (0, 0, 0), -1)
        cv2.addWeighted(overlay, 0.7, img_display, 0.3, 0, img_display)
        
        # Texte principal
        status = "✓ DÉTECTÉ" if confidence >= CONFIDENCE_THRESHOLD else "⚠ INCERTAIN"
        text_main = f"{status}: {class_name}"
        text_conf = f"Confiance: {confidence*100:.1f}%"
        
        cv2.putText(img_display, text_main, (10, 40),
                    cv2.FONT_HERSHEY_SIMPLEX, 1.0, color, 2)
        cv2.putText(img_display, text_conf, (10, 80),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.8, (255, 255, 255), 2)
        
        # Barre de probabilités sur le côté
        bar_x = w - 250
        bar_y_start = 140
        
        cv2.rectangle(img_display, (bar_x-10, bar_y_start-10), 
                     (w-10, h-10), (0, 0, 0), -1)
        cv2.addWeighted(img_display[bar_y_start-10:h-10, bar_x-10:w-10],
                       0.7, image[bar_y_start-10:h-10, bar_x-10:w-10], 0.3, 0,
                       img_display[bar_y_start-10:h-10, bar_x-10:w-10])
        
        # Titre
        cv2.putText(img_display, "Probabilites", (bar_x, bar_y_start-20),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.6, (255, 255, 255), 2)
        
        # Barres pour chaque classe
        sorted_indices = np.argsort(probs)[::-1]
        for i, idx in enumerate(sorted_indices):
            prob = probs[idx]
            name = CLASSES[idx]
            bar_color = COLORS[idx]
            
            y_pos = bar_y_start + i * 50
            
            # Nom de la classe
            cv2.putText(img_display, name[:12], (bar_x, y_pos + 15),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 1)
            
            # Barre de progression
            bar_width = int(prob * 200)
            cv2.rectangle(img_display, (bar_x, y_pos + 20), 
                         (bar_x + bar_width, y_pos + 35), bar_color, -1)
            
            # Pourcentage
            cv2.putText(img_display, f"{prob*100:.1f}%", (bar_x + 205, y_pos + 33),
                       cv2.FONT_HERSHEY_SIMPLEX, 0.4, (255, 255, 255), 1)
        
        return img_display


def test_single_image(detector, image_path, save_result=True):
    """Tester une seule image avec détection hybride robuste."""
    safe_print(f"\n{'='*70}")
    safe_print(f"TEST IMAGE AVEC DÉTECTION D'YEUX: {os.path.basename(image_path)}")
    safe_print(f"{'='*70}\n")
    
    image = cv2.imread(image_path)
    if image is None:
        safe_print(f"[ERROR] Impossible de charger l'image: {image_path}")
        return
    
    frame_result = image.copy()
    all_predictions = []
    h, w, _ = image.shape
    
    # Utiliser détection hybride (MediaPipe + Haar Cascades)
    eyes = detect_eyes_hybrid(image)
    
    if eyes:
        safe_print(f"[INFO] {len(eyes)} oeil(s) detecte(s).")
        for i, eye_info in enumerate(eyes, 1):
            ex, ey, ew, eh = eye_info['rect']
            eye_pts = eye_info['pts']
            detection_type = eye_info['type']
            
            # Extraire la région
            eye_crop = image[ey:ey+eh, ex:ex+ew]
            if eye_crop.shape[0] < 15 or eye_crop.shape[1] < 15:
                continue
            
            try:
                pred_class, confidence, _ = detector.predict(eye_crop)
                class_name = CLASSES[pred_class]
                color = COLORS[pred_class]
                all_predictions.append({'class': class_name, 'confidence': float(confidence)})
                
                # Encadrer l'œil avec un rectangle solide et épais
                cv2.rectangle(frame_result, (ex, ey), (ex+ew, ey+eh), color, 3)
                
                # Ajouter un texte avec fond
                text = f"{class_name} ({confidence*100:.0f}%)"
                (text_width, text_height), baseline = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, 0.7, 2)
                cv2.rectangle(frame_result, (ex, ey - text_height - baseline - 8), 
                             (ex + text_width + 8, ey - 5), color, -1)
                cv2.putText(frame_result, text, (ex + 4, ey - 10), cv2.FONT_HERSHEY_SIMPLEX, 
                           0.7, (0, 0, 0), 2)
                
                # Ajouter un point au centre de l'œil
                center_x = ex + ew // 2
                center_y = ey + eh // 2
                cv2.circle(frame_result, (center_x, center_y), 5, color, -1)
                cv2.circle(frame_result, (center_x, center_y), 5, (0, 0, 0), 2)
                
                safe_print(f"   Œil {i} ({detection_type}) -> {class_name} (Confiance: {confidence*100:.1f}%)")
            except Exception as e:
                safe_print(f"   [WARNING] Oeil {i}: Erreur de classification")
                continue
    else:
        safe_print("[WARNING] Aucun oeil detecte. Classification de l'image entiere...")
        try:
            if image.size > 0:
                pred_class, confidence, _ = detector.predict(image)
                class_name = CLASSES[pred_class]
                color = COLORS[pred_class]
                all_predictions.append({'class': class_name, 'confidence': float(confidence), 'fallback': True})
                
                # Affichage sur l'image entière
                text = f"Diagnostic global: {class_name} ({confidence*100:.0f}%)"
                (text_width, text_height), baseline = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, 0.9, 3)
                cv2.rectangle(frame_result, (15, 50), (15 + text_width + 10, 50 + text_height + baseline + 10), 
                             color, -1)
                cv2.putText(frame_result, text, (20, 50 + text_height), cv2.FONT_HERSHEY_SIMPLEX, 0.9, 
                           (0, 0, 0), 3)
                
                safe_print(f"   Diagnostic global: {class_name} (Confiance: {confidence*100:.1f}%)")
        except Exception as e:
            safe_print(f"[ERROR] Erreur de classification: {str(e)[:60]}")
    
    if all_predictions:
        fallback = all_predictions[0].get('fallback', False)
        if not fallback:
            safe_print(f"\n[OK] {len(all_predictions)} resultat(s) disponible(s)")
    else:
        safe_print("[ERROR] Aucun diagnostic n'a pu etre etabli.")
    
    if save_result:
        output_dir = "./results/images"
        os.makedirs(output_dir, exist_ok=True)
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        output_path = os.path.join(output_dir, f"result_{timestamp}.jpg")
        cv2.imwrite(output_path, frame_result)
        safe_print(f"[OK] Resultat sauvegarde: {output_path}")


def interactive_menu(detector):
    """Menu interactif"""
    while True:
        safe_print("\n" + "="*70)
        safe_print("[MEDICAL] OCULAR DISEASE DETECTION SYSTEM")
        safe_print("="*70)
        safe_print("\nChoose an option:")
        safe_print("  1. Test image")
        safe_print("  2. Test video")
        safe_print("  3. Test with webcam")
        safe_print("  4. Batch processing (image folder)")
        safe_print("  5. Quit")
        safe_print("-"*70)
        
        choice = input("Your choice: ").strip()
        
        if choice == "1":
            path = input("\nImage path: ").strip()
            if os.path.exists(path):
                test_single_image(detector, path)
            else:
                safe_print(f"ERROR: File not found: {path}")
        elif choice == "2":
            path = input("\nVideo path: ").strip()
            if os.path.exists(path):
                test_single_video(detector, path)
            else:
                safe_print(f"ERROR: File not found: {path}")
        elif choice == "3":
            test_webcam(detector)
        elif choice == "4":
            path = input("\nFolder path: ").strip()
            if os.path.isdir(path):
                batch_process_directory(detector, path)
            else:
                safe_print(f"ERROR: Folder not found: {path}")
        elif choice == "5":
            safe_print("\nGoodbye!")
            break
        else:
            safe_print("ERROR: Invalid choice")


def test_single_video(detector, video_path, save_result=True):
    """Tester sur une vidéo avec détection hybride robuste."""
    safe_print(f"\n{'='*70}")
    safe_print(f"TEST VIDÉO: {os.path.basename(video_path)}")
    safe_print(f"{'='*70}\n")
    
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        safe_print(f"[ERROR] Impossible d'ouvrir la video: {video_path}")
        return
    
    fps = int(cap.get(cv2.CAP_PROP_FPS))
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    
    safe_print(f"Résolution: {width}x{height}, FPS: {fps}, Total frames: {total_frames}\n")
    
    # Initialiser le writer vidéo
    output_path = None
    if save_result:
        os.makedirs("./results/videos", exist_ok=True)
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        output_path = f"./results/videos/result_video_{timestamp}.mp4"
        fourcc = cv2.VideoWriter_fourcc(*'mp4v')
        out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))
    
    frame_count = 0
    predictions_history = []
    pred_counts = {}
    fps_history = []
    
    safe_print("[INFO] Traitement en cours...")
    
    while True:
        start_time = time.time()
        ret, frame = cap.read()
        if not ret:
            break
        
        frame_result = frame.copy()
        
        # Utiliser détection hybride
        eyes = detect_eyes_hybrid(frame)
        
        if eyes:
            for eye_info in eyes:
                ex, ey, ew, eh = eye_info['rect']
                eye_crop = frame[ey:ey+eh, ex:ex+ew]
                
                if eye_crop.shape[0] < 15 or eye_crop.shape[1] < 15:
                    continue
                
                try:
                    pred_class, confidence, _ = detector.predict(eye_crop)
                    class_name = CLASSES[pred_class]
                    color = COLORS[pred_class]
                    predictions_history.append(class_name)
                    pred_counts[class_name] = pred_counts.get(class_name, 0) + 1
                    
                    # Encadrer l'œil
                    cv2.rectangle(frame_result, (ex, ey), (ex+ew, ey+eh), color, 3)
                    text = f"{class_name}: {confidence*100:.0f}%"
                    (text_width, text_height), baseline = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, 0.7, 2)
                    cv2.rectangle(frame_result, (ex, ey - text_height - baseline - 8), 
                                 (ex + text_width + 8, ey - 5), color, -1)
                    cv2.putText(frame_result, text, (ex + 4, ey - 10), cv2.FONT_HERSHEY_SIMPLEX, 
                               0.7, (0, 0, 0), 2)
                except Exception:
                    continue
        
        # FPS
        fps_val = 1.0 / (time.time() - start_time + 1e-6)
        fps_history.append(fps_val)
        if len(fps_history) > 30:
            fps_history.pop(0)
        avg_fps = np.mean(fps_history)
        
        cv2.putText(frame_result, f"FPS: {avg_fps:.1f}", (10, frame.shape[0] - 20),
                   cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 0), 2)
        
        if output_path:
            out.write(frame_result)
        
        frame_count += 1
        if frame_count % max(1, total_frames // 10) == 0:
            safe_print(f"  Frame {frame_count}/{total_frames} traité(e)s...")
    
    cap.release()
    if output_path:
        out.release()
    
    if predictions_history:
        safe_print(f"\n[SUMMARY] Resume de la video:")
        for pred_class, count in sorted(pred_counts.items(), key=lambda x: x[1], reverse=True):
            percentage = (count / len(predictions_history)) * 100
            safe_print(f"   {pred_class:<15}: {count:4d} frames ({percentage:5.1f}%)")
        
        dominant_class = max(pred_counts.items(), key=lambda x: x[1])
        safe_print(f"\n[RESULT] Diagnostic dominant: {dominant_class[0]}")
    else:
        safe_print("[WARNING] Aucun oeil detecte dans la video.")
    
    if output_path:
        safe_print(f"\n[OK] Video sauvegardee: {output_path}")


def batch_process_directory(detector, input_dir, output_dir="./results_batch", save_result=True):
    """Traitement batch d'un dossier d'images avec FaceLandmarker."""
    safe_print(f"\n{'='*70}")
    safe_print(f"TRAITEMENT BATCH: {os.path.basename(input_dir)}")
    safe_print(f"{'='*70}\n")
    
    # Lister les images
    image_extensions = ('.jpg', '.jpeg', '.png', '.bmp', '.tiff')
    image_files = [f for f in os.listdir(input_dir) 
                   if os.path.isfile(os.path.join(input_dir, f)) and f.lower().endswith(image_extensions)]
    
    if not image_files:
        safe_print(f"❌ Aucune image trouvée dans {input_dir}")
        return
    
    safe_print(f"📊 {len(image_files)} image(s) trouvée(s)")
    safe_print(f"⚡ Traitement en cours...\n")
    
    os.makedirs(output_dir, exist_ok=True)
    all_predictions = {}
    
    for i, image_file in enumerate(image_files, 1):
        image_path = os.path.join(input_dir, image_file)
        image = cv2.imread(image_path)
        
        if image is None:
            safe_print(f"⏭️  Skipped: {image_file} (impossible de charger)")
            continue
        
        frame_result = image.copy()
        h, w, _ = image.shape
        predictions_count = 0
        fallback = False
        
        # Utiliser détection hybride
        eyes = detect_eyes_hybrid(image)
        
        if eyes:
            for eye_info in eyes:
                ex, ey, ew, eh = eye_info['rect']
                eye_crop = image[ey:ey+eh, ex:ex+ew]
                
                if eye_crop.shape[0] < 15 or eye_crop.shape[1] < 15:
                    continue
                
                try:
                    pred_class, confidence, _ = detector.predict(eye_crop)
                    class_name = CLASSES[pred_class]
                    color = COLORS[pred_class]
                    predictions_count += 1
                    all_predictions[image_file] = class_name
                    
                    # Encadrer l'œil
                    cv2.rectangle(frame_result, (ex, ey), (ex+ew, ey+eh), color, 3)
                    text = f"{class_name}: {confidence*100:.0f}%"
                    (text_width, text_height), baseline = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, 0.7, 2)
                    cv2.rectangle(frame_result, (ex, ey - text_height - baseline - 8), 
                                 (ex + text_width + 8, ey - 5), color, -1)
                    cv2.putText(frame_result, text, (ex + 4, ey - 10), cv2.FONT_HERSHEY_SIMPLEX, 
                               0.7, (0, 0, 0), 2)
                except Exception:
                    continue
        else:
            # Fallback : classer l'image entière
            try:
                pred_class, confidence, _ = detector.predict(image)
                class_name = CLASSES[pred_class]
                color = COLORS[pred_class]
                fallback = True
                all_predictions[image_file] = class_name
                text = f"{class_name}: {confidence*100:.0f}% (entière)"
                (text_width, text_height), baseline = cv2.getTextSize(text, cv2.FONT_HERSHEY_SIMPLEX, 0.8, 2)
                cv2.rectangle(frame_result, (10, 40), (10 + text_width + 8, 40 + text_height + baseline + 8), 
                             color, -1)
                cv2.putText(frame_result, text, (14, 40 + text_height), cv2.FONT_HERSHEY_SIMPLEX, 0.8, 
                           (0, 0, 0), 2)
            except Exception:
                pass
        
        # Sauvegarder le résultat
        if save_result:
            output_path = os.path.join(output_dir, f"result_{i:03d}_{image_file}")
            cv2.imwrite(output_path, frame_result)
        
        status = "✅" if predictions_count > 0 or fallback else "⚠️"
        if predictions_count > 0:
            safe_print(f"  {status} [{i}/{len(image_files)}] {image_file}: {predictions_count} œil(s)")
        elif fallback:
            safe_print(f"  {status} [{i}/{len(image_files)}] {image_file}: fallback (image entière)")
        else:
            safe_print(f"  {status} [{i}/{len(image_files)}] {image_file}: aucun diagnostic")
    
    # Résumé
    safe_print(f"\n{'='*70}")
    safe_print(f"🎯 RÉSUMÉ BATCH")
    safe_print(f"{'='*70}")
    safe_print(f"✅ {len(image_files)} image(s) traitée(s)")
    safe_print(f"📁 Résultats sauvegardés dans: {output_dir}")
    
    if all_predictions:
        from collections import Counter
        counts = Counter(all_predictions.values())
        safe_print(f"\n📊 Diagnostics:")
        for class_name, count in counts.most_common():
            percentage = (count / len(all_predictions)) * 100
            safe_print(f"   {class_name:<15}: {count:3d} images ({percentage:5.1f}%)")


def test_webcam(detector):
    """Lancer le mode capture avec picamzero"""
    from capture_mode import start_capture_mode
    start_capture_mode(detector, detect_eyes_hybrid, CLASSES, COLORS)


def main():
    """Fonction principale"""
    try:
        safe_print("\n" + "="*70)
        safe_print("🏥 SYSTÈME DE DÉTECTION D'ANOMALIES OCULAIRES")
        safe_print("="*70 + "\n")
    except UnicodeEncodeError:
        # Fallback si locale ne supporte pas UTF-8
        safe_print("\n" + "="*70)
        safe_print("[MEDICAL] OCULAR DISEASE DETECTION SYSTEM")
        safe_print("="*70 + "\n")
    
    # Vérifier les arguments
    if len(sys.argv) < 2:
        safe_print("Usage:")
        safe_print("  python inference.py <model_path>")
        safe_print("  python inference.py <model_path> --image <path>")
        safe_print("  python inference.py <model_path> --video <path>")
        safe_print("  python inference.py <model_path> --webcam")
        safe_print("  python inference.py <model_path> --batch <folder>")
        safe_print("\nExemple:")
        safe_print("  python inference.py ./checkpoints/eye_disease_detector-50_XX.ckpt")
        sys.exit(1)
    
    model_path = sys.argv[1]
    
    # Charger le détecteur
    try:
        detector = EyeDetector(model_path)
    except Exception as e:
        safe_print(f"ERROR: Model loading failed: {e}")
        sys.exit(1)
    
    # Mode de fonctionnement
    if len(sys.argv) > 2:
        mode = sys.argv[2]
        
        if mode == "--image" and len(sys.argv) > 3:
            test_single_image(detector, sys.argv[3])
        
        elif mode == "--video" and len(sys.argv) > 3:
            test_single_video(detector, sys.argv[3])
        
        elif mode == "--webcam":
            test_webcam(detector)
        
        elif mode == "--batch" and len(sys.argv) > 3:
            batch_process_directory(detector, sys.argv[3])
        
        else:
            safe_print("ERROR: Invalid arguments")
            sys.exit(1)
    else:
        # Mode interactif
        interactive_menu(detector)


if __name__ == "__main__":
    main()
