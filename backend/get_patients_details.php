<?php
require_once 'config/cors.php';
require_once 'config/database.php';

header('Content-Type: application/json');

try {
    $database = new Database();
    $db = $database->getConnection();

    // Récupérer l'ID du patient depuis l'URL
    $patientId = $_GET['id'] ?? null;
    
    if (!$patientId) {
        throw new Exception('ID patient manquant');
    }

    // 1. Informations du patient
    $queryPatient = "
        SELECT 
            u.id_utilisateur as id,
            u.nom as lastName,
            u.prenom as firstName,
            u.age,
            u.sexe as gender,
            u.numero_telephone as phone,
            u.email,
            u.adresse as address,
            u.rfid_utilisateur as rfid,
            c.rfid_card,
            DATE(u.date_naissance) as birthDate
        FROM utilisateur u
        LEFT JOIN carte c ON u.id_utilisateur = c.id_utilisateur
        WHERE u.id_utilisateur = ?
    ";

    $stmtPatient = $db->prepare($queryPatient);
    $stmtPatient->execute([$patientId]);
    $patient = $stmtPatient->fetch(PDO::FETCH_ASSOC);

    if (!$patient) {
        throw new Exception('Patient non trouvé');
    }

    // 2. Dernière mesure
    $queryLastMeasure = "
        SELECT 
            temperature as temp,
            rythme_cardiaque as heartRate,
            poids as weight,
            date_mesure as date,
            statut as status
        FROM mesure 
        WHERE id_utilisateur = ? 
        ORDER BY date_mesure DESC 
        LIMIT 1
    ";

    $stmtLastMeasure = $db->prepare($queryLastMeasure);
    $stmtLastMeasure->execute([$patientId]);
    $lastMeasurement = $stmtLastMeasure->fetch(PDO::FETCH_ASSOC);

    // 3. Historique des mesures (30 derniers jours)
    $queryHistory = "
        SELECT 
            temperature as temp,
            rythme_cardiaque as heartRate,
            poids as weight,
            date_mesure as date,
            statut as status
        FROM mesure 
        WHERE id_utilisateur = ? 
        ORDER BY date_mesure DESC 
        LIMIT 50
    ";

    $stmtHistory = $db->prepare($queryHistory);
    $stmtHistory->execute([$patientId]);
    $measurements = $stmtHistory->fetchAll(PDO::FETCH_ASSOC);

    echo json_encode([
        'success' => true,
        'patient' => $patient,
        'lastMeasurement' => $lastMeasurement ?: null,
        'measurements' => $measurements,
        'measurementsCount' => count($measurements)
    ]);

} catch (Exception $e) {
    http_response_code(404);
    echo json_encode([
        'success' => false,
        'message' => 'Erreur lors de la récupération des données',
        'error' => $e->getMessage()
    ]);
}
?>