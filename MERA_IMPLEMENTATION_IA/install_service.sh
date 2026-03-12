#!/bin/bash
# ============================================================================
# Installation du serveur Flask en tant que service systemd
# Usage: sudo bash install_service.sh
# ============================================================================

echo "=================================================="
echo "Installation du service Flask - Détection Oculaire"
echo "=================================================="

# Vérifier les droits root
if [ "$EUID" -ne 0 ]; then
   echo "❌ Ce script doit être exécuté avec sudo"
   exit 1
fi

SERVICE_FILE="/etc/systemd/system/flask_server.service"
SOURCE_FILE="flask_server.service"

echo ""
echo "📋 Étape 1: Copie du fichier service"
cp "$SOURCE_FILE" "$SERVICE_FILE"
echo "✅ Fichier copié: $SERVICE_FILE"

echo ""
echo "📋 Étape 2: Rechargement de systemd"
systemctl daemon-reload
echo "✅ systemd rechargé"

echo ""
echo "📋 Étape 3: Activation du service au démarrage"
systemctl enable flask_server.service
echo "✅ Service activé"

echo ""
echo "📋 Étape 4: Démarrage du service"
systemctl start flask_server.service
echo "✅ Service démarré"

echo ""
echo "=================================================="
echo "✅ INSTALLATION RÉUSSIE"
echo "=================================================="
echo ""
echo "Commandes utiles:"
echo "  - Voir le statut:       sudo systemctl status flask_server"
echo "  - Arrêter le service:   sudo systemctl stop flask_server"
echo "  - Redémarrer:           sudo systemctl restart flask_server"
echo "  - Voir les logs:        sudo journalctl -u flask_server -f"
echo "  - Désactiver au boot:   sudo systemctl disable flask_server"
echo ""
echo "Le serveur est maintenant actif sur http://localhost:5000"
echo "et redémarrera automatiquement au boot du Raspberry Pi"
echo ""
