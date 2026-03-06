import React, { useState, useEffect, useReducer, useMemo } from 'react';
import { Users, Activity, Heart, Thermometer, AlertTriangle, RefreshCw, Wind, Eye, AlertCircle } from 'lucide-react';
import DynamicLineChart from '../components/charts/dynamic-linechart';
import DynamicBarChart from '../components/charts/dynamic-barchart';

// Reducer pour une meilleure gestion d'état
const dashboardReducer = (state, action) => {
  switch (action.type) {
    case 'SET_STATS':
      return { ...state, stats: action.payload };
    case 'SET_MEASURES':
      return { ...state, recentMeasures: action.payload };
    case 'SET_LOADING':
      return { ...state, loading: action.payload };
    case 'SET_REFRESHING':
      return { ...state, isRefreshing: action.payload };
    case 'SET_ERROR':
      return { ...state, error: action.payload };
    case 'SET_LAST_UPDATE':
      return { ...state, lastUpdate: action.payload };
    case 'RESET':
      return initialState;
    default:
      return state;
  }
};

const initialState = {
  stats: {
    totalPatients: 0,
    todayMeasures: 0,
    heartAlerts: 0,
    tempAlerts: 0,
    spo2Alerts: 0
  },
  recentMeasures: [],
  loading: true,
  lastUpdate: '',
  error: '',
  isRefreshing: false
};

