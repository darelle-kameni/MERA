import React, { useState, useEffect } from 'react';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  BarElement,
  Title,
  Tooltip,
  Legend,
} from 'chart.js';
import { Bar } from 'react-chartjs-2';

ChartJS.register(
  CategoryScale,
  LinearScale,
  BarElement,
  Title,
  Tooltip,
  Legend
);

const DynamicBarChart = () => {
  const [chartData, setChartData] = useState(null);
  const [loading, setLoading] = useState(true);
  const [chartType, setChartType] = useState('distribution');

  // Function to load chart data
  const fetchChartData = async () => {
    try {
      const response = await fetch(`/backend/get_chart_data.php?type=${chartType}`);
      if (response.ok) {
        const data = await response.json();
        
        if (data.success) {
          if (chartType === 'distribution') {
            // Graphique de distribution par groupe d'âge + mesures du jour
            const chartData = {
              labels: data.labels || [],
              datasets: [
                {
                  label: 'Patients by Age Group',
                  data: data.ageGroupData || [],
                  backgroundColor: [
                    'rgba(59, 130, 246, 0.6)',
                    'rgba(34, 197, 94, 0.6)',
                    'rgba(249, 115, 22, 0.6)',
                    'rgba(239, 68, 68, 0.6)',
                    'rgba(139, 92, 246, 0.6)',
                  ],
                  borderColor: [
                    'rgb(59, 130, 246)',
                    'rgb(34, 197, 94)',
                    'rgb(249, 115, 22)',
                    'rgb(239, 68, 68)',
                    'rgb(139, 92, 246)',
                  ],
                  borderWidth: 1.5,
                  borderRadius: 6,
                },
              ],
            };
            setChartData(chartData);
          } else if (chartType === 'alerts') {
            // Graphique des alertes des 7 derniers jours
            const chartData = {
              labels: data.labels || [],
              datasets: [
                {
                  label: 'Critical Alerts',
                  data: data.criticalData || [],
                  backgroundColor: 'rgba(239, 68, 68, 0.6)',
                  borderColor: 'rgb(239, 68, 68)',
                  borderWidth: 1.5,
                  borderRadius: 6,
                },
                {
                  label: 'Standard Alerts',
                  data: data.alertData || [],
                  backgroundColor: 'rgba(249, 115, 22, 0.6)',
                  borderColor: 'rgb(249, 115, 22)',
                  borderWidth: 1.5,
                  borderRadius: 6,
                },
                {
                  label: 'Eye Alerts',
                  data: data.eyeAlertData || [],
                  backgroundColor: 'rgba(59, 130, 246, 0.6)',
                  borderColor: 'rgb(59, 130, 246)',
                  borderWidth: 1.5,
                  borderRadius: 6,
                },
              ],
            };
            setChartData(chartData);
          } else if (chartType === 'comparison') {
            // Graphique de comparaison aujourd'hui vs hier
            const chartData = {
              labels: data.labels || [],
              datasets: [
                {
                  label: 'Today',
                  data: data.todayData || [],
                  backgroundColor: 'rgba(59, 130, 246, 0.6)',
                  borderColor: 'rgb(59, 130, 246)',
                  borderWidth: 1.5,
                  borderRadius: 6,
                },
                {
                  label: 'Yesterday',
                  data: data.yesterdayData || [],
                  backgroundColor: 'rgba(209, 213, 219, 0.6)',
                  borderColor: 'rgb(107, 114, 128)',
                  borderWidth: 1.5,
                  borderRadius: 6,
                },
              ],
            };
            setChartData(chartData);
          }
        }
      }
    } catch (error) {
      console.error('Error loading chart data:', error);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    setLoading(true);
    fetchChartData();
  }, [chartType]);

  useEffect(() => {
    // Auto-update every 60 seconds
    const interval = setInterval(fetchChartData, 60000);
    return () => clearInterval(interval);
  }, [chartType]);

  const options = {
    responsive: true,
    maintainAspectRatio: true,
    indexAxis: chartType === 'distribution' ? 'y' : 'x',
    plugins: {
      legend: {
        position: 'top',
        labels: {
          boxWidth: 12,
          padding: 15,
          font: {
            size: 12,
          },
        },
      },
      title: {
        display: false,
      },
      tooltip: {
        backgroundColor: 'rgba(0, 0, 0, 0.8)',
        padding: 12,
        titleFont: { size: 13 },
        bodyFont: { size: 12 },
        callbacks: {
          label: function(context) {
            let label = context.dataset.label || '';
            if (label) {
              label += ': ';
            }
            if (context.parsed.x !== undefined) {
              label += Math.round(context.parsed.x);
            } else if (context.parsed.y !== undefined) {
              label += Math.round(context.parsed.y);
            }
            return label;
          }
        }
      }
    },
    scales: {
      y: {
        beginAtZero: true,
        grid: {
          color: 'rgba(0, 0, 0, 0.05)',
          drawBorder: true,
        },
        ticks: {
          font: {
            size: 11,
          },
        },
      },
      x: {
        beginAtZero: true,
        grid: {
          color: 'rgba(0, 0, 0, 0.05)',
          drawBorder: true,
        },
        ticks: {
          font: {
            size: 11,
          },
        },
      },
    },
    animation: {
      duration: 750,
      easing: 'easeInOutQuart',
    },
  };

  if (loading) {
    return (
      <div className="h-64 flex items-center justify-center">
        <div className="text-center">
          <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-blue-600 mx-auto mb-2"></div>
          <div className="text-gray-500">Loading chart data...</div>
        </div>
      </div>
    );
  }

  return (
    <div className="w-full">
      <div className="mb-4 flex gap-2 flex-wrap">
        <button
          onClick={() => setChartType('distribution')}
          className={`px-4 py-2 rounded-lg font-medium transition-colors duration-200 ${
            chartType === 'distribution'
              ? 'bg-blue-100 text-blue-800 border border-blue-300'
              : 'bg-gray-100 text-gray-700 border border-gray-300 hover:bg-gray-200'
          }`}
        >
          Distribution
        </button>
        <button
          onClick={() => setChartType('alerts')}
          className={`px-4 py-2 rounded-lg font-medium transition-colors duration-200 ${
            chartType === 'alerts'
              ? 'bg-red-100 text-red-800 border border-red-300'
              : 'bg-gray-100 text-gray-700 border border-gray-300 hover:bg-gray-200'
          }`}
        >
          Alerts
        </button>
        <button
          onClick={() => setChartType('comparison')}
          className={`px-4 py-2 rounded-lg font-medium transition-colors duration-200 ${
            chartType === 'comparison'
              ? 'bg-green-100 text-green-800 border border-green-300'
              : 'bg-gray-100 text-gray-700 border border-gray-300 hover:bg-gray-200'
          }`}
        >
          Comparison
        </button>
      </div>
      {chartData ? <Bar options={options} data={chartData} /> : null}
    </div>
  );
};

export default DynamicBarChart;
