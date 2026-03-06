import React, { useState, useEffect } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { Search, Plus, Filter, Edit2, Trash2 } from 'lucide-react';

const Patients = () => {
  const navigate = useNavigate();
  const [searchTerm, setSearchTerm] = useState('');
  const [patients, setPatients] = useState([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);
  const [deleteConfirm, setDeleteConfirm] = useState(null);
  const [deleting, setDeleting] = useState(false);

  // Fonction pour déterminer la couleur des valeurs
  const getValueColor = (patient, type) => {
    const temp = patient.temp;
    const heartRate = patient.heartRate;
    const spo2 = patient.spo2;
    
    // If no data
    if (temp === null || heartRate === null || spo2 === null) return 'text-gray-700';
    
    if (type === 'temp') {
      if (temp > 40) return 'text-red-600 font-bold';
      if (temp >= 38 && temp <= 40) return 'text-orange-600 font-semibold';
      if (temp < 37) return 'text-yellow-600 font-semibold';
    }
    
    if (type === 'heartRate') {
      if (heartRate > 100) return 'text-red-600 font-bold';
      if (heartRate < 50) return 'text-yellow-600 font-semibold';
    }
    
    if (type === 'spo2') {
      if (spo2 < 90) return 'text-red-600 font-bold';
      if (spo2 >= 90 && spo2 < 95) return 'text-yellow-600 font-semibold';
    }
    
    return 'text-gray-700';
  };

  // Fonction pour déterminer la classe du statut
  const getStatusClass = (status) => {
    if (status === 'critique') return 'bg-red-100 text-red-800 border border-red-300';
    if (status === 'alerte') return 'bg-yellow-100 text-yellow-800 border border-yellow-300';
    if (status === 'normal') return 'bg-green-100 text-green-800 border border-green-300';
    if (status === 'non_mesure') return 'bg-gray-100 text-gray-800 border border-gray-300';
    return 'bg-gray-100 text-gray-800 border border-gray-300';
  };

  // Fonction pour afficher le texte du statut
  const getStatusText = (status) => {
    if (status === 'critique') return '⚠️ Critique';
    if (status === 'alerte') return '⚠️ Alerte';
    if (status === 'normal') return '✓ Normal';
    if (status === 'non_mesure') return 'Non mesuré';
    return 'Non mesuré';
  };

  // Récupérer les patients depuis l'API
  useEffect(() => {
    const fetchPatients = async () => {
      try {
        setLoading(true);
        
        const apiUrls ='/api/get_patient.php';
        
        console.log('🔄 Tentative de connexion à:', apiUrls);
        
        const response = await fetch(apiUrls, {
          method: 'GET',
          headers: {
            'Accept': 'application/json',
          },
          credentials: 'include',
        });
        
        console.log('📡 Statut réponse:', response.status, response.ok);
        
        if (response.ok) {
          const data = await response.json();
          console.log('📊 Data received:', data);
          
          if (data.success) {
            setPatients(data.patients);
            setError(null);
            console.log('✅ Data loaded successfully!');
            return;
          } else {
            throw new Error('API Error: ' + (data.message || 'Unknown'));
          }
        } else {
          throw new Error(`HTTP ${response.status}: ${response.statusText}`);
        }
        
      } catch (err) {
        setError('Unable to load patients: ' + err.message);
        console.error('❌ Detailed error:', err);
          
        // Fallback data with SpO2
        setPatients([
          { 
            id: 1, 
            name: 'Jean Dupont (Demo)', 
            age: 43, 
            rfid: 'RFID001', 
            temp: 41.2,  // CRITIQUE - ROUGE
            heartRate: 85, 
            spo2: 92,    // ALERTE - JAUNE
            weight: 75.2, 
            lastCheck: '2024-01-15', 
            status: 'critique' 
          },
          { 
            id: 2, 
            name: 'Marie Martin (Demo)', 
            age: 31, 
            rfid: 'RFID002', 
            temp: 38.5,  // ALERTE - ORANGE
            heartRate: 72, 
            spo2: 97,    // NORMAL
            weight: 62.8, 
            lastCheck: '2024-01-15', 
            status: 'alerte' 
          },
          { 
            id: 3, 
            name: 'Pierre Lambert (Demo)', 
            age: 56, 
            rfid: 'RFID003', 
            temp: 36.5,  // ALERTE - JAUNE
            heartRate: 65, 
            spo2: 88,    // CRITIQUE - ROUGE
            weight: 81.5, 
            lastCheck: '2024-01-14', 
            status: 'critique' 
          },
        ]);
      } finally {
        setLoading(false);
      }
    };

    fetchPatients();
  }, []);

  // Filtrage des patients
  const filteredPatients = patients.filter(patient =>
    patient.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
    (patient.rfid && patient.rfid.toLowerCase().includes(searchTerm.toLowerCase()))
  );

  // Function to delete a patient
  const handleDeletePatient = async (patientId) => {
    try {
      setDeleting(true);
      
      const response = await fetch('/api/delete_patient.php', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
          'Accept': 'application/json',
        },
        credentials: 'include',
        body: JSON.stringify({ id_utilisateur: patientId })
      });

      const data = await response.json();

      if (data.success) {
        // Remove patient from list
        setPatients(patients.filter(p => p.id !== patientId));
        setDeleteConfirm(null);
        alert('✅ Patient deleted successfully!');
      } else {
        alert('❌ Error: ' + (data.message || 'Unable to delete patient'));
      }
    } catch (err) {
      console.error('Error deleting:', err);
      alert('❌ Error deleting: ' + err.message);
    } finally {
      setDeleting(false);
    }
  };

  // Fonction pour aller à la page modification
  const handleEditPatient = (patientId) => {
    navigate(`/patients/${patientId}/edit`);
  };

  if (loading) {
    return (
      <div className="flex justify-center items-center h-64">
        <div className="text-lg text-gray-600">Chargement des patients...</div>
      </div>
    );
  }

  if (error) {
    return (
      <div className="flex justify-center items-center h-64">
        <div className="text-red-500 text-lg text-center">
          <div>❌ Error</div>
          <div className="text-sm mt-2">{error}</div>
          <button 
            onClick={() => window.location.reload()} 
            className="mt-4 px-4 py-2 bg-blue-500 text-white rounded hover:bg-blue-600"
          >
            Réessayer
          </button>
        </div>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      {/* Modal confirmation suppression */}
      {deleteConfirm && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
          <div className="bg-white rounded-lg shadow-lg p-6 max-w-sm mx-4">
            <h3 className="text-lg font-bold text-gray-900 mb-4">
              Confirm deletion
            </h3>
            <p className="text-gray-600 mb-6">
              Are you sure you want to delete this patient? This action is irreversible and will delete all his measurements.
            </p>
            <div className="flex gap-3 justify-end">
              <button
                onClick={() => setDeleteConfirm(null)}
                disabled={deleting}
                className="px-4 py-2 border border-gray-300 rounded-lg hover:bg-gray-50 disabled:opacity-50"
              >
                Cancel
              </button>
              <button
                onClick={() => handleDeletePatient(deleteConfirm)}
                disabled={deleting}
                className="px-4 py-2 bg-red-600 text-white rounded-lg hover:bg-red-700 disabled:opacity-50"
              >
                {deleting ? 'Deleting...' : 'Delete'}
              </button>
            </div>
          </div>
        </div>
      )}

      <div className="flex justify-between items-center">
        <h1 className="text-3xl font-bold text-gray-800">Patient Management</h1>
        <Link to="/patients/add" className="btn-primary flex items-center space-x-2">
          <Plus size={20} />
          <span>Add Patient</span>
        </Link>
      </div>

      {/* Barre de recherche et filtres */}
      <div className="card p-6">
        <div className="flex flex-col md:flex-row gap-4">
          <div className="flex-1 relative">
            <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 text-gray-400" size={20} />
            <input
              type="text"
              placeholder="Search by patient name or RFID ID..."
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              className="w-full pl-10 pr-4 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
            />
          </div>
          <button className="flex items-center space-x-2 px-4 py-2 border border-gray-300 rounded-lg hover:bg-gray-50">
            <Filter size={20} />
            <span>Filters</span>
          </button>
        </div>
      </div>

      {/* Liste des patients */}
      <div className="card p-6">
        <div className="mb-4 text-sm text-gray-600">
          {filteredPatients.length} patient(s) found
        </div>
        
        <table className="w-full">
          <thead>
            <tr className="border-b border-gray-200">
              <th className="text-left py-3 px-4 font-semibold text-gray-700">RFID ID</th>
              <th className="text-left py-3 px-4 font-semibold text-gray-700">Name</th>
              <th className="text-left py-3 px-4 font-semibold text-gray-700">Age</th>
              <th className="text-left py-3 px-4 font-semibold text-gray-700">Last Check</th>
              <th className="text-left py-3 px-4 font-semibold text-gray-700">Temperature</th>
              <th className="text-left py-3 px-4 font-semibold text-gray-700">Heart Rate</th>
              <th className="text-left py-3 px-4 font-semibold text-gray-700">SpO2</th>
              <th className="text-left py-3 px-4 font-semibold text-gray-700">Weight</th>
              <th className="text-left py-3 px-4 font-semibold text-gray-700">Status</th>
              <th className="text-left py-3 px-4 font-semibold text-gray-700">Actions</th>
            </tr>
          </thead>
          <tbody>
            {filteredPatients.map((patient) => (
              <tr key={patient.id} className="border-b border-gray-100 hover:bg-gray-50">
                <td className="py-3 px-4 font-mono text-blue-600">{patient.rfid || 'N/A'}</td>
                <td className="py-3 px-4 font-medium text-gray-900">{patient.name}</td>
                <td className="py-3 px-4 text-gray-700">{patient.age} years</td>
                <td className="py-3 px-4 text-gray-500">{patient.lastCheck || 'None'}</td>
                
                {/* Température avec couleur conditionnelle */}
                <td className="py-3 px-4">
                  <span className={getValueColor(patient, 'temp')}>
                    {patient.temp ? `${patient.temp}°C` : 'N/A'}
                  </span>
                </td>
                
                {/* Rythme cardiaque avec couleur conditionnelle */}
                <td className="py-3 px-4">
                  <span className={getValueColor(patient, 'heartRate')}>
                    {patient.heartRate ? `${patient.heartRate} bpm` : 'N/A'}
                  </span>
                </td>
                
                {/* SpO2 avec couleur conditionnelle */}
                <td className="py-3 px-4">
                  <span className={getValueColor(patient, 'spo2')}>
                    {patient.spo2 ? `${patient.spo2}%` : 'N/A'}
                  </span>
                </td>
                
                <td className="py-3 px-4 text-gray-700">
                  {patient.weight ? `${patient.weight} kg` : 'N/A'}
                </td>
                
                {/* Statut avec couleur conditionnelle */}
                <td className="py-3 px-4">
                  <span className={`inline-flex items-center px-2.5 py-0.5 rounded-full text-xs font-medium ${getStatusClass(patient.status)}`}>
                    {getStatusText(patient.status)}
                  </span>
                </td>
                
                <td className="py-3 px-4">
                  <div className="flex gap-2 items-center justify-start">
                    {/* Bouton Voir détails */}
                    <Link 
                      to={`/patients/${patient.id}`}
                      className="inline-flex items-center gap-1 px-3 py-1.5 bg-blue-50 text-blue-700 rounded-md hover:bg-blue-100 transition-colors duration-200 font-medium text-sm border border-blue-200 hover:border-blue-300"
                    >
                      👁️ View
                    </Link>

                    {/* Bouton Modifier */}
                    <button 
                      onClick={() => handleEditPatient(patient.id)}
                      className="inline-flex items-center gap-1 px-3 py-1.5 bg-green-50 text-green-700 rounded-md hover:bg-green-100 transition-colors duration-200 font-medium text-sm border border-green-200 hover:border-green-300"
                      title="Edit patient"
                    >
                      <Edit2 size={14} />
                      Edit
                    </button>

                    {/* Bouton Supprimer */}
                    <button 
                      onClick={() => setDeleteConfirm(patient.id)}
                      className="inline-flex items-center gap-1 px-3 py-1.5 bg-red-50 text-red-700 rounded-md hover:bg-red-100 transition-colors duration-200 font-medium text-sm border border-red-200 hover:border-red-300"
                      title="Delete patient"
                    >
                      <Trash2 size={14} />
                      Delete
                    </button>
                  </div>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
        
        {filteredPatients.length === 0 && (
          <div className="text-center py-12 text-gray-500">
            <div className="text-lg mb-2">No patients found</div>
            <div className="text-sm">
              {searchTerm ? 'Try with different search terms' : 'No patients in the database'}
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default Patients;