const Dashboard = () => {
  const [state, dispatch] = useReducer(dashboardReducer, initialState);
  const { stats, recentMeasures, loading, lastUpdate, error, isRefreshing } = state;

  // Fonction pour charger les données
  const fetchDashboardData = async () => {
    try {
      dispatch({ type: 'SET_REFRESHING', payload: true });
      
      // URLs des APIs - optimisé avec une seule URL
      const statsUrl = '/api/get_dashboard_stats.php';
      const measuresUrl = '/api/get_recent_mesures.php';

      let statsSuccess = false;
      let measuresSuccess = false;

      // Charger les statistiques
      try {
        console.log(` Chargement stats via: ${statsUrl}`);
        const response = await fetch(statsUrl);
        
        if (response.ok) {
          const data = await response.json();
          
          if (data.success) {
            dispatch({ type: 'SET_STATS', payload: data.stats });
            dispatch({ type: 'SET_LAST_UPDATE', payload: new Date().toLocaleString('fr-FR') });
            statsSuccess = true;
            console.log(' Statistiques chargées');
          }
        }
      } catch (err) {
        console.log(` Erreur stats:`, err.message);
      }

      // Charger les mesures récentes du jour
      try {
        console.log(` Chargement mesures via: ${measuresUrl}`);
        const response = await fetch(measuresUrl);
        
        if (response.ok) {
          const data = await response.json();
          
          if (data.success && data.measures) {
            // Formater les données pour l'affichage
            const formattedMeasures = data.measures.map(measure => ({
              id: measure.measureId,
              userId: measure.userId,
              rfid: measure.rfid,
              firstName: measure.firstName,
              lastName: measure.lastName,
              name: `${measure.firstName} ${measure.lastName}`,
              temp: measure.temp,
              heartRate: measure.heartRate,
              spo2: measure.spo2,
              weight: measure.weight,
              oeil_gauche: measure.oeil_gauche,
              oeil_droit: measure.oeil_droit,
              oeil_gauche_confiance: measure.oeil_gauche_confiance,
              oeil_droit_confiance: measure.oeil_droit_confiance,
              alerte_oculaire: measure.alerte_oculaire,
              time: new Date(measure.date).toLocaleTimeString('fr-FR', { 
                hour: '2-digit', 
                minute: '2-digit' 
              }),
              date: measure.date,
              status: measure.status
            }));
            dispatch({ type: 'SET_MEASURES', payload: formattedMeasures });
            measuresSuccess = true;
            console.log(` ${data.count} mesures du jour chargées`);
          }
        }
      } catch (err) {
        console.log(` Erreur mesures:`, err.message);
      }

      if (!statsSuccess && !measuresSuccess) {
        dispatch({ type: 'SET_ERROR', payload: 'Impossible de charger les données. Vérifiez la connexion au serveur.' });
      } else {
        dispatch({ type: 'SET_ERROR', payload: '' });
      }

    } catch (err) {
      dispatch({ type: 'SET_ERROR', payload: 'Erreur inattendue: ' + err.message });
      console.error(' Erreur dashboard:', err);
    } finally {
      dispatch({ type: 'SET_LOADING', payload: false });
      dispatch({ type: 'SET_REFRESHING', payload: false });
    }
  };

  // Polling automatique - chargement initial et toutes les 30 secondes
  useEffect(() => {
    // Charger immédiatement
    fetchDashboardData();

    // Mettre en place le polling toutes les 30 secondes (optimisé)
    const interval = setInterval(fetchDashboardData, 30000);

    // Nettoyer l'intervalle quand le composant est démonté
    return () => clearInterval(interval);
  }, []);

  // Fonction de rafraîchissement manuel
  const handleManualRefresh = () => {
    fetchDashboardData();
  };

  // Fonctions pour les couleurs
  const getTempColor = (temp) => {
    if (temp > 40) return 'bg-red-100 text-red-800';
    if (temp >= 38 && temp <= 40) return 'bg-orange-100 text-orange-800';
    if (temp < 37) return 'bg-yellow-100 text-yellow-800';
    return 'bg-green-100 text-green-800';
  };

  const getHeartRateColor = (heartRate) => {
    if (heartRate > 100) return 'bg-red-100 text-red-800';
    if (heartRate < 50) return 'bg-yellow-100 text-yellow-800';
    return 'bg-green-100 text-green-800';
  };

  const getSpo2Color = (spo2) => {
    if (spo2 < 90) return 'bg-red-100 text-red-800';
    if (spo2 >= 90 && spo2 < 95) return 'bg-yellow-100 text-yellow-800';
    return 'bg-green-100 text-green-800';
  };

  const getStatusConfig = (status) => {
    switch (status) {
      case 'critique':
        return { class: 'bg-red-100 text-red-800', text: '⚠️ Critical' };
      case 'alerte':
        return { class: 'bg-yellow-100 text-yellow-800', text: '⚠️ Alert' };
      case 'normal':
        return { class: 'bg-green-100 text-green-800', text: '✓ Normal' };
      default:
        return { class: 'bg-gray-100 text-gray-800', text: 'Not measured' };
    }
  };


  // Squelettes de chargement
  const StatCardSkeleton = () => (
    <div className="card p-3 bg-gray-50 border-0 animate-pulse">
      <div className="flex items-center justify-between">
        <div>
          <div className="h-6 bg-gray-300 rounded w-16 mb-2"></div>
          <div className="h-4 bg-gray-200 rounded w-20"></div>
        </div>
        <div className="h-5 w-5 bg-gray-300 rounded"></div>
      </div>
    </div>
  );

  const TableSkeleton = () => (
    <div className="space-y-3">
      {[...Array(5)].map((_, index) => (
        <div key={index} className="flex space-x-4 animate-pulse">
          <div className="h-4 bg-gray-200 rounded flex-1"></div>
          <div className="h-4 bg-gray-200 rounded flex-1"></div>
          <div className="h-4 bg-gray-200 rounded flex-1"></div>
          <div className="h-4 bg-gray-200 rounded flex-1"></div>
        </div>
      ))}
    </div>
  );

  // Optimiser les calculs avec useMemo
  const statsConfig = useMemo(() => [
    { 
      icon: Users, 
      number: stats.totalPatients.toString(), 
      label: 'Registered patients', 
      color: 'text-blue-600',
      bgColor: 'bg-blue-50'
    },
    { 
      icon: Activity, 
      number: stats.todayMeasures.toString(), 
      label: 'Measurements today', 
      color: 'text-green-600',
      bgColor: 'bg-green-50'
    },
    { 
      icon: Heart, 
      number: stats.heartAlerts.toString(), 
      label: 'Cardiac alerts', 
      color: 'text-red-600',
      bgColor: 'bg-red-50'
    },
    { 
      icon: Thermometer, 
      number: stats.tempAlerts.toString(), 
      label: 'Critical temperatures', 
      color: 'text-orange-600',
      bgColor: 'bg-orange-50'
    },
    { 
      icon: Wind,
      number: stats.spo2Alerts.toString(), 
      label: 'SpO2 alerts', 
      color: 'text-purple-600',
      bgColor: 'bg-purple-50'
    },
  ], [stats]);

  if (loading) {
    return (
      <div className="space-y-6">
        <div className="flex justify-between items-center">
          <div className="h-8 bg-gray-300 rounded w-48 animate-pulse"></div>
          <div className="flex items-center space-x-4">
            <div className="h-4 bg-gray-200 rounded w-32 animate-pulse"></div>
            <div className="h-9 bg-gray-200 rounded-lg w-24 animate-pulse"></div>
          </div>
        </div>

        {/* Squelettes des statistiques */}
        <div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-5 gap-3">
          {[...Array(5)].map((_, index) => (
            <StatCardSkeleton key={index} />
          ))}
        </div>

        {/* Squelettes des graphiques */}
        <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
          <div className="card p-6">
            <div className="h-6 bg-gray-300 rounded w-48 mb-4 animate-pulse"></div>
            <div className="h-64 bg-gray-100 rounded animate-pulse"></div>
          </div>
          <div className="card p-6">
            <div className="h-6 bg-gray-300 rounded w-40 mb-4 animate-pulse"></div>
            <div className="h-64 bg-gray-100 rounded animate-pulse"></div>
          </div>
        </div>

        {/* Squelette du tableau */}
        <div className="card p-6">
          <div className="h-6 bg-gray-300 rounded w-48 mb-4 animate-pulse"></div>
          <TableSkeleton />
        </div>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      <div className="flex justify-between items-center">
        <h1 className="text-3xl font-bold text-gray-800">Dashboard</h1>
        <div className="flex items-center space-x-4">
          <div className="text-sm text-gray-500">
            Last update: {lastUpdate || 'Loading...'}
          </div>
          <button
            onClick={handleManualRefresh}
            disabled={isRefreshing}
            className={`flex items-center space-x-2 px-3 py-2 rounded-lg border transition-colors ${
              isRefreshing 
                ? 'bg-gray-100 text-gray-400 cursor-not-allowed' 
                : 'bg-white text-blue-600 hover:bg-blue-50 border-blue-200'
            }`}
          >
            <RefreshCw size={16} className={isRefreshing ? 'animate-spin' : ''} />
            <span>Refresh</span>
          </button>
        </div>
      </div>

      {error && (
        <div className="bg-red-50 border border-red-200 rounded-lg p-4 animate-fade-in">
          <div className="flex items-center">
            <AlertTriangle className="text-red-600 mr-2" size={20} />
            <div className="flex-1">
              <span className="text-red-700 font-medium">{error}</span>
              <div className="text-red-600 text-sm mt-1">
                Verify that the backend server is running on port 8000
              </div>
            </div>
            <button
              onClick={handleManualRefresh}
              className="ml-4 text-red-600 hover:text-red-800 text-sm font-medium"
            >
              Retry
            </button>
          </div>
        </div>
      )}

      
      {/* Statistiques - 5 cartes sur une ligne */}
{/* Statistiques - responsive */}
<div className="grid grid-cols-2 md:grid-cols-3 lg:grid-cols-5 gap-3">
  {statsConfig.map((stat, index) => {
    const Icon = stat.icon;
    return (
      <div key={index} className={`card p-3 ${stat.bgColor} border-0`}>
        <div className="flex items-center justify-between">
          <div>
            <div className="text-lg font-bold text-gray-800">{stat.number}</div>
            <div className="text-xs text-gray-600">{stat.label}</div>
          </div>
          <Icon className={stat.color} size={20} />
        </div>
      </div>
    );
  })}
</div>

      {/* Graphiques */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        <div className="card p-6">
          <h3 className="text-lg font-semibold text-gray-800 mb-4">Parameters Evolution (7 days)</h3>
          <DynamicLineChart />
        </div>
        <div className="card p-6">
          <h3 className="text-lg font-semibold text-gray-800 mb-4">Patient Distribution</h3>
          <DynamicBarChart />
        </div>
      </div>

      {/* Dernières mesures du jour */}
      <div className="card p-6">
        <h3 className="text-lg font-semibold text-gray-800 mb-4">
          Today's Latest Measurements ({recentMeasures.length})
          {isRefreshing && (
            <span className="ml-2 text-sm text-blue-500">
              <RefreshCw size={14} className="inline animate-spin mr-1" />
              updating...
            </span>
          )}
        </h3>
        
        {recentMeasures.length > 0 ? (
          <div className="overflow-x-auto">
            <table className="min-w-full table">
              <thead>
                <tr className="border-b border-gray-200">
                  <th className="text-left py-3 px-4 font-semibold text-gray-700">RFID ID</th>
                  <th className="text-left py-3 px-4 font-semibold text-gray-700">Patient</th>
                  <th className="text-left py-3 px-4 font-semibold text-gray-700">Temperature</th>
                  <th className="text-left py-3 px-4 font-semibold text-gray-700">Heart Rate</th>
                  <th className="text-left py-3 px-4 font-semibold text-gray-700">SpO2</th>
                  <th className="text-left py-3 px-4 font-semibold text-gray-700">Weight</th>
                  <th className="text-left py-3 px-4 font-semibold text-gray-700">Eyes</th>
                  <th className="text-left py-3 px-4 font-semibold text-gray-700">Time</th>
                  <th className="text-left py-3 px-4 font-semibold text-gray-700">Status</th>
                </tr>
              </thead>
              <tbody>
                {recentMeasures.map((measure) => {
                  const statusConfig = getStatusConfig(measure.status);
                  return (
                    <tr key={measure.id} className="border-b border-gray-100 hover:bg-gray-50">
                      <td className="py-3 px-4 font-mono text-blue-600">
                        {measure.rfid || 'N/A'}
                      </td>
                      <td className="py-3 px-4">
                        <div className="font-medium text-gray-900">{measure.name}</div>
                      </td>
                      <td className="py-3 px-4">
                        <span className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium ${getTempColor(measure.temp)}`}>
                          {measure.temp ? `${measure.temp}°C` : 'N/A'}
                        </span>
                      </td>
                      <td className="py-3 px-4">
                        <span className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium ${getHeartRateColor(measure.heartRate)}`}>
                          {measure.heartRate ? `${measure.heartRate} bpm` : 'N/A'}
                        </span>
                      </td>
                      <td className="py-3 px-4">
                        <span className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium ${getSpo2Color(measure.spo2)}`}>
                          {measure.spo2 ? `${measure.spo2}%` : 'N/A'}
                        </span>
                      </td>
                      <td className="py-3 px-4 text-gray-700">
                        {measure.weight ? `${measure.weight} kg` : 'N/A'}
                      </td>
                      <td className="py-3 px-4">
                        {measure.oeil_gauche || measure.oeil_droit ? (
                          <div className={`flex items-center space-x-2 px-2.5 py-0.5 rounded-full text-xs font-medium ${
                            measure.alerte_oculaire ? 'bg-red-100 text-red-700' : 'bg-green-100 text-green-700'
                          }`}>
                            <Eye size={12} />
                            <div>
                              <span title="Œil Gauche">{measure.oeil_gauche}</span>
                              <span className="text-opacity-75">/</span>
                              <span title="Œil Droit">{measure.oeil_droit}</span>
                            </div>
                            {measure.alerte_oculaire && <AlertCircle size={12} />}
                          </div>
                        ) : (
                          <span className="text-gray-400">N/A</span>
                        )}
                      </td>
                      <td className="py-3 px-4 text-gray-500 text-sm">
                        {measure.time}
                      </td>
                      <td className="py-3 px-4">
                        <span className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium ${statusConfig.class}`}>
                          {statusConfig.text}
                        </span>
                      </td>
                    </tr>
                  );
                })}
              </tbody>
            </table>
          </div>
        ) : (
          <div className="text-center py-8 text-gray-500">
            <Activity className="mx-auto mb-2 text-gray-400" size={32} />
            <div>No measurements recorded today</div>
            <div className="text-sm mt-1">New measurements will appear automatically</div>
            <button 
              onClick={handleManualRefresh}
              className="mt-4 text-blue-600 hover:text-blue-800 text-sm"
            >
              Refresh
            </button>
          </div>
        )}
      </div>
    </div>
  );
};

export default Dashboard;
