<?php
/**
 * CONFIGURATION GÉNÉRALE DE L'APPLICATION
 * Paramètres globaux et constantes
 */

// Mode de développement (true = développement, false = production)
define('DEBUG_MODE', true);

// Configuration de la base de données
define('DB_HOST', 'localhost');
define('DB_NAME', 'robot_medical');
define('DB_USER', 'dark-linux');
define('DB_PASS', '');
define('DB_CHARSET', 'utf8mb4');

// Configuration de l'API
define('API_VERSION', 'v1');
define('ITEMS_PER_PAGE', 20);

// Chemins des dossiers
define('BASE_PATH', dirname(dirname(__FILE__)));
define('UPLOAD_PATH', BASE_PATH . '/uploads/');

// Configuration des logs
if (DEBUG_MODE) {
    ini_set('display_errors', 1);
    ini_set('display_startup_errors', 1);
    error_reporting(E_ALL);
} else {
    ini_set('display_errors', 0);
    ini_set('display_startup_errors', 0);
    error_reporting(0);
}

// Début de la session si nécessaire
if (session_status() == PHP_SESSION_NONE) {
    session_start();
}

/**
 * Fonction utilitaire pour le logging
 */
function log_message($message, $level = 'INFO') {
    $timestamp = date('Y-m-d H:i:s');
    $log_entry = "[$timestamp] [$level] $message" . PHP_EOL;
    
    if (DEBUG_MODE) {
        error_log($log_entry);
    }
    
    // Écriture dans un fichier log (avec gestion d'erreur si le dossier n'existe pas)
    $log_file = BASE_PATH . '/logs/app.log';
    try {
        file_put_contents($log_file, $log_entry, FILE_APPEND | LOCK_EX);
    } catch (Exception $e) {
        // Si le fichier log n'est pas accessible, on log seulement dans error_log
        error_log("Impossible d'écrire dans le fichier log: " . $e->getMessage());
    }
}

// Message de démarrage
if (DEBUG_MODE) {
    log_message("Application Medical Robot démarrée en mode développement");
}
?>
