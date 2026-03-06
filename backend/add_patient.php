<?php
require_once 'config/cors.php';
require_once 'config/database.php';

header('Content-Type: application/json');

// Debug: Log la méthode HTTP
error_log(" Méthode HTTP reçue: " . $_SERVER['REQUEST_METHOD']);
error_log(" Données POST: " . json_encode($_POST));
error_log(" Données INPUT: " . file_get_contents('php://input'));

// Autoriser OPTIONS pour CORS
if ($_SERVER['REQUEST_METHOD'] == 'OPTIONS') {
    http_response_code(200);
    exit();
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    
    echo json_encode([
        'success' => false, 
        'message' => 'Méthode non autorisée. Utilisez POST.',
        'received_method' => $_SERVER['REQUEST_METHOD']
    ]);
    exit;
}

try {
    $database = new Database();
    $db = $database->getConnection();

    // Vérifier le Content-Type
    $contentType = $_SERVER['CONTENT_TYPE'] ?? '';
    error_log("📋 Content-Type: " . $contentType);

    // Récupération des données selon le Content-Type
    if (strpos($contentType, 'application/json') !== false) {
        $input = json_decode(file_get_contents('php://input'), true);
    } else {
        // Fallback pour form-data ou x-www-form-urlencoded
        $input = $_POST;
    }
    
    error_log("📥 Données reçues: " . json_encode($input));

    if (!$input || empty($input)) {
        throw new Exception('Aucune donnée reçue');
    }

    // Validation des champs obligatoires
    $required = ['rfid', 'firstName', 'lastName', 'age'];
    foreach ($required as $field) {
        if (empty($input[$field])) {
            throw new Exception("Le champ $field est obligatoire");
        }
    }

    // Vérifier si le RFID existe déjà
    $checkRfid = $db->prepare("SELECT id_utilisateur FROM utilisateur WHERE rfid_utilisateur = ?");
    $checkRfid->execute([$input['rfid']]);
    if ($checkRfid->fetch()) {
        throw new Exception("L'ID RFID '{$input['rfid']}' existe déjà");
    }

    // Calculer la date de naissance approximative
    $currentYear = date('Y');
    $birthYear = $currentYear - intval($input['age']);
    $dateNaissance = $birthYear . '-01-01'; // Approximation

    // Commencer la transaction
    $db->beginTransaction();

    // 1. Insérer dans la table utilisateur
    $sqlUser = "INSERT INTO utilisateur (nom, prenom, date_naissance, age, sexe, numero_telephone, email, adresse, rfid_utilisateur) 
                VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)";
    
    $stmtUser = $db->prepare($sqlUser);
    $success = $stmtUser->execute([
        $input['lastName'],
        $input['firstName'],
        $dateNaissance,
        intval($input['age']),
        null, // sexe - à ajouter dans le formulaire si besoin
        $input['phone'] ?? null,
        $input['email'] ?? null,
        $input['address'] ?? null,
        $input['rfid']
    ]);

    if (!$success) {
        throw new Exception("Erreur lors de l'insertion dans utilisateur");
    }

    $userId = $db->lastInsertId();
    error_log("✅ Utilisateur inséré avec ID: " . $userId);

    // 2. Insérer dans la table carte
    $sqlCarte = "INSERT INTO carte (rfid_card, id_utilisateur) VALUES (?, ?)";
    $stmtCarte = $db->prepare($sqlCarte);
    $successCarte = $stmtCarte->execute([$input['rfid'], $userId]);

    if (!$successCarte) {
        throw new Exception("Erreur lors de l'insertion dans carte");
    }

    error_log("✅ Carte RFID insérée pour l'utilisateur: " . $userId);

    // Valider la transaction
    $db->commit();

    error_log("🎉 Transaction réussie pour le patient ID: " . $userId);

    echo json_encode([
        'success' => true,
        'message' => 'Patient ajouté avec succès',
        'patient_id' => $userId,
        'rfid' => $input['rfid']
    ]);

} catch (Exception $e) {
    // Annuler la transaction en cas d'erreur
    if (isset($db) && $db->inTransaction()) {
        $db->rollBack();
        error_log("❌ Transaction annulée: " . $e->getMessage());
    }
    
    http_response_code(400);
    echo json_encode([
        'success' => false,
        'message' => 'Erreur lors de l\'ajout du patient',
        'error' => $e->getMessage()
    ]);
    
    error_log("❌ Erreur add_patient: " . $e->getMessage());
}
?>