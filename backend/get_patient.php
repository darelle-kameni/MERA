<?php
require_once 'config/database.php';

// Headers CORS
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: http://localhost:5174');
header('Access-Control-Allow-Credentials: true');
header('Access-Control-Allow-Headers: Content-Type, Accept, Authorization, X-Requested-With');
header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS');

if ($_SERVER['REQUEST_METHOD'] == 'OPTIONS') {
    http_response_code(200);
    exit();
}

try {
    $database = new Database();
    $db = $database->getConnection();

    $query = "
        SELECT 
            u.id_utilisateur as id,
            CONCAT(u.nom, ' ', u.prenom) as name,
            u.age,
            c.rfid_card as rfid,
            m.temperature as temp,
            m.rythme_cardiaque as heartRate,
            m.poids as weight,
            m.spo2 as spo2,  -- NOUVEAU: SpO2
            DATE(m.date_mesure) as lastCheck,
            m.statut as status
        FROM utilisateur u
        LEFT JOIN carte c ON u.id_utilisateur = c.id_utilisateur
        LEFT JOIN (
            SELECT id_utilisateur, MAX(date_mesure) as derniere_mesure
            FROM mesure
            GROUP BY id_utilisateur
        ) derniere ON u.id_utilisateur = derniere.id_utilisateur
        LEFT JOIN mesure m ON derniere.id_utilisateur = m.id_utilisateur AND derniere.derniere_mesure = m.date_mesure
        ORDER BY u.nom, u.prenom
    ";

    $stmt = $db->prepare($query);
    $stmt->execute();
    $patients = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // Calculer les statuts avec SpO2
    foreach ($patients as &$patient) {
        $temp = $patient['temp'];
        $heartRate = $patient['heartRate'];
        $spo2 = $patient['spo2'];
        
        // Déterminer le statut basé sur les valeurs
        if ($temp !== null && $heartRate !== null && $spo2 !== null) {
            if ($temp > 40 || $heartRate > 100 || $spo2 < 90) {
                $patient['status'] = 'critique';
            } elseif (($temp >= 38 && $temp <= 40) || $temp < 37 || $heartRate < 50 || ($spo2 >= 90 && $spo2 < 95)) {
                $patient['status'] = 'alerte';
            } else {
                $patient['status'] = 'normal';
            }
        } else {
            $patient['status'] = 'non_mesure';
        }
    }

    echo json_encode([
        'success' => true,
        'patients' => $patients
    ]);

} catch (Exception $e) {
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erreur de base de données',
        'error' => $e->getMessage()
    ]);
}
?>