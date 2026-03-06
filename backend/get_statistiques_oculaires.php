<?php
/**
 * API GET_STATISTIQUES_OCULAIRES - STATISTIQUES DE DÉTECTION OCULAIRE
 */

require_once 'config/database.php';
require_once 'config/cors.php';

// Configuration CORS
setupCors();

try {
    $database = new Database();
    $db = $database->getConnection();

    $stats = [];

    // ⭐ TOTAL ANOMALIES DÉTECTÉES
    $query = "SELECT COUNT(*) as total FROM mesure WHERE alerte_oculaire = 1";
    $result = $db->query($query);
    $stats['total_anomalies'] = intval($result->fetch()['total']);

    // ⭐ ANOMALIES AUJOURD'HUI
    $query = "SELECT COUNT(*) as total FROM mesure WHERE alerte_oculaire = 1 AND DATE(date_mesure) = CURDATE()";
    $result = $db->query($query);
    $stats['anomalies_today'] = intval($result->fetch()['total']);

    // ⭐ ANOMALIES CETTE SEMAINE
    $query = "SELECT COUNT(*) as total FROM mesure WHERE alerte_oculaire = 1 AND DATE(date_mesure) >= DATE_SUB(CURDATE(), INTERVAL 7 DAY)";
    $result = $db->query($query);
    $stats['anomalies_week'] = intval($result->fetch()['total']);

    // ⭐ DISTRIBUTION DES DIAGNOSTICS (7 derniers jours)
    $query = "
        SELECT 
            oeil_gauche as diagnostic,
            COUNT(*) as count
        FROM mesure 
        WHERE oeil_gauche IS NOT NULL 
        AND date_mesure >= DATE_SUB(NOW(), INTERVAL 7 DAY)
        GROUP BY oeil_gauche
        ORDER BY count DESC
    ";
    $result = $db->query($query);
    $diagnostics_dist = [];
    while ($row = $result->fetch()) {
        $diagnostics_dist[] = [
            'diagnostic' => $row['diagnostic'],
            'count' => intval($row['count'])
        ];
    }
    $stats['diagnostics_7days'] = $diagnostics_dist;

    // ⭐ TAUX DE DÉTECTION (30 derniers jours)
    $query = "
        SELECT 
            COUNT(*) as total,
            SUM(CASE WHEN oeil_gauche IS NOT NULL OR oeil_droit IS NOT NULL THEN 1 ELSE 0 END) as with_detection
        FROM mesure
        WHERE date_mesure >= DATE_SUB(NOW(), INTERVAL 30 DAY)
    ";
    $result = $db->query($query);
    $detection = $result->fetch();
    $detection_rate = ($detection['total'] > 0) 
        ? round(($detection['with_detection'] / $detection['total']) * 100, 2)
        : 0;
    $stats['detection_rate_30days'] = floatval($detection_rate);

    // ⭐ MOYENNE DE CONFIANCE (dernières 100 mesures avec anomalie)
    $query = "
        SELECT 
            AVG(oeil_gauche_confiance) as avg_left,
            AVG(oeil_droit_confiance) as avg_right
        FROM mesure
        WHERE alerte_oculaire = 1
        ORDER BY date_mesure DESC
        LIMIT 100
    ";
    $result = $db->query($query);
    $confidence = $result->fetch();
    $stats['avg_confidence'] = [
        'left_eye' => floatval($confidence['avg_left'] ?? 0),
        'right_eye' => floatval($confidence['avg_right'] ?? 0)
    ];

    // ⭐ PATIENTS AVEC ANOMALIES
    $query = "
        SELECT COUNT(DISTINCT id_utilisateur) as total 
        FROM mesure 
        WHERE alerte_oculaire = 1
    ";
    $result = $db->query($query);
    $stats['patients_with_anomalies'] = intval($result->fetch()['total']);

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'statistiques' => $stats,
        'timestamp' => date('Y-m-d H:i:s')
    ]);

} catch (Exception $e) {
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erreur serveur',
        'error' => $e->getMessage()
    ]);
}
?>
