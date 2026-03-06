import React, { useState, useEffect } from 'react';
import DynamicLineChartFlexible from '../components/charts/dynamic-linechart-flexible';
import { Eye, AlertCircle } from 'lucide-react';

const MedicalData = () => {
  const [oculairStats, setOculaireStats] = useState(null);
  const [anomalies, setAnomalies] = useState([]);
  const [loading, setLoading] = useState(false);

  // Charger les statistiques oculaires
  useEffect(() => {
    const fetchOculaireStats = async () => {
      try {
        setLoading(true);
        const response = await fetch('http://localhost:8000/backend/get_statistiques_oculaires.php');
        if (response.ok) {
          const data = await response.json();
          if (data.success) {
            setOculaireStats(data.statistiques);
          }
        }
      } catch (err) {
        console.log('Erreur chargement statistiques oculaires:', err);
      }
    };

    const fetchAnomalies = async () => {
      try {
        const response = await fetch('http://localhost:8000/backend/get_anomalies.php?limit=5');
        if (response.ok) {
          const data = await response.json();
          if (data.success) {
            setAnomalies(data.data || []);
          }
        }
      } catch (err) {
        console.log('Erreur chargement anomalies:', err);
      } finally {
        setLoading(false);
      }
    };

    fetchOculaireStats();
    fetchAnomalies();
  }, []);

  const medicalStats = [
    { parameter: 'Average Temperature', value: '36.8°C', trend: 'stable', color: 'text-blue-600' },
    { parameter: 'Average Heart Rate', value: '72 bpm', trend: 'stable', color: 'text-green-600' },
    { parameter: 'Average SpO2', value: '97%', trend: 'stable', color: 'text-cyan-600' },
    { parameter: 'Average Weight', value: '73.2 kg', trend: 'decrease', color: 'text-purple-600' },
    { parameter: 'Measurements Today', value: '156', trend: 'increase', color: 'text-orange-600' },
  ];

  return (
    <div className="space-y-6">
      <h1 className="text-3xl font-bold text-gray-800">Medical Data</h1>

      {/* Statistiques médicales */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-6">
        {medicalStats.map((stat, index) => (
          <div key={index} className="card p-6">
            <h3 className="text-lg font-semibold text-gray-800">{stat.parameter}</h3>
            <div className={`text-2xl font-bold mt-2 ${stat.color}`}>{stat.value}</div>
            <div className={`text-sm mt-1 ${
              stat.trend === 'increase' ? 'text-green-600' : 
              stat.trend === 'decrease' ? 'text-red-600' : 'text-gray-600'
            }`}>
              {stat.trend === 'increase' ? '↗' : stat.trend === 'decrease' ? '↘' : '→'} {stat.trend}
            </div>
          </div>
        ))}
      </div>

      {/* Detailed charts */}
      <div className="card p-6">
        <h3 className="text-lg font-semibold text-gray-800 mb-4">Parameters Evolution over 30 days</h3>
        <DynamicLineChartFlexible days={30} />
      </div>

      {/* Statistiques oculaires */}
      {oculairStats && (
        <div className="bg-gradient-to-r from-purple-50 to-pink-50 border-l-4 border-purple-600 rounded-lg p-6">
          <div className="flex items-center space-x-3 mb-4">
            <Eye className="text-purple-600" size={28} />
            <h3 className="text-xl font-bold text-gray-800">Ocular Analysis</h3>
          </div>
          <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
            <div className="bg-white p-4 rounded-lg shadow-sm">
              <div className="text-sm text-gray-600 mb-1">Detected Anomalies</div>
              <div className="text-2xl font-bold text-red-600">{oculairStats.total_anomalies}</div>
              <div className="text-xs text-gray-500 mt-1">Total</div>
            </div>
            <div className="bg-white p-4 rounded-lg shadow-sm">
              <div className="text-sm text-gray-600 mb-1">Today</div>
              <div className="text-2xl font-bold text-orange-600">{oculairStats.anomalies_today}</div>
              <div className="text-xs text-gray-500 mt-1">Last 24 hours</div>
            </div>
            <div className="bg-white p-4 rounded-lg shadow-sm">
              <div className="text-sm text-gray-600 mb-1">Average Confidence</div>
              <div className="text-2xl font-bold text-blue-600">
                {((oculairStats.avg_confidence.left_eye + oculairStats.avg_confidence.right_eye) / 2 * 100).toFixed(0)}%
              </div>
              <div className="text-xs text-gray-500 mt-1">AI</div>
            </div>
            <div className="bg-white p-4 rounded-lg shadow-sm">
              <div className="text-sm text-gray-600 mb-1">Affected Patients</div>
              <div className="text-2xl font-bold text-purple-600">{oculairStats.patients_with_anomalies}</div>
              <div className="text-xs text-gray-500 mt-1">Unique</div>
            </div>
          </div>
        </div>
      )}

      {/* Anomalies oculaires récentes */}
      {anomalies.length > 0 && (
        <div className="card p-6">
          <div className="flex items-center space-x-2 mb-4">
            <AlertCircle className="text-red-600" size={24} />
            <h3 className="text-lg font-semibold text-gray-800">Detected Ocular Anomalies</h3>
          </div>
          <div className="space-y-3">
            {anomalies.map((anomaly, index) => (
              <div key={index} className="flex items-center justify-between p-4 bg-red-50 border border-red-200 rounded-lg">
                <div>
                  <div className="font-semibold text-gray-800">
                    {anomaly.prenom} {anomaly.nom} <span className="text-sm text-gray-600 font-normal">({anomaly.rfid})</span>
                  </div>
                  <div className="text-sm text-gray-700 mt-1">
                    <Eye className="inline mr-1" size={14} />
                    Left: <span className="font-semibold">{anomaly.oeil_gauche}</span> ({(anomaly.oeil_gauche_confiance * 100).toFixed(0)}%)
                    {' | '}
                    Right: <span className="font-semibold">{anomaly.oeil_droit}</span> ({(anomaly.oeil_droit_confiance * 100).toFixed(0)}%)
                  </div>
                  <div className="text-xs text-gray-600 mt-1">
                    {new Date(anomaly.date_mesure).toLocaleDateString('fr-FR')} à {new Date(anomaly.date_mesure).toLocaleTimeString('fr-FR', { hour: '2-digit', minute: '2-digit' })}
                  </div>
                </div>
                <span className="px-3 py-1 bg-red-200 text-red-800 rounded-full text-xs font-semibold">Alert</span>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Recent alerts */}
      <div className="card p-6">
        <h3 className="text-lg font-semibold text-gray-800 mb-4">Recent Alerts</h3>
        <div className="space-y-3">
          <div className="flex items-center justify-between p-3 bg-red-50 border border-red-200 rounded-lg">
            <div>
              <span className="font-medium text-red-800">RFID007 - High Temperature</span>
              <p className="text-red-600 text-sm">Temperature: 38.2°C detected at 09:45</p>
            </div>
            <span className="text-red-800 font-medium">Critical</span>
          </div>
          <div className="flex items-center justify-between p-3 bg-orange-50 border border-orange-200 rounded-lg">
            <div>
              <span className="font-medium text-orange-800">RFID012 - High Heart Rate</span>
              <p className="text-orange-600 text-sm">Heart Rate: 95 bpm detected at 10:20</p>
            </div>
            <span className="text-orange-800 font-medium">Moderate</span>
          </div>
        </div>
      </div>
    </div>
  );
};

export default MedicalData;
