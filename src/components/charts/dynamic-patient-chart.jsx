import React, { useState, useEffect } from 'react';
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler,
} from 'chart.js';
import { Line } from 'react-chartjs-2';

ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler
);

const DynamicPatientChart = ({ patientId, measurements = [] }) => {
  const [chartData, setChartData] = useState(null);
  const [loading, setLoading] = useState(true);

  // Load patient data
  const fetchPatientChartData = async () => {
    try {
      if (measurements.length > 0) {
        // Use measurements provided in props
        const formattedData = formatPatientData(measurements);
        setChartData(formattedData);
      } else {
        // Load from API if no measurements provided
        const response = await fetch(`/backend/get_patient_chart_data.php?id=${patientId}`);
        if (response.ok) {
          const data = await response.json();
          if (data.success) {
            // Format API data for Chart.js
            const formattedData = formatApiData(data.chartData);
            setChartData(formattedData);
          }
        }
      }
    } catch (error) {
      console.error('Error loading patient data:', error);
      setLoading(false);
    } finally {
      setLoading(false);
    }
  };

  // Format patient data
  const formatPatientData = (measurements) => {
    const labels = measurements.map(m => 
      new Date(m.date).toLocaleDateString('fr-FR', { day: 'numeric', month: 'short' })
    );
    
    const tempData = measurements.map(m => m.temp || 0);
    const heartRateData = measurements.map(m => m.heartRate || 0);
    const weightData = measurements.map(m => m.weight || 0);

    return {
      labels,
      datasets: [
        {
          label: 'Temperature (°C)',
          data: tempData,
          borderColor: 'rgb(239, 68, 68)',
          backgroundColor: 'rgba(239, 68, 68, 0.1)',
          tension: 0.4,
          fill: true,
          yAxisID: 'y',
        },
        {
          label: 'Heart Rate (bpm)',
          data: heartRateData,
          borderColor: 'rgb(59, 130, 246)',
          backgroundColor: 'rgba(59, 130, 246, 0.1)',
          tension: 0.4,
          fill: true,
          yAxisID: 'y1',
        },
        {
          label: 'Weight (kg)',
          data: weightData,
          borderColor: 'rgb(16, 185, 129)',
          backgroundColor: 'rgba(16, 185, 129, 0.1)',
          tension: 0.4,
          fill: true,
          yAxisID: 'y2',
        },
      ],
    };
  };

  // Format API data for Chart.js
  const formatApiData = (apiData) => {
    return {
      labels: apiData.labels,
      datasets: [
        {
          label: 'Temperature (°C)',
          data: apiData.temperatureData,
          borderColor: 'rgb(239, 68, 68)',
          backgroundColor: 'rgba(239, 68, 68, 0.1)',
          tension: 0.4,
          fill: true,
          yAxisID: 'y',
        },
        {
          label: 'Heart Rate (bpm)',
          data: apiData.heartRateData,
          borderColor: 'rgb(59, 130, 246)',
          backgroundColor: 'rgba(59, 130, 246, 0.1)',
          tension: 0.4,
          fill: true,
          yAxisID: 'y1',
        },
        {
          label: 'SpO2 (%)',
          data: apiData.spo2Data,
          borderColor: 'rgb(147, 51, 234)',
          backgroundColor: 'rgba(147, 51, 234, 0.1)',
          tension: 0.4,
          fill: true,
          yAxisID: 'y2',
        },
        {
          label: 'Weight (kg)',
          data: apiData.weightData,
          borderColor: 'rgb(16, 185, 129)',
          backgroundColor: 'rgba(16, 185, 129, 0.1)',
          tension: 0.4,
          fill: true,
          yAxisID: 'y3',
        },
      ],
    };
  };


  useEffect(() => {
    fetchPatientChartData();
    
    // Auto-update every 60 seconds
    const interval = setInterval(fetchPatientChartData, 60000);
    return () => clearInterval(interval);
  }, [patientId, measurements]);

  const options = {
    responsive: true,
    interaction: {
      mode: 'index',
      intersect: false,
    },
    plugins: {
      legend: {
        position: 'top',
      },
      title: {
        display: true,
        text: 'Patient Parameters Evolution',
      },
      tooltip: {
        callbacks: {
          label: function(context) {
            let label = context.dataset.label || '';
            if (label) {
              label += ': ';
            }
            if (context.parsed.y !== null) {
              if (label.includes('Temperature')) {
                label += context.parsed.y + '°C';
              } else if (label.includes('Heart Rate')) {
                label += context.parsed.y + ' bpm';
              } else if (label.includes('SpO2')) {
                label += context.parsed.y + '%';
              } else if (label.includes('Weight')) {
                label += context.parsed.y + ' kg';
              } else {
                label += context.parsed.y;
              }
            }
            return label;
          }
        }
      }
    },
    scales: {
      x: {
        display: true,
        title: {
          display: true,
          text: 'Date'
        },
        grid: {
          color: 'rgba(0, 0, 0, 0.1)',
        },
      },
      y: {
        type: 'linear',
        display: true,
        position: 'left',
        title: {
          display: true,
          text: 'Temperature (°C)'
        },
        min: 35,
        max: 40,
        grid: {
          color: 'rgba(0, 0, 0, 0.1)',
        },
      },
      y1: {
        type: 'linear',
        display: true,
        position: 'right',
        title: {
          display: true,
          text: 'Heart Rate (bpm)'
        },
        min: 50,
        max: 120,
        grid: {
          drawOnChartArea: false,
        },
      },
      y2: {
        type: 'linear',
        display: true,
        position: 'right',
        title: {
          display: true,
          text: 'SpO2 (%)'
        },
        min: 90,
        max: 100,
        grid: {
          drawOnChartArea: false,
        },
      },
      y3: {
        type: 'linear',
        display: false,
        position: 'right',
        title: {
          display: true,
          text: 'Weight (kg)'
        },
        grid: {
          drawOnChartArea: false,
        },
      },
    },
    animation: {
      duration: 1000,
      easing: 'easeInOutQuart',
    },
  };

  if (loading) {
    return (
      <div className="h-64 flex items-center justify-center">
        <div className="text-center">
          <div className="animate-spin rounded-full h-8 w-8 border-b-2 border-blue-600 mx-auto mb-2"></div>
          <div className="text-gray-500">Loading patient data...</div>
        </div>
      </div>
    );
  }

  if (!chartData) {
    return (
      <div className="h-64 flex items-center justify-center">
        <div className="text-gray-500">No measurement data available</div>
      </div>
    );
  }

  return <Line options={options} data={chartData} />;
};

export default DynamicPatientChart;
