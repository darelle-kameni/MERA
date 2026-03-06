<?php
/**
 * SCRIPT DE RÉINITIALISATION DE LA BASE DE DONNÉES
 * 
 * Ce script permet de réinitialiser complètement la base de données
 * en supprimant toutes les tables et en les recréant avec des données de test
 */

error_reporting(E_ALL);
ini_set('display_errors', 1);

header('Content-Type: text/html; charset=utf-8');

echo "<!DOCTYPE html>
<html>
<head>
    <title>Réinitialisation Base de Données - Robot Médical</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }
        h1 { color: #2563eb; border-bottom: 2px solid #2563eb; padding-bottom: 10px; }
        h2 { color: #374151; margin-top: 30px; }
        .success { color: #059669; font-weight: bold; }
        .error { color: #dc2626; font-weight: bold; }
        .warning { color: #d97706; font-weight: bold; }
        .info { color: #2563eb; }
        .step { background: #f3f4f6; padding: 15px; margin: 10px 0; border-radius: 8px; }
        .btn { 
            background: #dc2626; 
            color: white; 
            padding: 12px 24px; 
            border: none; 
            border-radius: 6px; 
            cursor: pointer; 
            font-size: 16px; 
            margin: 20px 0;
        }
        .btn:hover { background: #b91c1c; }
        .btn-success { background: #059669; }
        .btn-success:hover { background: #047857; }
    </style>
</head>
<body>
    <h1>🔧 RÉINITIALISATION BASE DE DONNÉES ROBOT MÉDICAL</h1>
";

// Configuration de la base de données
$host = "localhost";
$username = "dark-linux";
$password = "";
$database = "robot_medical";

try {
    // Connexion à MySQL
    $conn = new mysqli($host, $username, $password);
    
    if ($conn->connect_error) {
        throw new Exception("Erreur de connexion MySQL: " . $conn->connect_error);
    }
    
    echo "<div class='success'>✅ Connexion MySQL réussie</div>";
    
    // Étape 1: Supprimer la base de données si elle existe
    echo "<div class='step'>
        <h2>1. Suppression de la base de données</h2>";
    
    if ($conn->query("DROP DATABASE IF EXISTS $database")) {
        echo "<div class='success'>✅ Base de données supprimée</div>";
    } else {
        throw new Exception("❌ Erreur suppression base: " . $conn->error);
    }
    
    echo "</div>";
    
    // Étape 2: Recréer la base de données
    echo "<div class='step'>
        <h2>2. Création de la base de données</h2>";
    
    if ($conn->query("CREATE DATABASE $database CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci")) {
        echo "<div class='success'>✅ Base de données créée</div>";
    } else {
        throw new Exception("❌ Erreur création base: " . $conn->error);
    }
    
    // Sélectionner la base
    $conn->select_db($database);
    echo "</div>";
    
    // Étape 3: Création des tables
    echo "<div class='step'>
        <h2>3. Création des tables</h2>";
    
    // Table utilisateur
    $sql_utilisateur = "CREATE TABLE utilisateur (
        id_utilisateur INT AUTO_INCREMENT PRIMARY KEY,
        nom VARCHAR(100) NOT NULL,
        prenom VARCHAR(100) NOT NULL,
        date_naissance DATE,
        age INT NOT NULL,
        sexe CHAR(1) CHECK (sexe IN ('M','F')),
        numero_telephone VARCHAR(20),
        email VARCHAR(100),
        adresse TEXT,
        rfid_utilisateur VARCHAR(50) UNIQUE,
        contact_urgence VARCHAR(100),
        historique_medical TEXT
    )";
    
    if ($conn->query($sql_utilisateur)) {
        echo "<div class='success'>✅ Table 'utilisateur' créée</div>";
    } else {
        throw new Exception("❌ Erreur création table utilisateur: " . $conn->error);
    }
    
    // Table carte
    $sql_carte = "CREATE TABLE carte (
        rfid_card VARCHAR(50) PRIMARY KEY,
        id_utilisateur INT NOT NULL UNIQUE,
        date_creation TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        FOREIGN KEY (id_utilisateur) REFERENCES utilisateur(id_utilisateur) ON DELETE CASCADE
    )";
    
    if ($conn->query($sql_carte)) {
        echo "<div class='success'>✅ Table 'carte' créée</div>";
    } else {
        throw new Exception("❌ Erreur création table carte: " . $conn->error);
    }
    
    // Table mesure
    $sql_mesure = "CREATE TABLE mesure (
        id_mesure INT AUTO_INCREMENT PRIMARY KEY,
        id_utilisateur INT NOT NULL,
        poids DECIMAL(5,2),
        spo2 DECIMAL(5,2),
        temperature DECIMAL(4,2),
        rythme_cardiaque INT,
        date_mesure TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        statut VARCHAR(20) DEFAULT 'normal' CHECK (statut IN ('normal','alerte','critique')),
        FOREIGN KEY (id_utilisateur) REFERENCES utilisateur(id_utilisateur) ON DELETE CASCADE
    )";
    
    if ($conn->query($sql_mesure)) {
        echo "<div class='success'>✅ Table 'mesure' créée</div>";
    } else {
        throw new Exception("❌ Erreur création table mesure: " . $conn->error);
    }
    
    echo "</div>";
    
    // Étape 4: Insertion des données de test
    echo "<div class='step'>
        <h2>4. Insertion des données de test</h2>";
    
    // Insertion des utilisateurs
    $users = [
        [1, 'Dupont', 'Jean', '1980-05-15', 43, 'M', '0612345678', 'jean.dupont@email.com', '10 Rue de la Paix, Paris', 'RFID001', 'Marie Dupont - 0698765432', 'Aucun antécédent notable'],
        [2, 'Martin', 'Marie', '1992-08-22', 31, 'F', '0698765432', 'marie.martin@email.com', '25 Avenue des Fleurs, Lyon', 'RFID002', 'Pierre Martin - 0612345678', 'Allergie aux pénicillines'],
        [3, 'Lambert', 'Pierre', '1975-03-10', 48, 'M', '0711223344', 'pierre.lambert@email.com', '15 Rue du Commerce, Marseille', 'RFID003', 'Sophie Lambert - 0755667788', 'Hypertension traitée'],
        [4, 'Dubois', 'Alice', '1988-12-05', 35, 'F', '0755667788', 'alice.dubois@email.com', '42 Boulevard Victor Hugo, Lille', 'RFID004', 'Thomas Dubois - 0601020304', 'Diabète type 2'],
        [5, 'Moreau', 'Thomas', '1965-07-20', 58, 'M', '0601020304', 'thomas.moreau@email.com', '8 Rue de la République, Toulouse', 'RFID005', 'Alice Moreau - 0699887766', 'Asthme contrôlé'],
        [6, 'Garcia', 'Sophie', '1995-11-30', 28, 'F', '0699887766', 'sophie.garcia@email.com', '33 Avenue Charles de Gaulle, Bordeaux', 'RFID006', 'Lucas Garcia - 0788996655', 'Aucun antécédent'],
        [7, 'Petit', 'Lucas', '1983-04-18', 40, 'M', '0788996655', 'lucas.petit@email.com', '12 Place de la Mairie, Nantes', 'RFID007', 'Julie Petit - 0633445566', 'Chirurgie appendicite 2015'],
        [8, 'Roux', 'Julie', '1972-09-25', 51, 'F', '0633445566', 'julie.roux@email.com', '7 Rue des Écoles, Strasbourg', 'RFID008', 'Michel Roux - 0777665544', 'Ostéoporose'],
        [9, 'Leroy', 'Michel', '1958-01-12', 66, 'M', '0777665544', 'michel.leroy@email.com', '20 Avenue Jean Jaurès, Montpellier', 'RFID009', 'Émilie Leroy - 0611223344', 'Cardiopathie ischémique'],
        [10, 'Fournier', 'Émilie', '1990-06-08', 33, 'F', '0611223344', 'emilie.fournier@email.com', '5 Rue de la Gare, Nice', 'RFID010', 'Jean Fournier - 0711223344', 'Grossesse en cours']
    ];
    
    $stmt_user = $conn->prepare("INSERT INTO utilisateur (id_utilisateur, nom, prenom, date_naissance, age, sexe, numero_telephone, email, adresse, rfid_utilisateur, contact_urgence, historique_medical) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    foreach ($users as $user) {
        $stmt_user->bind_param("isssisssssss", ...$user);
        if (!$stmt_user->execute()) {
            throw new Exception("❌ Erreur insertion utilisateur: " . $stmt_user->error);
        }
    }
    echo "<div class='success'>✅ 10 utilisateurs insérés</div>";
    
    // Insertion des cartes RFID
    $cards = [
        ['RFID001', 1],
        ['RFID002', 2],
        ['RFID003', 3],
        ['RFID004', 4],
        ['RFID005', 5],
        ['RFID006', 6],
        ['RFID007', 7],
        ['RFID008', 8],
        ['RFID009', 9],
        ['RFID010', 10]
    ];
    
    $stmt_card = $conn->prepare("INSERT INTO carte (rfid_card, id_utilisateur) VALUES (?, ?)");
    
    foreach ($cards as $card) {
        $stmt_card->bind_param("si", $card[0], $card[1]);
        if (!$stmt_card->execute()) {
            throw new Exception("❌ Erreur insertion carte: " . $stmt_card->error);
        }
    }
    echo "<div class='success'>✅ 10 cartes RFID insérées</div>";
    
    // Insertion des mesures
    $measurements = [
        [1, 75.20, 98.0, 36.80, 72, '2024-01-15 07:30:00', 'normal'],
        [2, 62.80, 99.0, 37.20, 68, '2024-01-15 08:15:00', 'normal'],
        [3, 81.50, 97.0, 36.50, 75, '2024-01-14 13:20:00', 'normal'],
        [4, 58.00, 95.0, 37.80, 82, '2024-01-16 09:45:00', 'alerte'],
        [5, 70.30, 98.0, 36.90, 71, '2024-01-16 10:30:00', 'normal'],
        [6, 65.00, 99.0, 36.70, 69, '2024-01-16 12:15:00', 'normal'],
        [7, 78.50, 92.0, 38.20, 85, '2024-01-16 13:00:00', 'critique'],
        [8, 60.20, 98.0, 36.60, 70, '2024-01-15 15:45:00', 'normal'],
        [9, 83.00, 96.0, 37.10, 78, '2024-01-14 09:30:00', 'normal'],
        [10, 55.80, 99.0, 36.40, 65, '2024-01-16 14:20:00', 'normal'],
        [1, 74.80, 97.0, 36.90, 74, '2024-01-16 16:30:00', 'normal'],
        [2, 63.00, 98.0, 37.00, 70, '2024-01-16 17:15:00', 'normal'],
        [3, 81.20, 96.0, 36.70, 76, '2024-01-16 18:00:00', 'normal']
    ];
    
    $stmt_measure = $conn->prepare("INSERT INTO mesure (id_utilisateur, poids, spo2, temperature, rythme_cardiaque, date_mesure, statut) VALUES (?, ?, ?, ?, ?, ?, ?)");
    
    foreach ($measurements as $measure) {
        $stmt_measure->bind_param("idddiss", $measure[0], $measure[1], $measure[2], $measure[3], $measure[4], $measure[5], $measure[6]);
        if (!$stmt_measure->execute()) {
            throw new Exception("❌ Erreur insertion mesure: " . $stmt_measure->error);
        }
    }
    echo "<div class='success'>✅ " . count($measurements) . " mesures insérées</div>";
    
    echo "</div>";
    
    // Étape 5: Vérification finale
    echo "<div class='step'>
        <h2>5. Vérification finale</h2>";
    
    // Compter les utilisateurs
    $result = $conn->query("SELECT COUNT(*) as total FROM utilisateur");
    $row = $result->fetch_assoc();
    echo "<div class='info'>👥 Utilisateurs: " . $row['total'] . "</div>";
    
    // Compter les cartes
    $result = $conn->query("SELECT COUNT(*) as total FROM carte");
    $row = $result->fetch_assoc();
    echo "<div class='info'>💳 Cartes RFID: " . $row['total'] . "</div>";
    
    // Compter les mesures
    $result = $conn->query("SELECT COUNT(*) as total FROM mesure");
    $row = $result->fetch_assoc();
    echo "<div class='info'>📊 Mesures: " . $row['total'] . "</div>";
    
    // Dernière mesure
    $result = $conn->query("SELECT u.prenom, u.nom, m.temperature, m.spo2, m.rythme_cardiaque, m.date_mesure 
                           FROM mesure m 
                           JOIN utilisateur u ON m.id_utilisateur = u.id_utilisateur 
                           ORDER BY m.date_mesure DESC LIMIT 1");
    if ($row = $result->fetch_assoc()) {
        echo "<div class='info'>🕐 Dernière mesure: " . $row['prenom'] . " " . $row['nom'] . " - " . 
             $row['temperature'] . "°C, " . $row['spo2'] . "% SpO2, " . $row['rythme_cardiaque'] . " bpm</div>";
    }
    
    echo "</div>";
    
    // Message de succès final
    echo "
    <div style='background: #d1fae5; padding: 20px; border-radius: 8px; margin: 20px 0;'>
        <h2 class='success'>🎉 RÉINITIALISATION RÉUSSIE !</h2>
        <p>La base de données a été complètement réinitialisée avec succès.</p>
        <p><strong>Données disponibles:</strong></p>
        <ul>
            <li>10 patients avec informations complètes</li>
            <li>10 cartes RFID associées</li>
            <li>13 mesures de test avec paramètres SpO2</li>
            <li>Statuts de santé variés (normal/alerte/critique)</li>
        </ul>
    </div>
    
    <div style='text-align: center; margin: 30px 0;'>
        <a href='test_apis.php' style='background: #059669; color: white; padding: 12px 24px; text-decoration: none; border-radius: 6px; margin-right: 10px;'>🧪 Tester les APIs</a>
        <a href='../index.html' style='background: #2563eb; color: white; padding: 12px 24px; text-decoration: none; border-radius: 6px;'>🏠 Retour à l'accueil</a>
    </div>
    ";
    
} catch (Exception $e) {
    echo "<div class='error'>❌ ERREUR: " . $e->getMessage() . "</div>";
    echo "<div class='warning'>⚠️ La réinitialisation a échoué. Vérifiez les paramètres de connexion MySQL.</div>";
}

echo "
</body>
</html>";
?>
