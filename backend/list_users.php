<?php
$conn = new mysqli("localhost", "root", "", "robot-medical");

echo "<h2>Utilisateurs disponibles:</h2>";
$result = $conn->query("SELECT * FROM utilisateur");
while($row = $result->fetch_assoc()) {
    echo "ID: " . $row['id_utilisateur'] . " - Nom: " . $row['nom'] . "<br>";
}

$conn->close();
?>