-- phpMyAdmin SQL Dump
-- version 5.2.1
-- https://www.phpmyadmin.net/
--
-- Hôte : 127.0.0.1
-- Généré le : lun. 03 nov. 2025 à 21:53
-- Version du serveur : 10.4.28-MariaDB
-- Version de PHP : 8.2.4

SET SQL_MODE = "NO_AUTO_VALUE_ON_ZERO";
START TRANSACTION;
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8mb4 */;

--
-- Base de données : `robot-medical`
--

-- --------------------------------------------------------

--
-- Structure de la table `carte`
--

CREATE TABLE `carte` (
  `rfid_card` varchar(50) NOT NULL,
  `id_utilisateur` int(11) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

--
-- Déchargement des données de la table `carte`
--

INSERT INTO `carte` (`rfid_card`, `id_utilisateur`) VALUES
('RFID001', 1),
('RFID002', 2),
('RFID003', 3),
('RFID004', 4),
('RFID005', 5),
('RFID006', 6),
('RFID007', 7),
('RFID008', 8),
('RFID009', 9),
('RFID010', 10);

-- --------------------------------------------------------

--
-- Structure de la table `mesure`
--

CREATE TABLE `mesure` (
  `id_mesure` int(11) NOT NULL,
  `id_utilisateur` int(11) NOT NULL,
  `poids` decimal(5,2) DEFAULT NULL,
  `temperature` decimal(4,2) DEFAULT NULL,
  `rythme_cardiaque` int(11) DEFAULT NULL,
  `date_mesure` timestamp NOT NULL DEFAULT current_timestamp(),
  `statut` varchar(20) DEFAULT 'normal' CHECK (`statut` in ('normal','alerte','critique'))
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

--
-- Déchargement des données de la table `mesure`
--

INSERT INTO `mesure` (`id_mesure`, `id_utilisateur`, `poids`, `temperature`, `rythme_cardiaque`, `date_mesure`, `statut`) VALUES
(1, 1, 75.20, 36.80, 72, '2024-01-15 06:30:00', 'normal'),
(2, 2, 62.80, 37.20, 68, '2024-01-15 07:15:00', 'normal'),
(3, 3, 81.50, 36.50, 75, '2024-01-14 12:20:00', 'normal'),
(4, 4, 58.00, 37.80, 82, '2024-01-16 08:45:00', 'alerte'),
(5, 5, 70.30, 36.90, 71, '2024-01-16 09:30:00', 'normal'),
(6, 6, 65.00, 36.70, 69, '2024-01-16 11:15:00', 'normal'),
(7, 7, 78.50, 38.20, 85, '2024-01-16 12:00:00', 'critique'),
(8, 8, 60.20, 36.60, 70, '2024-01-15 14:45:00', 'normal'),
(9, 9, 83.00, 37.10, 78, '2024-01-14 08:30:00', 'normal'),
(10, 10, 55.80, 36.40, 65, '2024-01-16 13:20:00', 'normal');

-- --------------------------------------------------------

--
-- Structure de la table `utilisateur`
--

CREATE TABLE `utilisateur` (
  `id_utilisateur` int(11) NOT NULL,
  `nom` varchar(100) NOT NULL,
  `prenom` varchar(100) NOT NULL,
  `date_naissance` date DEFAULT NULL,
  `age` int(11) NOT NULL,
  `sexe` char(1) DEFAULT NULL CHECK (`sexe` in ('M','F')),
  `numero_telephone` varchar(20) DEFAULT NULL,
  `email` varchar(100) DEFAULT NULL,
  `adresse` text DEFAULT NULL,
  `rfid_utilisateur` varchar(50) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1 COLLATE=latin1_swedish_ci;

--
-- Déchargement des données de la table `utilisateur`
--

INSERT INTO `utilisateur` (`id_utilisateur`, `nom`, `prenom`, `date_naissance`, `age`, `sexe`, `numero_telephone`, `email`, `adresse`, `rfid_utilisateur`) VALUES
(1, 'Dupont', 'Jean', '1980-05-15', 43, 'M', '0612345678', 'jean.dupont@email.com', '10 Rue de la Paix, Paris', 'RFID001'),
(2, 'Martin', 'Marie', '1992-08-22', 31, 'F', '0698765432', 'marie.martin@email.com', '25 Avenue des Fleurs, Lyon', 'RFID002'),
(3, 'Lambert', 'Pierre', '1975-03-10', 48, 'M', '0711223344', 'pierre.lambert@email.com', '15 Rue du Commerce, Marseille', 'RFID003'),
(4, 'Dubois', 'Alice', '1988-12-05', 35, 'F', '0755667788', 'alice.dubois@email.com', '42 Boulevard Victor Hugo, Lille', 'RFID004'),
(5, 'Moreau', 'Thomas', '1965-07-20', 58, 'M', '0601020304', 'thomas.moreau@email.com', '8 Rue de la République, Toulouse', 'RFID005'),
(6, 'Garcia', 'Sophie', '1995-11-30', 28, 'F', '0699887766', 'sophie.garcia@email.com', '33 Avenue Charles de Gaulle, Bordeaux', 'RFID006'),
(7, 'Petit', 'Lucas', '1983-04-18', 40, 'M', '0788996655', 'lucas.petit@email.com', '12 Place de la Mairie, Nantes', 'RFID007'),
(8, 'Roux', 'Julie', '1972-09-25', 51, 'F', '0633445566', 'julie.roux@email.com', '7 Rue des Écoles, Strasbourg', 'RFID008'),
(9, 'Leroy', 'Michel', '1958-01-12', 66, 'M', '0777665544', 'michel.leroy@email.com', '20 Avenue Jean Jaurès, Montpellier', 'RFID009'),
(10, 'Fournier', 'Émilie', '1990-06-08', 33, 'F', '0611223344', 'emilie.fournier@email.com', '5 Rue de la Gare, Nice', 'RFID010');

--
-- Index pour les tables déchargées
--

--
-- Index pour la table `carte`
--
ALTER TABLE `carte`
  ADD PRIMARY KEY (`rfid_card`),
  ADD UNIQUE KEY `id_utilisateur` (`id_utilisateur`);

--
-- Index pour la table `mesure`
--
ALTER TABLE `mesure`
  ADD PRIMARY KEY (`id_mesure`),
  ADD KEY `id_utilisateur` (`id_utilisateur`);

--
-- Index pour la table `utilisateur`
--
ALTER TABLE `utilisateur`
  ADD PRIMARY KEY (`id_utilisateur`),
  ADD UNIQUE KEY `rfid_utilisateur` (`rfid_utilisateur`);

--
-- AUTO_INCREMENT pour les tables déchargées
--

--
-- AUTO_INCREMENT pour la table `mesure`
--
ALTER TABLE `mesure`
  MODIFY `id_mesure` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=11;

--
-- AUTO_INCREMENT pour la table `utilisateur`
--
ALTER TABLE `utilisateur`
  MODIFY `id_utilisateur` int(11) NOT NULL AUTO_INCREMENT, AUTO_INCREMENT=11;

--
-- Contraintes pour les tables déchargées
--

--
-- Contraintes pour la table `carte`
--
ALTER TABLE `carte`
  ADD CONSTRAINT `carte_ibfk_1` FOREIGN KEY (`id_utilisateur`) REFERENCES `utilisateur` (`id_utilisateur`) ON DELETE CASCADE;

--
-- Contraintes pour la table `mesure`
--
ALTER TABLE `mesure`
  ADD CONSTRAINT `mesure_ibfk_1` FOREIGN KEY (`id_utilisateur`) REFERENCES `utilisateur` (`id_utilisateur`) ON DELETE CASCADE;
COMMIT;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
