<?php
require_once 'config/database.php';
require_once 'config/cors.php';

// Configuration CORS
setupCors();

/**
 * API Arduino - Communication avec le système médical
 * 
 * Endpoints:
 * 1. POST /api_arduino.php?action=rechercher_etudiant
 *    - Recherche un étudiant par UID RFID
 *    - Retourne les informations de l'étudiant
 * 
 * 2. POST /api_arduino.php?action=ajouter_mesure
 *    - Ajoute une mesure médicale pour un étudiant
 *    - Retourne le statut de la mesure
 */

// Fonction pour rechercher un étudiant par UID RFID
function rechercherEtudiant($db, $uid_rfid) {
    try {
        // Nettoyer l'UID
        $uid_rfid = trim($uid_rfid);
        
        if (empty($uid_rfid)) {
            return [
                'success' => false,
                'message' => 'UID RFID vide'
            ];
        }

        // Recherche de l'étudiant avec la carte
        $query = "SELECT 
                    u.id_utilisateur, 
                    u.nom, 
                    u.prenom,
                    u.date_naissance,
                    u.sexe,
                    c.rfid_card,
                    c.id_utilisateur
                  FROM utilisateur u 
                  INNER JOIN carte c ON u.id_utilisateur = c.id_utilisateur 
                  WHERE c.rfid_card = ? OR u.rfid_utilisateur = ?";
        
        $stmt = $db->prepare($query);
        $stmt->execute([$uid_rfid, $uid_rfid]);
        $etudiant = $stmt->fetch(PDO::FETCH_ASSOC);

        if ($etudiant) {
            return [
                'success' => true,
                'etudiant' => [
                    'id_utilisateur' => (int)$etudiant['id_utilisateur'],
                    'nom' => $etudiant['nom'],
                    'prenom' => $etudiant['prenom'],
                    'nom_complet' => $etudiant['prenom'] . ' ' . $etudiant['nom'],
                    'date_naissance' => $etudiant['date_naissance'],
                    'sexe' => $etudiant['sexe'],
                    'rfid_card' => $etudiant['rfid_card'],
                    'id_utilisateur_carte' => $etudiant['id_utilisateur']
                ]
            ];
        } else {
            return [
                'success' => false,
                'message' => 'Étudiant non trouvé pour cet UID RFID'
            ];
        }

    } catch (Exception $e) {
        return [
            'success' => false,
            'message' => 'Erreur lors de la recherche: ' . $e->getMessage()
        ];
    }
}

// Fonction pour ajouter une mesure médicale
function ajouterMesureArduino($db, $data) {
    try {
        // Validation des données requises
        $required_fields = ['id_utilisateur', 'temperature', 'rythme_cardiaque', 'spo2', 'poids'];
        foreach ($required_fields as $field) {
            if (!isset($data[$field])) {
                return [
                    'success' => false,
                    'message' => "Champ manquant: $field"
                ];
            }
        }

        // Récupération et validation des données
        $id_utilisateur = (int)$data['id_utilisateur'];
        $temperature = floatval($data['temperature']);
        $rythme_cardiaque = intval($data['rythme_cardiaque']);
        $spo2 = intval($data['spo2']);
        $poids = floatval($data['poids']);

        // Validation des valeurs
        $erreurs = [];
        
        if ($id_utilisateur <= 0) {
            $erreurs[] = "ID utilisateur invalide";
        }
        if ($temperature < 30 || $temperature > 45) {
            $erreurs[] = "Température doit être entre 30°C et 45°C";
        }
        if ($rythme_cardiaque < 30 || $rythme_cardiaque > 200) {
            $erreurs[] = "Rythme cardiaque doit être entre 30 et 200 bpm";
        }
        if ($poids < 20 || $poids > 200) {
            $erreurs[] = "Poids doit être entre 20kg et 200kg";
        }
        if ($spo2 < 70 || $spo2 > 100) {
            $erreurs[] = "SpO2 doit être entre 70% et 100%";
        }

        if (!empty($erreurs)) {
            return [
                'success' => false,
                'message' => 'Erreurs de validation',
                'erreurs' => $erreurs
            ];
        }

        // Calculer le statut
        $statut = 'normal';
        if ($temperature > 40 || $rythme_cardiaque > 100 || $spo2 < 90) {
            $statut = 'critique';
        } elseif (($temperature >= 38 && $temperature <= 40) || 
                 $temperature < 37 || 
                 $rythme_cardiaque < 50 || 
                 ($spo2 >= 90 && $spo2 < 95)) {
            $statut = 'alerte';
        }

        // Insérer la mesure
        $query = "INSERT INTO mesure 
                  (id_utilisateur, temperature, rythme_cardiaque, poids, spo2, statut, date_mesure) 
                  VALUES (?, ?, ?, ?, ?, ?, NOW())";
        
        $stmt = $db->prepare($query);
        $result = $stmt->execute([
            $id_utilisateur, 
            $temperature, 
            $rythme_cardiaque, 
            $poids, 
            $spo2, 
            $statut
        ]);

        if ($result) {
            $id_mesure = $db->lastInsertId();
            
            return [
                'success' => true,
                'message' => 'Mesure ajoutée avec succès',
                'id_mesure' => $id_mesure,
                'statut' => $statut,
                'timestamp' => date('Y-m-d H:i:s')
            ];
        } else {
            throw new Exception('Erreur lors de l\'insertion en base de données');
        }

    } catch (Exception $e) {
        return [
            'success' => false,
            'message' => 'Erreur: ' . $e->getMessage()
        ];
    }
}

// Traitement principal
try {
    $database = new Database();
    $db = $database->getConnection();

    // Récupérer l'action
    $action = $_GET['action'] ?? '';

    // Lire les données JSON
    $input = json_decode(file_get_contents('php://input'), true);
    
    if ($input === null) {
        throw new Exception('Données JSON invalides');
    }

    switch ($action) {
        case 'rechercher_etudiant':
            if (!isset($input['uid_rfid'])) {
                throw new Exception('UID RFID manquant');
            }
            $resultat = rechercherEtudiant($db, $input['uid_rfid']);
            break;

        case 'ajouter_mesure':
            $resultat = ajouterMesureArduino($db, $input);
            break;

        default:
            $resultat = [
                'success' => false,
                'message' => 'Action non reconnue',
                'actions_disponibles' => ['rechercher_etudiant', 'ajouter_mesure']
            ];
            break;
    }

    echo json_encode($resultat);

} catch (Exception $e) {
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erreur serveur: ' . $e->getMessage()
    ]);
}
?>
