import React, { useState } from 'react';
import { Download, FileText, BarChart3, Send, Calendar, Users, AlertTriangle } from 'lucide-react';
import DynamicBarChart from '../components/charts/dynamic-barchart';

const Reports = () => {
  const [loading, setLoading] = useState({});
  const [recentReports, setRecentReports] = useState([
    { 
      name: 'Rapport Hebdomadaire - Semaine 02', 
      date: new Date('2024-01-15'), 
      type: 'weekly',
      size: '2.3 MB'
    },
    { 
      name: 'Analyse Mensuelle - Décembre', 
      date: new Date('2024-01-01'), 
      type: 'monthly',
      size: '5.1 MB'
    },
    { 
      name: 'Rapport des Alertes - Janvier', 
      date: new Date('2024-01-10'), 
      type: 'alerts',
      size: '1.8 MB'
    },
  ]);

  const reportTemplates = [
    { 
      name: 'Weekly Report', 
      description: 'Complete summary of weekly measurements', 
      icon: FileText,
      type: 'weekly'
    },
    { 
      name: 'Trends Analysis', 
      description: 'Parameters evolution over 30 days', 
      icon: BarChart3,
      type: 'trends'
    },
    { 
      name: 'Alerts Report', 
      description: 'Summary of alerts and incidents', 
      icon: AlertTriangle,
      type: 'alerts'
    },
  ];

  // Fonction pour générer un rapport
  const generateReport = async (reportType, format = 'pdf') => {
    setLoading(prev => ({ ...prev, [reportType]: true }));

    try {
      // Simulation de délai de génération
      await new Promise(resolve => setTimeout(resolve, 2000));

      let content, mimeType, filename;

      // Données simulées selon le type de rapport
      const reportData = getReportData(reportType);

      switch (format) {
        case 'pdf':
          // Génération d'un vrai PDF
          content = generateReportPDF(reportData, reportType);
          mimeType = 'application/pdf';
          filename = `rapport_${reportType}_${new Date().toISOString().slice(0, 10)}.pdf`;
          break;
        
        case 'csv':
          content = generateReportCSV(reportData, reportType);
          mimeType = 'text/csv;charset=utf-8;';
          filename = `rapport_${reportType}_${new Date().toISOString().slice(0, 10)}.csv`;
          break;
        
        case 'json':
          content = JSON.stringify(reportData, null, 2);
          mimeType = 'application/json';
          filename = `rapport_${reportType}_${new Date().toISOString().slice(0, 10)}.json`;
          break;
        
        case 'excel':
          content = generateReportCSV(reportData, reportType);
          mimeType = 'application/vnd.ms-excel';
          filename = `rapport_${reportType}_${new Date().toISOString().slice(0, 10)}.xls`;
          break;
        
        default:
          content = generateReportCSV(reportData, reportType);
          mimeType = 'text/csv;charset=utf-8;';
          filename = `rapport_${reportType}_${new Date().toISOString().slice(0, 10)}.csv`;
      }

      // Création et téléchargement du fichier
      const blob = new Blob([content], { type: mimeType });
      const url = window.URL.createObjectURL(blob);
      const link = document.createElement('a');
      link.href = url;
      link.download = filename;
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
      window.URL.revokeObjectURL(url);

      // Ajouter aux rapports récents
      const newReport = {
        name: `${getReportTitle(reportType)} - ${new Date().toLocaleDateString('fr-FR', { month: 'long', year: 'numeric' })}`,
        date: new Date(),
        type: reportType,
        size: `${(content.length / 1024 / 1024).toFixed(1)} MB`
      };

      setRecentReports(prev => [newReport, ...prev.slice(0, 2)]);

      // Message de succès
      alert(`Report "${getReportTitle(reportType)}" generated successfully!`);

    } catch (error) {
      console.error('Erreur lors de la génération du rapport:', error);
      alert('Error generating report. Please try again.');
    } finally {
      setLoading(prev => ({ ...prev, [reportType]: false }));
    }
  };


  // Fonction pour générer un PDF simple
  const generateReportPDF = (data, reportType) => {
    // Création d'un PDF simple en utilisant une approche HTML
    const htmlContent = `
      <!DOCTYPE html>
      <html>
      <head>
        <meta charset="UTF-8">
        <title>${getReportTitle(reportType)}</title>
        <style>
          body { font-family: Arial, sans-serif; margin: 40px; line-height: 1.6; }
          h1 { color: #2563eb; border-bottom: 2px solid #2563eb; padding-bottom: 10px; text-align: center; }
          h2 { color: #374151; margin-top: 30px; border-bottom: 1px solid #e5e7eb; padding-bottom: 5px; }
          table { width: 100%; border-collapse: collapse; margin: 20px 0; font-size: 12px; }
          th, td { border: 1px solid #d1d5db; padding: 8px; text-align: left; }
          th { background-color: #f3f4f6; font-weight: bold; }
          .header { text-align: center; margin-bottom: 30px; }
          .footer { margin-top: 50px; text-align: center; color: #6b7280; font-size: 10px; }
          .info-section { margin-bottom: 20px; }
          .info-label { font-weight: bold; color: #374151; }
        </style>
      </head>
      <body>
        <div class="header">
          <h1>ROBOT MÉDICAL - ${getReportTitle(reportType).toUpperCase()}</h1>
          <p>Généré le ${new Date().toLocaleDateString('fr-FR')}</p>
        </div>
        
        ${generatePDFContent(data, reportType)}
        
        <div class="footer">
          <p>Document généré automatiquement par le système Robot Médical</p>
          <p>Confidentiel - Usage médical uniquement</p>
        </div>
      </body>
      </html>
    `;
    
    return htmlContent;
  };

  // Fonction pour générer le contenu PDF selon le type de rapport
  const generatePDFContent = (data, reportType) => {
    switch (reportType) {
      case 'weekly':
        return `
          <div class="info-section">
            <h2>Informations Générales</h2>
            <p><span class="info-label">Période:</span> ${data.period}</p>
            <p><span class="info-label">Nombre de patients:</span> ${data.patientCount}</p>
            <p><span class="info-label">Total des mesures:</span> ${data.totalMeasurements}</p>
            <p><span class="info-label">Alertes détectées:</span> ${data.alerts}</p>
          </div>
          
          <h2>Mesures par Jour</h2>
          <table>
            <tr>
              <th>Jour</th>
              <th>Mesures</th>
              <th>Température Moyenne</th>
              <th>Rythme Cardiaque Moyen</th>
              <th>SpO2 Moyen</th>
            </tr>
            ${data.dailyStats.map(day => `
              <tr>
                <td>${day.day}</td>
                <td>${day.measurements}</td>
                <td>${day.avgTemp}°C</td>
                <td>${day.avgHeartRate} bpm</td>
                <td>${day.avgSpO2}%</td>
              </tr>
            `).join('')}
          </table>
          
          <h2>Alertes Détectées</h2>
          <table>
            <tr>
              <th>Date</th>
              <th>Patient</th>
              <th>Paramètre</th>
              <th>Valeur</th>
              <th>Statut</th>
            </tr>
            ${data.alertsList.map(alert => `
              <tr>
                <td>${alert.date}</td>
                <td>${alert.patient}</td>
                <td>${alert.parameter}</td>
                <td>${alert.value}</td>
                <td>${alert.status}</td>
              </tr>
            `).join('')}
          </table>
        `;

      case 'trends':
        return `
          <div class="info-section">
            <h2>Analyse des Tendances</h2>
            <p><span class="info-label">Période:</span> ${data.period}</p>
            <p><span class="info-label">Échantillon:</span> ${data.sampleSize} patients</p>
          </div>
          
          <h2>Tendances des Paramètres</h2>
          <table>
            <tr>
              <th>Paramètre</th>
              <th>Valeur Moyenne</th>
              <th>Évolution</th>
              <th>Tendance</th>
            </tr>
            ${data.trends.map(trend => `
              <tr>
                <td>${trend.parameter}</td>
                <td>${trend.average}</td>
                <td>${trend.evolution}</td>
                <td>${trend.trend}</td>
              </tr>
            `).join('')}
          </table>
          
          <h2>Variations Hebdomadaires</h2>
          <table>
            <tr>
              <th>Semaine</th>
              <th>Mesures</th>
              <th>Alertes</th>
              <th>Taux d'activité</th>
            </tr>
            ${data.weeklyVariations.map(week => `
              <tr>
                <td>${week.week}</td>
                <td>${week.measurements}</td>
                <td>${week.alerts}</td>
                <td>${week.activityRate}</td>
              </tr>
            `).join('')}
          </table>
        `;

      case 'alerts':
        return `
          <div class="info-section">
            <h2>Rapport des Alertes Médicales</h2>
            <p><span class="info-label">Période:</span> ${data.period}</p>
            <p><span class="info-label">Alertes totales:</span> ${data.totalAlerts}</p>
            <p><span class="info-label">Alertes critiques:</span> ${data.criticalAlerts}</p>
          </div>
          
          <h2>Détail des Alertes</h2>
          <table>
            <tr>
              <th>Date</th>
              <th>Heure</th>
              <th>Patient</th>
              <th>Type d'alerte</th>
              <th>Paramètre</th>
              <th>Valeur</th>
              <th>Seuil</th>
              <th>Gravité</th>
              <th>Action</th>
            </tr>
            ${data.alertDetails.map(alert => `
              <tr>
                <td>${alert.date}</td>
                <td>${alert.time}</td>
                <td>${alert.patient}</td>
                <td>${alert.type}</td>
                <td>${alert.parameter}</td>
                <td>${alert.value}</td>
                <td>${alert.threshold}</td>
                <td>${alert.severity}</td>
                <td>${alert.action}</td>
              </tr>
            `).join('')}
          </table>
          
          <h2>Synthèse par Type</h2>
          <table>
            <tr>
              <th>Type d'alerte</th>
              <th>Nombre</th>
              <th>Pourcentage</th>
            </tr>
            ${data.alertSummary.map(summary => `
              <tr>
                <td>${summary.type}</td>
                <td>${summary.count}</td>
                <td>${summary.percentage}%</td>
              </tr>
            `).join('')}
          </table>
        `;

      default:
        return `
          <h2>Rapport Générique</h2>
          <table>
            <tr>
              <th>Donnée</th>
              <th>Valeur</th>
            </tr>
            ${Object.entries(data).map(([key, value]) => `
              <tr>
                <td>${key}</td>
                <td>${value}</td>
              </tr>
            `).join('')}
          </table>
        `;
    }
  };

  // Fonction pour envoyer un rapport par email
  const sendReport = (reportType) => {
    const email = window.prompt('Enter email address to send the report:');
    if (email) {
      // Simulation of sending
      setTimeout(() => {
        alert(`Report "${getReportTitle(reportType)}" sent to ${email}`);
      }, 1000);
    }
  };

  // Fonction pour générer le CSV selon le type de rapport
  const generateReportCSV = (data, reportType) => {
    let csv = '';

    switch (reportType) {
      case 'weekly':
        csv += 'RAPPORT HEBDOMADAIRE - ROBOT MÉDICAL\n\n';
        csv += `Période: ${data.period}\n`;
        csv += `Nombre de patients: ${data.patientCount}\n`;
        csv += `Total des mesures: ${data.totalMeasurements}\n`;
        csv += `Alertes: ${data.alerts}\n\n`;
        
        csv += 'MESURES PAR JOUR\n';
        csv += 'Jour,Mesures,Température Moyenne,Rythme Cardiaque Moyen,SpO2 Moyen\n';
        data.dailyStats.forEach(day => {
          csv += `${day.day},${day.measurements},${day.avgTemp},${day.avgHeartRate},${day.avgSpO2}\n`;
        });
        
        csv += '\nALERTES DÉTECTÉES\n';
        csv += 'Date,Patient,Paramètre,Valeur,Statut\n';
        data.alertsList.forEach(alert => {
          csv += `"${alert.date}",${alert.patient},${alert.parameter},${alert.value},${alert.status}\n`;
        });
        break;

      case 'trends':
        csv += 'ANALYSE DES TENDANCES - 30 JOURS\n\n';
        csv += `Période: ${data.period}\n`;
        csv += `Échantillon: ${data.sampleSize} patients\n\n`;
        
        csv += 'TENDANCES DES PARAMÈTRES\n';
        csv += 'Paramètre,Valeur Moyenne,Évolution,Tendance\n';
        data.trends.forEach(trend => {
          csv += `${trend.parameter},${trend.average},${trend.evolution},${trend.trend}\n`;
        });
        
        csv += '\nVARIATIONS HEBDOMADAIRES\n';
        csv += 'Semaine,Mesures,Alertes,Taux d\'activité\n';
        data.weeklyVariations.forEach(week => {
          csv += `${week.week},${week.measurements},${week.alerts},${week.activityRate}\n`;
        });
        break;

      case 'alerts':
        csv += 'RAPPORT DES ALERTES MÉDICALES\n\n';
        csv += `Période: ${data.period}\n`;
        csv += `Alertes totales: ${data.totalAlerts}\n`;
        csv += `Alertes critiques: ${data.criticalAlerts}\n\n`;
        
        csv += 'DÉTAIL DES ALERTES\n';
        csv += 'Date,Heure,Patient,Type d\'alerte,Paramètre,Valeur,Seuil,Gravité,Action\n';
        data.alertDetails.forEach(alert => {
          csv += `"${alert.date}","${alert.time}",${alert.patient},${alert.type},${alert.parameter},${alert.value},${alert.threshold},${alert.severity},${alert.action}\n`;
        });
        
        csv += '\nSYNTHÈSE PAR TYPE\n';
        csv += 'Type d\'alerte,Nombre,Pourcentage\n';
        data.alertSummary.forEach(summary => {
          csv += `${summary.type},${summary.count},${summary.percentage}\n`;
        });
        break;

      default:
        csv += 'RAPPORT GÉNÉRIQUE\n';
        csv += 'Donnée,Valeur\n';
        Object.entries(data).forEach(([key, value]) => {
          csv += `${key},${value}\n`;
        });
    }

    return csv;
  };

  // Données simulées pour les rapports
  const getReportData = (reportType) => {
    const baseData = {
      period: `${new Date(Date.now() - 7 * 24 * 60 * 60 * 1000).toLocaleDateString()} au ${new Date().toLocaleDateString()}`,
      generated: new Date().toISOString()
    };

    switch (reportType) {
      case 'weekly':
        return {
          ...baseData,
          patientCount: 142,
          totalMeasurements: 1248,
          alerts: 23,
          dailyStats: [
            { day: 'Lundi', measurements: 189, avgTemp: 36.8, avgHeartRate: 72, avgSpO2: 97 },
            { day: 'Mardi', measurements: 204, avgTemp: 36.9, avgHeartRate: 74, avgSpO2: 96 },
            { day: 'Mercredi', measurements: 198, avgTemp: 36.7, avgHeartRate: 71, avgSpO2: 98 },
            { day: 'Jeudi', measurements: 215, avgTemp: 37.0, avgHeartRate: 76, avgSpO2: 95 },
            { day: 'Vendredi', measurements: 192, avgTemp: 36.8, avgHeartRate: 73, avgSpO2: 97 },
            { day: 'Samedi', measurements: 156, avgTemp: 36.9, avgHeartRate: 75, avgSpO2: 96 },
            { day: 'Dimanche', measurements: 94, avgTemp: 36.7, avgHeartRate: 70, avgSpO2: 98 },
          ],
          alertsList: [
            { date: '2024-01-15', patient: 'DUPONT Jean', parameter: 'Température', value: '38.5°C', status: 'Élevée' },
            { date: '2024-01-14', patient: 'MARTIN Marie', parameter: 'Rythme cardiaque', value: '95 bpm', status: 'Accéléré' },
            { date: '2024-01-13', patient: 'LEFORT Pierre', parameter: 'SpO2', value: '88%', status: 'Faible' },
            { date: '2024-01-12', patient: 'DUBOIS Alice', parameter: 'Poids', value: '+2.5kg', status: 'Augmentation' },
          ]
        };

      case 'trends':
        return {
          ...baseData,
          sampleSize: 89,
          trends: [
            { parameter: 'Température corporelle', average: '36.8°C', evolution: '+0.1°C', trend: 'Stable' },
            { parameter: 'Rythme cardiaque', average: '73 bpm', evolution: '-2 bpm', trend: 'Amélioration' },
            { parameter: 'Poids moyen', average: '74.2 kg', evolution: '+0.3 kg', trend: 'Stable' },
            { parameter: 'Activité physique', average: '85%', evolution: '+5%', trend: 'Amélioration' },
          ],
          weeklyVariations: [
            { week: 'Semaine 1', measurements: 845, alerts: 18, activityRate: '82%' },
            { week: 'Semaine 2', measurements: 912, alerts: 23, activityRate: '85%' },
            { week: 'Semaine 3', measurements: 878, alerts: 15, activityRate: '87%' },
            { week: 'Semaine 4', measurements: 934, alerts: 21, activityRate: '86%' },
          ]
        };

      case 'alerts':
        return {
          ...baseData,
          totalAlerts: 67,
          criticalAlerts: 12,
          alertDetails: [
            { date: '2024-01-15', time: '10:23', patient: 'DUPONT Jean', type: 'Critique', parameter: 'Température', value: '39.2°C', threshold: '38.0°C', severity: 'Haute', action: 'Médecin notifié' },
            { date: '2024-01-14', time: '14:45', patient: 'MARTIN Marie', type: 'Alerte', parameter: 'Rythme cardiaque', value: '110 bpm', threshold: '100 bpm', severity: 'Moyenne', action: 'Surveillance' },
            { date: '2024-01-13', time: '09:12', patient: 'LEFORT Pierre', type: 'Alerte', parameter: 'Pression', value: '150/95', threshold: '140/90', severity: 'Moyenne', action: 'Contrôle programmé' },
          ],
          alertSummary: [
            { type: 'Température élevée', count: 28, percentage: '42%' },
            { type: 'Rythme cardiaque', count: 19, percentage: '28%' },
            { type: 'Pression artérielle', count: 12, percentage: '18%' },
            { type: 'Poids anormal', count: 8, percentage: '12%' },
          ]
        };

      default:
        return baseData;
    }
  };

  // Titre des rapports
  const getReportTitle = (reportType) => {
    const titles = {
      weekly: 'Weekly Report',
      trends: 'Trends Analysis',
      alerts: 'Alerts Report'
    };
    return titles[reportType] || 'Report';
  };

  // Télécharger un rapport récent
  const downloadRecentReport = (report) => {
    const reportData = getReportData(report.type);
    const content = generateReportCSV(reportData, report.type);
    const blob = new Blob([content], { type: 'text/csv;charset=utf-8;' });
    const url = window.URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = `${report.name}.csv`;
    link.click();
    window.URL.revokeObjectURL(url);
  };

  // Menu de sélection de format
  const handleGenerateWithFormat = (reportType) => {
    const format = window.prompt(
      `Choose format for "${getReportTitle(reportType)}":\n1 - PDF\n2 - CSV\n3 - Excel\n4 - JSON\n\nEnter number:`
    );
    
    const formats = { '1': 'pdf', '2': 'csv', '3': 'excel', '4': 'json' };
    generateReport(reportType, formats[format] || 'pdf');
  };

  return (
    <div className="space-y-6">
      <h1 className="text-3xl font-bold text-gray-800">Reports and Analytics</h1>

      {/* Modèles de rapports */}
      <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
        {reportTemplates.map((report, index) => {
          const Icon = report.icon;
          const isLoading = loading[report.type];

          return (
            <div key={index} className="card p-6 text-center hover:shadow-lg transition-shadow">
              <Icon className="mx-auto text-blue-600 mb-4" size={32} />
              <h3 className="text-lg font-semibold text-gray-800 mb-2">{report.name}</h3>
              <p className="text-gray-600 text-sm mb-4">{report.description}</p>
              <div className="flex space-x-2">
                <button 
                  onClick={() => handleGenerateWithFormat(report.type)}
                  disabled={isLoading}
                  className={`flex-1 flex items-center justify-center space-x-2 py-2 rounded-lg ${
                    isLoading 
                      ? 'bg-blue-400 cursor-not-allowed' 
                      : 'bg-blue-600 hover:bg-blue-700'
                  } text-white transition-colors`}
                >
                  <Download size={16} />
                  <span>{isLoading ? 'Generating...' : 'Generate'}</span>
                </button>
                <button 
                  onClick={() => sendReport(report.type)}
                  className="flex-1 flex items-center justify-center space-x-2 py-2 border border-gray-300 rounded-lg hover:bg-gray-50 transition-colors"
                >
                  <Send size={16} />
                  <span>Send</span>
                </button>
              </div>
            </div>
          );
        })}
      </div>

      {/* Options de format rapide */}
      <div className="bg-gray-50 border border-gray-200 rounded-lg p-4">
        <h3 className="text-sm font-semibold text-gray-800 mb-3">Quick export formats</h3>
        <div className="flex flex-wrap gap-2">
          {reportTemplates.map((report) => (
            <div key={report.type} className="flex items-center space-x-2">
              <span className="text-xs text-gray-600">{getReportTitle(report.type)}:</span>
              <div className="flex space-x-1">
                {['pdf', 'csv', 'excel', 'json'].map(format => (
                  <button
                    key={format}
                    onClick={() => generateReport(report.type, format)}
                    className="px-2 py-1 bg-white border border-gray-300 rounded text-xs text-gray-700 hover:bg-gray-100 capitalize"
                  >
                    {format}
                  </button>
                ))}
              </div>
            </div>
          ))}
        </div>
      </div>

      {/* Statistiques avancées */}
      <div className="card p-6">
        <h3 className="text-lg font-semibold text-gray-800 mb-4">Distribution by Age Group</h3>
        <DynamicBarChart />
      </div>

      {/* Rapports récents */}
      <div className="card p-6">
        <h3 className="text-lg font-semibold text-gray-800 mb-4">Recently Generated Reports</h3>
        <div className="space-y-3">
          {recentReports.map((report, index) => (
            <div key={index} className="flex items-center justify-between p-3 border border-gray-200 rounded-lg">
              <div className="flex items-center space-x-4">
                <FileText className="text-blue-600" size={20} />
                <div>
                  <span className="font-medium text-gray-800">{report.name}</span>
                  <p className="text-gray-600 text-sm">
                    Generated {report.date.toLocaleDateString('en-US')} • {report.size}
                  </p>
                </div>
              </div>
              <button 
                onClick={() => downloadRecentReport(report)}
                className="flex items-center space-x-2 text-blue-600 hover:text-blue-800 transition-colors"
              >
                <Download size={16} />
                <span>Download</span>
              </button>
            </div>
          ))}
        </div>
      </div>
    </div>
  );
};

export default Reports;
