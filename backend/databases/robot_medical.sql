-- phpMyAdmin SQL Dump
-- version 5.1.1
-- https://www.phpmyadmin.net/
--
-- Host: 127.0.0.1
-- Generation Time: Nov 07, 2025 at 07:05 PM
-- Server version: 10.4.21-MariaDB
-- PHP Version: 8.0.10

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Database: robot_medical
--

-- --------------------------------------------------------

--
-- Table structure for table carte
--

CREATE TABLE carte (
  rfid_card varchar(50) NOT NULL,
  id_utilisateur int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table carte
--

INSERT INTO carte (rfid_card, id_utilisateur) VALUES
('RFID001', 1),
('RFID002', 2),
('RFID003', 3),
('RFID004', 4),
('RFID005', 5),
('RFID006', 6),
('RFID007', 7),
('RFID008', 8),
('RFID009', 9),
('RFID010', 10),
('RFID0014', 13),
('RFID0019', 14),
('RFID0020', 16),
('RFID0022', 17),
('RFID2233', 18);

-- --------------------------------------------------------

--
-- Table structure for table mesure
--

CREATE TABLE mesure (
  id_mesure int(11) NOT NULL,
  id_utilisateur int(11) NOT NULL,
  poids decimal(5,2) DEFAULT NULL,
  spo2 decimal(5,2) DEFAULT NULL,
  temperature decimal(4,2) DEFAULT NULL,
  rythme_cardiaque int(11) DEFAULT NULL,
  date_mesure timestamp NOT NULL DEFAULT current_timestamp(),
  statut varchar(20) DEFAULT 'normal' CHECK (statut in ('normal','alerte','critique')),
  oeil_gauche varchar(100) DEFAULT NULL COMMENT 'Diagnostic œil gauche',
  oeil_gauche_confiance decimal(5,4) DEFAULT NULL COMMENT 'Confiance IA (0.0-1.0)',
  oeil_droit varchar(100) DEFAULT NULL COMMENT 'Diagnostic œil droit',
  oeil_droit_confiance decimal(5,4) DEFAULT NULL COMMENT 'Confiance IA (0.0-1.0)',
  alerte_oculaire tinyint(1) DEFAULT 0 COMMENT '1 si anomalie détectée',
  photo_path varchar(255) DEFAULT NULL COMMENT 'Chemin photo annotée'
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table mesure
--

INSERT INTO mesure (id_mesure, id_utilisateur, poids, spo2, temperature, rythme_cardiaque, date_mesure, statut) VALUES
(1, 1, '75.20', NULL, '36.80', 72, '2024-01-15 07:30:00', 'normal'),
(2, 2, '62.80', NULL, '37.20', 68, '2024-01-15 08:15:00', 'normal'),
(3, 3, '81.50', NULL, '36.50', 75, '2024-01-14 13:20:00', 'normal'),
(4, 4, '58.00', NULL, '37.80', 82, '2024-01-16 09:45:00', 'alerte'),
(5, 5, '70.30', NULL, '36.90', 71, '2024-01-16 10:30:00', 'normal'),
(6, 6, '65.00', NULL, '36.70', 69, '2024-01-16 12:15:00', 'normal'),
(7, 7, '78.50', NULL, '38.20', 85, '2024-01-16 13:00:00', 'critique'),
(8, 8, '60.20', NULL, '36.60', 70, '2024-01-15 15:45:00', 'normal'),
(9, 9, '83.00', NULL, '37.10', 78, '2024-01-14 09:30:00', 'normal'),
(10, 10, '55.80', NULL, '36.40', 65, '2024-01-16 14:20:00', 'normal'),
(11, 13, '55.00', NULL, '37.00', 80, '2025-11-07 04:25:53', 'normal'),
(12, 14, '100.00', NULL, '40.00', 99, '2025-11-07 04:27:29', 'normal'),
(13, 1, '70.50', '98.00', '37.20', 75, '2025-11-07 09:54:23', 'normal'),
(14, 16, '70.00', '98.00', '35.00', 75, '2025-11-07 10:15:18', 'alerte'),
(15, 16, '70.00', '98.00', '35.00', 75, '2025-11-07 10:16:09', 'alerte'),
(16, 16, '70.00', '98.00', '35.00', 75, '2025-11-07 10:16:14', 'alerte'),
(17, 14, '56.00', '99.00', '37.00', 112, '2025-11-07 10:20:17', 'critique'),
(18, 8, '72.00', '70.00', '37.00', 30, '2025-11-07 12:34:25', 'critique');

-- --------------------------------------------------------

--
-- Table structure for table utilisateur
--

CREATE TABLE utilisateur (
  id_utilisateur int(11) NOT NULL,
  nom varchar(100) NOT NULL,
  prenom varchar(100) NOT NULL,
  date_naissance date DEFAULT NULL,
  age int(11) NOT NULL,
  sexe char(1) DEFAULT NULL CHECK (sexe in ('M','F')),
  numero_telephone varchar(20) DEFAULT NULL,
  email varchar(100) DEFAULT NULL,
  adresse text DEFAULT NULL,
  rfid_utilisateur varchar(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

--
-- Dumping data for table utilisateur
--

INSERT INTO utilisateur (id_utilisateur, nom, prenom, date_naissance, age, sexe, numero_telephone, email, adresse, rfid_utilisateur) VALUES
(1, 'Dupont', 'Jean', '1980-05-15', 43, 'M', '0612345678', 'jean.dupont@email.com', '10 Rue de la Paix, Paris', 'RFID001'),
(2, 'Martin', 'Marie', '1992-08-22', 31, 'F', '0698765432', 'marie.martin@email.com', '25 Avenue des Fleurs, Lyon', 'RFID002'),
(3, 'Lambert', 'Pierre', '1975-03-10', 48, 'M', '0711223344', 'pierre.lambert@email.com', '15 Rue du Commerce, Marseille', 'RFID003'),
(4, 'Dubois', 'Alice', '1988-12-05', 35, 'F', '0755667788', 'alice.dubois@email.com', '42 Boulevard Victor Hugo, Lille', 'RFID004'),
(5, 'Moreau', 'Thomas', '1965-07-20', 58, 'M', '0601020304', 'thomas.moreau@email.com', '8 Rue de la République, Toulouse', 'RFID005'),
(6, 'Garcia', 'Sophie', '1995-11-30', 28, 'F', '0699887766', 'sophie.garcia@email.com', '33 Avenue Charles de Gaulle, Bordeaux', 'RFID006'),
(7, 'Petit', 'Lucas', '1983-04-18', 40, 'M', '0788996655', 'lucas.petit@email.com', '12 Place de la Mairie, Nantes', 'RFID007'),
(8, 'Roux', 'Julie', '1972-09-25', 51, 'F', '0633445566', 'julie.roux@email.com', '7 Rue des Écoles, Strasbourg', 'RFID008'),
(9, 'Leroy', 'Michel', '1958-01-12', 66, 'M', '0777665544', 'michel.leroy@email.com', '20 Avenue Jean Jaurès, Montpellier', 'RFID009'),
(10, 'Fournier', 'Émilie', '1990-06-08', 33, 'F', '0611223344', 'emilie.fournier@email.com', '5 Rue de la Gare, Nice', 'RFID010'),
(13, 'Egoume', 'Daphnée', '2003-01-01', 22, NULL, '123456789', 'egoume@gmail.com', 'PK17', 'RFID0014'),
(14, 'DOMCHE', 'FREEDY', '1992-01-01', 33, NULL, '', 'domche@gmail.com', '', 'RFID0019'),
(16, 'AMBO\'O', 'Junette', '2007-01-01', 18, NULL, '', 'amboo@gmail.com', '', 'RFID0020'),
(17, 'NYOUMA', 'SAMUEL', '2000-01-01', 25, NULL, '', '', '', 'RFID0022'),
(18, 'TIDO', 'Leal', '2006-01-01', 19, NULL, '', '', '', 'RFID2233');

--
-- Indexes for dumped tables
--

--
-- Indexes for table carte
--
ALTER TABLE carte
  ADD PRIMARY KEY (rfid_card),
  ADD UNIQUE KEY id_utilisateur (id_utilisateur);

--
-- Indexes for table mesure
--
ALTER TABLE mesure
  ADD PRIMARY KEY (id_mesure),
  ADD KEY id_utilisateur (id_utilisateur),
  ADD KEY idx_alerte_oculaire (alerte_oculaire, date_mesure);

--
-- Indexes for table utilisateur
--
ALTER TABLE utilisateur
  ADD PRIMARY KEY (id_utilisateur),
  ADD UNIQUE KEY rfid_utilisateur (rfid_utilisateur);

--
-- AUTO_INCREMENT for dumped tables
--

--
-- AUTO_INCREMENT for table mesure
--
ALTER TABLE mesure
  MODIFY id_mesure int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=19;

--
-- AUTO_INCREMENT for table utilisateur
--
ALTER TABLE utilisateur
  MODIFY id_utilisateur int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=19;

--
-- Constraints for dumped tables
--

--
-- Constraints for table carte
--
ALTER TABLE carte
  ADD CONSTRAINT carte_ibfk_1 FOREIGN KEY (id_utilisateur) REFERENCES utilisateur (id_utilisateur) ON DELETE CASCADE;

--
-- Constraints for table mesure
--
ALTER TABLE mesure
  ADD CONSTRAINT mesure_ibfk_1 FOREIGN KEY (id_utilisateur) REFERENCES utilisateur (id_utilisateur) ON DELETE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;