<?php
/**
 * API GET_ANOMALIES - RÉCUPÈRE LES ANOMALIES OCULAIRES
 */

require_once 'config/database.php';
require_once 'config/cors.php';

// Configuration CORS
setupCors();

try {
    $database = new Database();
    $db = $database->getConnection();

    // Paramètres de pagination
    $limit = isset($_GET['limit']) ? intval($_GET['limit']) : 20;
    $offset = isset($_GET['offset']) ? intval($_GET['offset']) : 0;
    $limit = min($limit, 100);

    // Requête pour les anomalies récentes
    $query = "
        SELECT 
            m.id_mesure,
            m.date_mesure,
            u.id_utilisateur,
            u.nom,
            u.prenom,
            c.rfid_card,
            m.oeil_gauche,
            m.oeil_gauche_confiance,
            m.oeil_droit,
            m.oeil_droit_confiance,
            m.temperature,
            m.rythme_cardiaque,
            m.poids,
            m.spo2,
            m.photo_path
        FROM mesure m
        INNER JOIN utilisateur u ON m.id_utilisateur = u.id_utilisateur
        LEFT JOIN carte c ON u.id_utilisateur = c.id_utilisateur
        WHERE m.alerte_oculaire = 1
        ORDER BY m.date_mesure DESC
        LIMIT ? OFFSET ?
    ";

    $stmt = $db->prepare($query);
    $stmt->execute([$limit, $offset]);
    $anomalies_data = $stmt->fetchAll(PDO::FETCH_ASSOC);

    // Formater les résultats
    $anomalies = [];
    foreach ($anomalies_data as $row) {
        $anomalies[] = [
            'id_mesure' => intval($row['id_mesure']),
            'date' => $row['date_mesure'],
            'patient' => [
                'id' => intval($row['id_utilisateur']),
                'nom' => $row['nom'],
                'prenom' => $row['prenom'],
                'rfid' => $row['rfid_card']
            ],
            'yeux' => [
                'gauche' => [
                    'diagnostic' => $row['oeil_gauche'],
                    'confiance' => floatval($row['oeil_gauche_confiance'] ?? 0)
                ],
                'droit' => [
                    'diagnostic' => $row['oeil_droit'],
                    'confiance' => floatval($row['oeil_droit_confiance'] ?? 0)
                ]
            ],
            'biometrie' => [
                'temperature' => floatval($row['temperature'] ?? 0),
                'bpm' => intval($row['rythme_cardiaque'] ?? 0),
                'poids' => floatval($row['poids'] ?? 0),
                'spo2' => intval($row['spo2'] ?? 0)
            ],
            'photo_path' => $row['photo_path']
        ];
    }

    // Compter le total
    $count_query = "SELECT COUNT(*) as total FROM mesure WHERE alerte_oculaire = 1";
    $count_stmt = $db->query($count_query);
    $total = $count_stmt->fetch()['total'];

    http_response_code(200);
    echo json_encode([
        'success' => true,
        'total' => intval($total),
        'count' => count($anomalies),
        'limit' => $limit,
        'offset' => $offset,
        'anomalies' => $anomalies
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
