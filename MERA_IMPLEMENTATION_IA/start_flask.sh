#!/bin/bash
# ============================================================================
# Script de démarrage du serveur Flask pour détection oculaire
# Utilisation: ./start_flask.sh
# ============================================================================

cd /home/mera/mera_detection

# Afficher les informations
echo "=================================================="
echo "DÉMARRAGE SERVEUR FLASK - DÉTECTION OCULAIRE"
echo "=================================================="
echo "Port: 5000"
echo "IP: 0.0.0.0 (accessible sur le réseau)"
echo ""
echo "URL d'accès:"
echo "  POST  http://localhost:5000/api/capture"
echo "  GET   http://localhost:5000/api/results/<session_id>"
echo "  GET   http://localhost:5000/health"
echo "=================================================="
echo ""

# Vérifier que venv est activé
if [ -z "$VIRTUAL_ENV" ]; then
    echo "⚠️  Activation de l'environnement virtuel..."
    source .venv/bin/activate
fi

# Démarrer le serveur Flask
python3 flask_server.py
