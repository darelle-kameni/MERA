<?php
error_reporting(E_ALL);
ini_set('display_errors', 1);

$conn = new mysqli("localhost", "root", "", "robot-medical");

if ($conn->connect_error) {
    die("Erreur connexion BD: " . $conn->connect_error);
}

echo "<h2>🔧 CONFIGURATION BASE DE DONNÉES POUR ARDUINO</h2>";

// 1. Supprimer et recréer la table carte proprement
echo "<h3>1. Création table 'carte'...</h3>";
$conn->query("DROP TABLE IF EXISTS carte");

$sql_carte = "CREATE TABLE carte (
    id_carte VARCHAR(50) PRIMARY KEY,
    id_utilisateur INT,
    date_creation TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (id_utilisateur) REFERENCES utilisateur(id_utilisateur)
)";

if ($conn->query($sql_carte)) {
    echo "✅ Table 'carte' créée avec succès<br>";
} else {
    echo "❌ Erreur création table carte: " . $conn->error . "<br>";
    exit;
}

// 2. Vérifier/Ajouter colonne spo2
echo "<h3>2. Colonne 'spo2' dans 'mesure'...</h3>";
$check_spo2 = $conn->query("SHOW COLUMNS FROM mesure LIKE 'spo2'");
if ($check_spo2->num_rows == 0) {
    if ($conn->query("ALTER TABLE mesure ADD COLUMN spo2 INT")) {
        echo "✅ Colonne 'spo2' ajoutée<br>";
    } else {
        echo "❌ Erreur ajout spo2: " . $conn->error . "<br>";
    }
} else {
    echo "✅ Colonne 'spo2' existe déjà<br>";
}

// 3. Ajouter données de test directement
echo "<h3>3. Ajout données de test...</h3>";
$sql_insert = "INSERT INTO carte (id_carte, id_utilisateur) VALUES 
    ('123456789', 1),
    ('987654321', 2),
    ('555555555', 3)";

if ($conn->query($sql_insert)) {
    echo "✅ Données de test ajoutées:<br>";
    echo "- Carte 123456789 → Jean Dupont (ID: 1)<br>";
    echo "- Carte 987654321 → Marie Martin (ID: 2)<br>";
    echo "- Carte 555555555 → Pierre Lambert (ID: 3)<br>";
} else {
    echo "❌ Erreur insertion: " . $conn->error . "<br>";
}

// 4. Vérification
echo "<h3>4. Vérification...</h3>";
$result = $conn->query("
    SELECT c.id_carte, u.id_utilisateur, u.prenom, u.nom 
    FROM carte c 
    JOIN utilisateur u ON c.id_utilisateur = u.id_utilisateur
");

if ($result->num_rows > 0) {
    echo "✅ Table 'carte' configurée:<br>";
    while($row = $result->fetch_assoc()) {
        echo "- {$row['id_carte']} → {$row['prenom']} {$row['nom']}<br>";
    }
}

echo "<hr><h3>🎯 CONFIGURATION TERMINÉE!</h3>";
echo "<p><a href='test_apis.php' style='color: blue;'>🧪 Tester les APIs maintenant</a></p>";

$conn->close();
?>