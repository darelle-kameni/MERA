<?php
header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, GET, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

error_reporting(E_ALL);
ini_set('display_errors', 1);

$conn = new mysqli("localhost", "root", "", "robot-medical");

if ($conn->connect_error) {
    echo json_encode(["success" => false, "message" => "Erreur connexion BD"]);
    exit;
}

// Accepter OPTIONS pour CORS
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    exit(0);
}

// Lire les données brutes
$raw_input = file_get_contents('php://input');

// Essayer de parser JSON
$input = json_decode($raw_input, true);

if ($input === null || !isset($input['id_carte'])) {
    $response = json_encode([
        "success" => false, 
        "message" => "Format JSON invalide ou id_carte manquant",
        "debug" => ["raw_input" => $raw_input]
    ]);
    echo $response;
    exit;
}

$id_carte = trim($input['id_carte']);

if (empty($id_carte)) {
    $response = json_encode(["success" => false, "message" => "ID carte vide"]);
    echo $response;
    exit;
}

// Recherche de l'étudiant
$sql = "SELECT u.id_utilisateur, u.nom, u.prenom 
        FROM utilisateur u 
        INNER JOIN carte c ON u.id_utilisateur = c.id_utilisateur 
        WHERE c.id_carte = ?";
        
$stmt = $conn->prepare($sql);
$stmt->bind_param("s", $id_carte);
$stmt->execute();
$result = $stmt->get_result();

if ($result->num_rows > 0) {
    $etudiant = $result->fetch_assoc();
    $response = json_encode([
        "success" => true, 
        "etudiant" => [
            "id_utilisateur" => (int)$etudiant['id_utilisateur'],
            "nom" => $etudiant['nom'],
            "prenom" => $etudiant['prenom']
        ]
    ]);
} else {
    $response = json_encode(["success" => false, "message" => "Carte non trouvée"]);
}

echo $response;

$stmt->close();
$conn->close();
?>