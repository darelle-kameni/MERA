<?php
require_once 'config/database.php';
require_once 'config/cors.php';

// Configuration CORS
setupCors();

try {
    $database = new Database();
    $db = $database->getConnection();

    // 1. Nombre total de patients
    $queryPatients = "SELECT COUNT(*) as total FROM utilisateur";
    $stmtPatients = $db->query($queryPatients);
    $totalPatients = $stmtPatients->fetch()['total'];

    // 2. Mesures aujourd'hui
    $today = date('Y-m-d');
    $queryTodayMeasures = "SELECT COUNT(*) as total FROM mesure WHERE DATE(date_mesure) = ?";
    $stmtTodayMeasures = $db->prepare($queryTodayMeasures);
    $stmtTodayMeasures->execute([$today]);
    $todayMeasures = $stmtTodayMeasures->fetch()['total'];

    // 3. Alertes cardiaques (rythme > 100 ou < 50)
    $queryHeartAlerts = "
        SELECT COUNT(*) as total 
        FROM mesure 
        WHERE (rythme_cardiaque > 100 OR rythme_cardiaque < 50)
        AND DATE(date_mesure) = ?
    ";
    $stmtHeartAlerts = $db->prepare($queryHeartAlerts);
    $stmtHeartAlerts->execute([$today]);
    $heartAlerts = $stmtHeartAlerts->fetch()['total'];

    // 4. Températures critiques (> 38°C)
    $queryTempAlerts = "
        SELECT COUNT(*) as total 
        FROM mesure 
        WHERE temperature > 38 
        AND DATE(date_mesure) = ?
    ";
    $stmtTempAlerts = $db->prepare($queryTempAlerts);
    $stmtTempAlerts->execute([$today]);
    $tempAlerts = $stmtTempAlerts->fetch()['total'];

    // 5. Alertes SpO2 (< 95%) - NOUVEAU
    $querySpo2Alerts = "
        SELECT COUNT(*) as total 
        FROM mesure 
        WHERE spo2 < 95 
        AND DATE(date_mesure) = ?
    ";
    $stmtSpo2Alerts = $db->prepare($querySpo2Alerts);
    $stmtSpo2Alerts->execute([$today]);
    $spo2Alerts = $stmtSpo2Alerts->fetch()['total'];

    echo json_encode([
        'success' => true,
        'stats' => [
            'totalPatients' => $totalPatients,
            'todayMeasures' => $todayMeasures,
            'heartAlerts' => $heartAlerts,
            'tempAlerts' => $tempAlerts,
            'spo2Alerts' => $spo2Alerts // NOUVEAU
        ],
        'lastUpdate' => date('Y-m-d H:i:s')
    ]);

} catch (Exception $e) {
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erreur lors de la récupération des statistiques',
        'error' => $e->getMessage()
    ]);
}
?>
