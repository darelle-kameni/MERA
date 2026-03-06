<?php
/**
 * CONFIGURATION BASE DE DONNÉES MYSQL
 * Gère la connexion à la base medical_robot
 */
class Database {
    // Paramètres de connexion MySQL
    private $host = "localhost";
    private $db_name = "robot_medical";  // Nom la bd 
    private $username = "dark-linux";    // Utilisateur système
    private $password = "";              // Mot de passe (vide pour auth socket)
    public $conn;

    /**
     * Établir la connexion à MySQL
     * @return PDO Object connexion
     */
    public function getConnection() {
        $this->conn = null;
        
        try {
            // Création de la connexion PDO
            $this->conn = new PDO(
                "mysql:host=" . $this->host . ";dbname=" . $this->db_name . ";charset=utf8mb4",
                $this->username, 
                $this->password,
                [
                    // Options importantes pour la sécurité et les performances
                    PDO::ATTR_ERRMODE => PDO::ERRMODE_EXCEPTION,      // Génère des exceptions en cas d'erreur
                    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC, // Retourne des tableaux associatifs
                    PDO::ATTR_EMULATE_PREPARES => false               // Désactive l'émulation des requêtes préparées
                ]
            );
            
            // Message de succès (seulement en développement)
            error_log("✅ Connexion MySQL réussie à: " . $this->db_name);
            
        } catch(PDOException $exception) {
            // Journalisation de l'erreur
            error_log("❌ Erreur de connexion MySQL: " . $exception->getMessage());
            
            // Réponse JSON claire pour React
            http_response_code(500);
            echo json_encode([
                "success" => false,
                "message" => "Service de base de données indisponible",
                "error" => "Impossible de se connecter à MySQL"
            ]);
            exit;
        }
        
        return $this->conn;
    }
    
    /**
     * Fermer la connexion
     */
    public function closeConnection() {
        $this->conn = null;
    }
}

/**
 * Fonction utilitaire pour obtenir une connexion à la base de données
 * Compatible avec les fichiers existants qui utilisent getDatabaseConnection()
 */
function getDatabaseConnection() {
    $database = new Database();
    return $database->getConnection();
}
?>
