#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Capture vidéo alternative pour Raspberry Pi utilisant rpicam-vid
Contourne les problèmes de libcamera/OpenCV
"""

import os
os.environ['PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION'] = 'python'

import subprocess
import numpy as np
import cv2
import threading
from queue import Queue
import time


class RPiCameraCapture:
    """Capture vidéo via rpicam-vid en streaming"""
    
    def __init__(self, width=640, height=480, fps=30):
        self.width = width
        self.height = height
        self.fps = fps
        self.frame_queue = Queue(maxsize=2)
        self.process = None
        self.thread = None
        self.running = False
        
    def start(self):
        """Démarrer la capture"""
        self.running = True
        
        # Lancer rpicam-vid avec output en fichier FIFO
        cmd = [
            'rpicam-vid',
            '-t', '0',  # Durée infinie
            '--width', str(self.width),
            '--height', str(self.height),
            '--framerate', str(self.fps),
            '--codec', 'mjpeg',
            '-o', '-'  # Output vers stdout
        ]
        
        try:
            self.process = subprocess.Popen(
                cmd,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                bufsize=10*1024*1024
            )
            
            # Démarrer thread de lecture
            self.thread = threading.Thread(target=self._read_frames, daemon=True)
            self.thread.start()
            
            print("✅ rpicam-vid lancé avec succès")
            return True
        except Exception as e:
            print(f"❌ Erreur: {e}")
            return False
    
    def _read_frames(self):
        """Thread: lire les frames du JPEG stream"""
        try:
            while self.running:
                # Lire le header JPEG
                data = self.process.stdout.read(2)
                if len(data) < 2:
                    break
                
                # Chercher le début du JPEG (FFD8)
                if data != b'\xff\xd8':
                    continue
                
                # Lire jusqu'à la fin du JPEG (FFD9)
                jpeg_data = data
                while True:
                    byte = self.process.stdout.read(1)
                    if not byte:
                        break
                    jpeg_data += byte
                    if byte == b'\xd9' and len(jpeg_data) > 2 and jpeg_data[-2:-1] == b'\xff':
                        break
                
                # Décoder le JPEG
                nparr = np.frombuffer(jpeg_data, np.uint8)
                frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
                
                if frame is not None:
                    # Mettre dans la queue (ignorer l'ancienne frame si queue pleine)
                    try:
                        self.frame_queue.put_nowait(frame)
                    except:
                        pass
        except Exception as e:
            print(f"⚠️  Erreur lecture: {e}")
        finally:
            self.running = False
    
    def read(self):
        """Lire une frame"""
        try:
            frame = self.frame_queue.get(timeout=1)
            return True, frame
        except:
            return False, None
    
    def release(self):
        """Arrêter la capture"""
        self.running = False
        if self.process:
            self.process.terminate()
            self.process.wait(timeout=2)
        if self.thread:
            self.thread.join(timeout=2)


if __name__ == "__main__":
    print("Test capture rpicam")
    cap = RPiCameraCapture()
    
    if cap.start():
        time.sleep(2)  # Laisser démarrer
        for i in range(5):
            ret, frame = cap.read()
            if ret:
                print(f"  Frame {i}: {frame.shape}")
            else:
                print(f"  Frame {i}: erreur")
    
    cap.release()
