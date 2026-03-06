<?php
/**
 * API ADD_MESURE - VERSION AVEC SUPPORT DONNÉES OCULAIRES
 * Reçoit les mesures biométriques et oculaires de l'ESP32
 */

require_once 'config/database.php';

header('Content-Type: application/json; charset=utf-8');
error_reporting(E_ALL);
ini_set('display_errors', 1);

// ============================================================
// RÉCEPTION DES DONNÉES
// ============================================================

$request_method = $_SERVER['REQUEST_METHOD'];

// Accepter JSON POST et form-data POST
if ($request_method === 'POST') {
    $content_type = $_SERVER['CONTENT_TYPE'] ?? '';
    
    if (strpos($content_type, 'application/json') !== false) {
        $input = json_decode(file_get_contents('php://input'), true);
    } else {
        $input = $_POST;
    }
    
    try {
        $database = new Database();
        $db = $database->getConnection();
        
        // Mesures biométriques
        $id_utilisateur = isset($input['id_utilisateur']) ? intval($input['id_utilisateur']) : null;
        $poids = isset($input['poids']) ? floatval($input['poids']) : null;
        $temperature = isset($input['temperature']) ? floatval($input['temperature']) : null;
        $rythme_cardiaque = isset($input['rythme_cardiaque']) ? intval($input['rythme_cardiaque']) : null;
        $spo2 = isset($input['spo2']) ? intval($input['spo2']) : null;
        
        // ⭐⭐ DONNÉES OCULAIRES - Accepter les noms envoyés par l'ESP32
        $oeil_gauche = isset($input['oeil_gauche']) ? htmlspecialchars(trim($input['oeil_gauche'])) : null;
        $oeil_gauche_confiance = isset($input['oeil_gauche_conf']) ? floatval($input['oeil_gauche_conf']) : (isset($input['oeil_gauche_confiance']) ? floatval($input['oeil_gauche_confiance']) : null);
        $oeil_droit = isset($input['oeil_droit']) ? htmlspecialchars(trim($input['oeil_droit'])) : null;
        $oeil_droit_confiance = isset($input['oeil_droit_conf']) ? floatval($input['oeil_droit_conf']) : (isset($input['oeil_droit_confiance']) ? floatval($input['oeil_droit_confiance']) : null);
        $alerte_oculaire = isset($input['alerte_oculaire']) ? intval($input['alerte_oculaire']) : 0;
        $photo_path = isset($input['photo_path']) ? htmlspecialchars(trim($input['photo_path'])) : null;
        
        // ============================================================
        // VALIDATION
        // ============================================================
        
        $errors = [];
        
        if (!$id_utilisateur) {
            $errors[] = "ID utilisateur manquant";
        }
        
        // Au moins une mesure requise
        if (!$poids && !$temperature && !$rythme_cardiaque && !$spo2 && !$oeil_gauche && !$oeil_droit) {
            $errors[] = "Aucune donnée à insérer";
        }
        
        if (!empty($errors)) {
            http_response_code(400);
            echo json_encode([
                "success" => false,
                "message" => "Données invalides",
                "errors" => $errors
            ]);
            exit;
        }
        
        // ============================================================
        // INSERTION
        // ============================================================
        
        $sql = "INSERT INTO mesure (
            id_utilisateur, 
            poids, 
            temperature, 
            rythme_cardiaque,
            spo2,
            oeil_gauche,
            oeil_gauche_confiance,
            oeil_droit,
            oeil_droit_confiance,
            alerte_oculaire,
            photo_path,
            date_mesure,
            statut
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), 'normal')";
        
        $stmt = $db->prepare($sql);
        $stmt->execute([
            $id_utilisateur,
            $poids,
            $temperature,
            $rythme_cardiaque,
            $spo2,
            $oeil_gauche,
            $oeil_gauche_confiance,
            $oeil_droit,
            $oeil_droit_confiance,
            $alerte_oculaire,
            $photo_path
        ]);
        
        $id_mesure = $db->lastInsertId();
        
        // 📝 Logger les alertes oculaires
        if ($alerte_oculaire == 1) {
            $log_msg = "[" . date('Y-m-d H:i:s') . "] ALERTE OCULAIRE - ID:$id_mesure | User:$id_utilisateur | OG:$oeil_gauche ($oeil_gauche_confiance) | OD:$oeil_droit ($oeil_droit_confiance)\n";
            @file_put_contents(__DIR__ . '/logs/alertes_oculaires.log', $log_msg, FILE_APPEND);
        }
        
        http_response_code(201);
        echo json_encode([
            "success" => true,
            "message" => "Mesure enregistrée",
            "id_mesure" => $id_mesure,
            "timestamp" => date('Y-m-d H:i:s')
        ]);
        
    } catch (Exception $e) {
        http_response_code(500);
        echo json_encode([
            "success" => false,
            "message" => "Erreur serveur",
            "error" => $e->getMessage()
        ]);
    }
} else {
    http_response_code(405);
    echo json_encode([
        "success" => false,
        "message" => "Méthode non autorisée"
    ]);
}
?>