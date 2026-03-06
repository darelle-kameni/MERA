<?php
/**
 * API UPDATE PATIENT
 * Modifie les informations d'un patient
 */

require_once 'config/database.php';
require_once 'config/cors.php';

header('Content-Type: application/json');
setupCors();

try {
    // Vérifier la méthode HTTP
    if ($_SERVER['REQUEST_METHOD'] !== 'POST' && $_SERVER['REQUEST_METHOD'] !== 'PUT') {
        http_response_code(405);
        echo json_encode([
            'success' => false,
            'message' => 'Méthode non autorisée. Utilisez POST ou PUT.'
        ]);
        exit;
    }

    // Récupérer les données
    $content_type = $_SERVER['CONTENT_TYPE'] ?? '';
    
    if (strpos($content_type, 'application/json') !== false) {
        $input = json_decode(file_get_contents('php://input'), true);
    } else {
        $input = $_POST;
    }

    // Vérifier l'ID patient
    $id_utilisateur = isset($input['id_utilisateur']) ? intval($input['id_utilisateur']) : null;
    
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
    $query = "SELECT id_utilisateur FROM utilisateur WHERE id_utilisateur = ?";
    $stmt = $db->prepare($query);
    $stmt->execute([$id_utilisateur]);
    
    if ($stmt->rowCount() === 0) {
        http_response_code(404);
        echo json_encode([
            'success' => false,
            'message' => 'Patient non trouvé'
        ]);
        exit;
    }

    // Récupérer les champs à mettre à jour
    $nom = isset($input['nom']) ? trim($input['nom']) : null;
    $prenom = isset($input['prenom']) ? trim($input['prenom']) : null;
    $age = isset($input['age']) ? intval($input['age']) : null;
    $sexe = isset($input['sexe']) ? trim($input['sexe']) : null;
    $numero_telephone = isset($input['numero_telephone']) ? trim($input['numero_telephone']) : null;
    $email = isset($input['email']) ? trim($input['email']) : null;
    $adresse = isset($input['adresse']) ? trim($input['adresse']) : null;
    $date_naissance = isset($input['date_naissance']) ? trim($input['date_naissance']) : null;

    // Construire la requête UPDATE dynamiquement
    $fields = [];
    $values = [];

    if ($nom !== null) {
        $fields[] = 'nom = ?';
        $values[] = $nom;
    }
    if ($prenom !== null) {
        $fields[] = 'prenom = ?';
        $values[] = $prenom;
    }
    if ($age !== null) {
        $fields[] = 'age = ?';
        $values[] = $age;
    }
    if ($sexe !== null) {
        $fields[] = 'sexe = ?';
        $values[] = $sexe;
    }
    if ($numero_telephone !== null) {
        $fields[] = 'numero_telephone = ?';
        $values[] = $numero_telephone;
    }
    if ($email !== null) {
        $fields[] = 'email = ?';
        $values[] = $email;
    }
    if ($adresse !== null) {
        $fields[] = 'adresse = ?';
        $values[] = $adresse;
    }
    if ($date_naissance !== null) {
        $fields[] = 'date_naissance = ?';
        $values[] = $date_naissance;
    }

    if (empty($fields)) {
        http_response_code(400);
        echo json_encode([
            'success' => false,
            'message' => 'Aucun champ à mettre à jour'
        ]);
        exit;
    }

    $values[] = $id_utilisateur;

    // Exécuter l'UPDATE
    $query = "UPDATE utilisateur SET " . implode(', ', $fields) . " WHERE id_utilisateur = ?";
    $stmt = $db->prepare($query);
    $result = $stmt->execute($values);

    if ($result) {
        // Récupérer les données mises à jour
        $query = "SELECT 
                    id_utilisateur, 
                    nom, 
                    prenom, 
                    age, 
                    sexe,
                    numero_telephone,
                    email,
                    adresse,
                    date_naissance
                  FROM utilisateur 
                  WHERE id_utilisateur = ?";
        
        $stmt = $db->prepare($query);
        $stmt->execute([$id_utilisateur]);
        $patient = $stmt->fetch(PDO::FETCH_ASSOC);

        echo json_encode([
            'success' => true,
            'message' => 'Patient modifié avec succès',
            'patient' => $patient
        ]);
    } else {
        http_response_code(500);
        echo json_encode([
            'success' => false,
            'message' => 'Erreur lors de la mise à jour'
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
