#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Health Check - Monitore le Flask server toutes les 5 minutes
Redémarre si besoin et log les problèmes
"""

import os
import sys
import time
import subprocess
import requests
from datetime import datetime

LOG_FILE = "/home/mera/mera_detection/logs/health_check.log"
FLASK_URL = "http://localhost:5000/api/capture"
CHECK_INTERVAL = 300  # 5 minutes

def log_message(msg):
    """Log avec timestamp"""
    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    log_entry = f"[{timestamp}] {msg}"
    print(log_entry)
    
    try:
        os.makedirs(os.path.dirname(LOG_FILE), exist_ok=True)
        with open(LOG_FILE, "a") as f:
            f.write(log_entry + "\n")
    except:
        pass

def is_service_running():
    """Vérifie si le service est actif"""
    try:
        result = subprocess.run(
            ["systemctl", "is-active", "flask_server"],
            capture_output=True,
            text=True,
            timeout=5
        )
        return result.returncode == 0
    except:
        return False

def is_flask_responding():
    """Vérifie si Flask répond"""
    try:
        response = requests.get(
            "http://localhost:5000/health",
            timeout=5
        )
        return response.status_code == 200
    except:
        # Même si /health n'existe pas, essaie de vérifier via une requête POST
        try:
            response = requests.post(
                FLASK_URL,
                json={"patient_id": 0},
                timeout=5
            )
            # 202 = Processing = service OK
            return response.status_code == 202
        except:
            return False

def restart_service():
    """Redémarre le service"""
    log_message("[HEALTH] Service non réactif - Redémarrage...")
    try:
        subprocess.run(
            ["sudo", "systemctl", "restart", "flask_server"],
            capture_output=True,
            timeout=10
        )
        log_message("[HEALTH] Service redémarré avec succès")
        return True
    except Exception as e:
        log_message(f"[HEALTH] ERREUR redémarrage: {e}")
        return False

def main():
    """Boucle de monitoring"""
    log_message("[HEALTH] ===== Health Check Démarré =====")
    log_message(f"[HEALTH] Interval de vérification: {CHECK_INTERVAL}s")
    
    consecutive_failures = 0
    max_failures = 2
    
    while True:
        try:
            time.sleep(CHECK_INTERVAL)
            
            # Vérification 1: Service systemd
            if not is_service_running():
                consecutive_failures += 1
                log_message(f"[HEALTH] ❌ Service NON actif (fail {consecutive_failures}/{max_failures})")
                
                if consecutive_failures >= max_failures:
                    restart_service()
                    consecutive_failures = 0
                continue
            
            # Vérification 2: Flask répond
            if not is_flask_responding():
                consecutive_failures += 1
                log_message(f"[HEALTH] ❌ Flask NON réactif (fail {consecutive_failures}/{max_failures})")
                
                if consecutive_failures >= max_failures:
                    restart_service()
                    consecutive_failures = 0
                continue
            
            # Tout est OK
            consecutive_failures = 0
            log_message("[HEALTH] ✓ Service OK - Audit réussi")
            
        except Exception as e:
            log_message(f"[HEALTH] ERREUR: {e}")
            consecutive_failures += 1
            if consecutive_failures >= max_failures:
                restart_service()
                consecutive_failures = 0

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        log_message("[HEALTH] Arrêt du monitoring")
        sys.exit(0)
    except Exception as e:
        log_message(f"[HEALTH] ERREUR FATALE: {e}")
        sys.exit(1)
