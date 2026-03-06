<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);

$conn = new mysqli("localhost", "root", "", "robot-medical");

if ($conn->connect_error) {
    die("Erreur connexion BD: " . $conn->connect_error);
}

echo "=== AJOUT D'UNE NOUVELLE MESURE ===\n\n";

// Afficher les utilisateurs
echo "Utilisateurs disponibles:\n";
$result = $conn->query("SELECT id_utilisateur, nom, prenom FROM utilisateur");
while($row = $result->fetch_assoc()) {
    echo "ID: " . $row['id_utilisateur'] . " - " . $row['prenom'] . " " . $row['nom'] . "\n";
}

echo "\nID Utilisateur: ";
$id_utilisateur = trim(fgets(STDIN));

echo "Poids (kg): ";
$poids = trim(fgets(STDIN));

echo "Température (°C): ";
$temperature = trim(fgets(STDIN));

echo "Rythme cardiaque (bpm): ";
$rythme_cardiaque = trim(fgets(STDIN));

// Vérifier que l'utilisateur existe
$check_sql = "SELECT id_utilisateur FROM utilisateur WHERE id_utilisateur = ?";
$check_stmt = $conn->prepare($check_sql);
$check_stmt->bind_param("i", $id_utilisateur);
$check_stmt->execute();
$check_result = $check_stmt->get_result();

if ($check_result->num_rows === 0) {
    die("❌ Erreur: L'utilisateur ID $id_utilisateur n'existe pas\n");
}

// Insérer la mesure
$sql = "INSERT INTO mesure (id_utilisateur, poids, temperature, rythme_cardiaque, date_mesure) 
        VALUES (?, ?, ?, ?, NOW())";

$stmt = $conn->prepare($sql);
$stmt->bind_param("iddd", $id_utilisateur, $poids, $temperature, $rythme_cardiaque);

if ($stmt->execute()) {
    echo "✅ Mesure ajoutée avec succès pour l'utilisateur ID: $id_utilisateur\n";
    echo "📊 ID de la nouvelle mesure: " . $conn->insert_id . "\n";
} else {
    echo "❌ Erreur: " . $conn->error . "\n";
}

$stmt->close();
$conn->close();
?>