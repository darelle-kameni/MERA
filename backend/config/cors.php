<?php
/**
 * Configuration CORS centralisée
 */
function setupCors() {
    // Headers CORS
    header('Content-Type: application/json');
    header('Access-Control-Allow-Origin: http://localhost:5173, http://192.168.1.127:5173');
    header('Access-Control-Allow-Credentials: true');
    header('Access-Control-Allow-Headers: Content-Type, Accept, Authorization, X-Requested-With, X-API-Key');
    header('Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS');
    
    // Headers de sécurité
    header('X-Content-Type-Options: nosniff');
    header('X-Frame-Options: DENY');
    header('X-XSS-Protection: 1; mode=block');
    
    if ($_SERVER['REQUEST_METHOD'] == 'OPTIONS') {
        http_response_code(200);
        exit();
    }
}
?>
