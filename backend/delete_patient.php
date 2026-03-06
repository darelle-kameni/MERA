<?php
/**
 * API DELETE PATIENT
 * Supprime un patient et ses mesures associées
 */

require_once 'config/database.php';
require_once 'config/cors.php';

header('Content-Type: application/json');
setupCors();

try {
    // Vérifier la méthode HTTP
    if ($_SERVER['REQUEST_METHOD'] !== 'DELETE' && $_SERVER['REQUEST_METHOD'] !== 'POST') {
        http_response_code(405);
        echo json_encode([
            'success' => false,
            'message' => 'Méthode non autorisée. Utilisez DELETE ou POST.'
        ]);
        exit;
    }

    // Récupérer l'ID patient
    if ($_SERVER['REQUEST_METHOD'] === 'DELETE') {
        // Pour DELETE: récupérer depuis l'URL ou le body
        $id_utilisateur = isset($_GET['id']) ? intval($_GET['id']) : null;
        
        if (!$id_utilisateur) {
            $input = json_decode(file_get_contents('php://input'), true);
            $id_utilisateur = isset($input['id_utilisateur']) ? intval($input['id_utilisateur']) : null;
        }
    } else {
        // Pour POST: récupérer depuis le formulaire ou JSON
        $input = isset($_POST) && !empty($_POST) ? $_POST : json_decode(file_get_contents('php://input'), true);
        $id_utilisateur = isset($input['id_utilisateur']) ? intval($input['id_utilisateur']) : null;
    }

    if (!$id_utilisateur) {
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'ID utilisateur manquant'
        ]);
        exit;
    }

    $database = new Database();
    $db = $database->getConnection();

    // Vérifier que le patient existe
    $query = "SELECT nom, prenom FROM utilisateur WHERE id_utilisateur = ?";
    $stmt = $db->prepare($query);
    $stmt->execute([$id_utilisateur]);
    $patient = $stmt->fetch(PDO::FETCH_ASSOC);
    
    if (!$patient) {
        http_response_code(404);
        echo json_encode([
            'success' => false,
            'message' => 'Patient non trouvé'
        ]);
        exit;
    }

    // Supprimer les mesures du patient
    $query = "DELETE FROM mesure WHERE id_utilisateur = ?";
    $stmt = $db->prepare($query);
    $stmt->execute([$id_utilisateur]);
    $mesures_supprimees = $stmt->rowCount();

    // Supprimer la carte RFID du patient
    $query = "DELETE FROM carte WHERE id_utilisateur = ?";
    $stmt = $db->prepare($query);
    $stmt->execute([$id_utilisateur]);

    // Supprimer le patient
    $query = "DELETE FROM utilisateur WHERE id_utilisateur = ?";
    $stmt = $db->prepare($query);
    $result = $stmt->execute([$id_utilisateur]);

    if ($result && $stmt->rowCount() > 0) {
        echo json_encode([
            'success' => true,
            'message' => 'Patient ' . $patient['prenom'] . ' ' . $patient['nom'] . ' supprimé avec succès',
            'patient' => [
                'id_utilisateur' => $id_utilisateur,
                'nom' => $patient['nom'],
                'prenom' => $patient['prenom']
            ],
            'mesures_supprimees' => $mesures_supprimees
        ]);
    } else {
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Erreur lors de la suppression'
        ]);
    }

} catch (Exception $e) {
    http_response_code(500);
    echo json_encode([
        'success' => false,
        'message' => 'Erreur serveur: ' . $e->getMessage()
    ]);
}
?>
