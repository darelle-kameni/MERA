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

    // Récupérer les dernières mesures du jour
    $query = "
        SELECT 
            m.id_mesure as measureId,
            u.id_utilisateur as userId,
            c.rfid_card as rfid,
            u.prenom as firstName,
            u.nom as lastName,
            m.temperature as temp,
            m.rythme_cardiaque as heartRate,
            m.poids as weight,
            m.spo2 as spo2,
            m.oeil_gauche as oeil_gauche,
            m.oeil_droit as oeil_droit,
            m.oeil_gauche_confiance as oeil_gauche_confiance,
            m.oeil_droit_confiance as oeil_droit_confiance,
            m.alerte_oculaire as alerte_oculaire,
            m.date_mesure as date,
            m.statut as status
        FROM mesure m
        INNER JOIN utilisateur u ON m.id_utilisateur = u.id_utilisateur
        LEFT JOIN carte c ON u.id_utilisateur = c.id_utilisateur
        WHERE DATE(m.date_mesure) = CURDATE()
        ORDER BY m.date_mesure DESC
        LIMIT 20
    ";

    $stmt = $db->prepare($query);
    $stmt->execute();
    $measures = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // Calculer le statut pour chaque mesure (avec SpO2)
    foreach ($measures as &$measure) {
        $temp = $measure['temp'];
        $heartRate = $measure['heartRate'];
        $spo2 = $measure['spo2'];
        
        if ($temp !== null && $heartRate !== null && $spo2 !== null) {
            if ($temp > 40 || $heartRate > 100 || $spo2 < 90) {
                $measure['status'] = 'critique';
            } elseif (($temp >= 38 && $temp <= 40) || $temp < 37 || $heartRate < 50 || ($spo2 >= 90 && $spo2 < 95)) {
                $measure['status'] = 'alerte';
            } else {
                $measure['status'] = 'normal';
            }
        } else {
            $measure['status'] = 'non_mesure';
        }
    }

    echo json_encode([
        'success' => true,
        'measures' => $measures,
        'count' => count($measures),
        'queryDate' => date('Y-m-d')
    ]);

} catch (Exception $e) {
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erreur lors de la récupération des mesures récentes',
        'error' => $e->getMessage()
    ]);
}
?>