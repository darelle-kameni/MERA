// server-simple.js - VERSION ULTRA SIMPLE
import express from "express";
import mysql from "mysql2";
import cors from "cors";

const app = express();
const PORT = 3001;

// Middleware basique
app.use(cors());
app.use(express.json());

// Connexion MySQL simple
const db = mysql.createConnection({
  host: '127.0.0.1',
  user: 'root',
  password: '',
  database: 'robot-medical'
});

// Test de connexion
db.connect((err) => {
  if (err) {
    console.error('❌ Erreur MySQL:', err.message);
    return;
  }
  console.log('✅ MySQL connecté!');
});

// Route racine TRÈS SIMPLE
app.get("/", (req, res) => {
  res.json({ 
    message: "🤖 API Robot Médical - SERVEUR ACTIF",
    status: "success",
    timestamp: new Date().toISOString()
  });
});

// Route utilisateurs simple
app.get("/utilisateurs", (req, res) => {
  db.query("SELECT id_utilisateur, nom, prenom FROM utilisateur LIMIT 5", (err, results) => {
    if (err) {
      console.error('Erreur DB:', err);
      return res.status(500).json({ error: 'Erreur base de données' });
    }
    res.json({
      status: 'success',
      utilisateurs: results
    });
  });
});

// Route mesures simple
app.get("/mesures", (req, res) => {
  db.query("SELECT * FROM mesure ORDER BY date_mesure DESC LIMIT 5", (err, results) => {
    if (err) {
      console.error('Erreur DB:', err);
      return res.status(500).json({ error: 'Erreur base de données' });
    }
    res.json({
      status: 'success',
      mesures: results
    });
  });
});

// Route POST simplifiée
app.post("/mesures", (req, res) => {
  console.log('📦 Données reçues:', req.body);
  
  const { id_utilisateur, poids, temperature, rythme_cardiaque } = req.body;
  
  // Validation simple
  if (!id_utilisateur) {
    return res.status(400).json({ error: 'id_utilisateur manquant' });
  }
  
  const sql = `INSERT INTO mesure (id_utilisateur, poids, temperature, rythme_cardiaque) VALUES (?, ?, ?, ?)`;
  
  db.execute(sql, [id_utilisateur, poids, temperature, rythme_cardiaque], (err, results) => {
    if (err) {
      console.error('❌ Erreur insertion:', err);
      return res.status(500).json({ error: 'Erreur insertion' });
    }
    
    res.json({
      status: 'success',
      message: 'Mesure enregistrée',
      id_mesure: results.insertId
    });
  });
});

// Gestion des erreurs globales
app.use((err, req, res, next) => {
  console.error('💥 Erreur serveur:', err);
  res.status(500).json({ error: 'Erreur interne du serveur' });
});

// Démarrer le serveur
app.listen(PORT, () => {
  console.log('🎯 ================================');
  console.log('🚀 SERVEUR SIMPLE DÉMARRÉ');
  console.log(`📍 http://localhost:${PORT}`);
  console.log('🎯 ================================');
});