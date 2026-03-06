import React, { useState } from 'react';
import { Link, useNavigate } from 'react-router-dom';
import { ArrowLeft, Save, UserPlus } from 'lucide-react';

const AddPatient = () => {
  const navigate = useNavigate();
  const [formData, setFormData] = useState({
    rfid: '',
    firstName: '',
    lastName: '',
    age: '',
    email: '',
    phone: '',
    address: '',
    emergencyContact: '',
    medicalHistory: ''
  });
  const [loading, setLoading] = useState(false); // ← AJOUTÉ
  const [error, setError] = useState(''); // ← AJOUTÉ

  const handleSubmit = async (e) => {
    e.preventDefault();
    setLoading(true);
    setError('');

    console.log('🔄 Starting form submission');
    console.log('📋 Form data:', formData);

    try {
      const apiUrls = [
        '/api/add_patient.php',
        'http://localhost/medical-robo-dashboardt/backend/add_patient.php',
        'http://127.0.0.1/medical-robot-dashboard/backend/add_patient.php'
      ];

      let success = false;
      let lastError = 'No successful attempt';

      for (const url of apiUrls) {
        try {
          console.log(`🔄 Attempting: ${url}`);
          
          const response = await fetch(url, {
            method: 'POST',
            headers: {
              'Content-Type': 'application/json',
              'Accept': 'application/json',
            },
            body: JSON.stringify(formData)
          });

          console.log(`📨 HTTP Response: ${response.status} ${response.statusText}`);
          
          // Check if response is JSON
          const contentType = response.headers.get('content-type');
          console.log(`📄 Content-Type: ${contentType}`);
          
          let data;
          if (contentType && contentType.includes('application/json')) {
            data = await response.json();
          } else {
            const text = await response.text();
            console.log('📝 Text response:', text);
            throw new Error(`Non-JSON response: ${text.substring(0, 100)}`);
          }

          console.log('✅ JSON data:', data);

          if (data.success) {
            console.log('✅ SUCCESS - Patient added:', data);
            alert(`✅ Patient added successfully!\nID: ${data.patient_id}\nRFID: ${data.rfid}`);
            navigate('/patients');
            success = true;
            break;
          } else {
            lastError = data.message || data.error || 'Unknown error';
            console.log(`❌ API Error ${url}:`, lastError);
          }

        } catch (err) {
          lastError = `${url}: ${err.message}`;
          console.log(`❌ Fetch error ${url}:`, err);
        }
      }

      if (!success) {
        setError(`Unable to add patient: ${lastError}`);
        console.log('⚠️ All URLs failed, entering simulation mode');
        
        // Simulation mode
        if (window.confirm('API is not responding. Do you want to simulate the addition to continue testing?')) {
          console.log('Simulation mode activated');
          alert('Simulation mode: Patient added successfully! (Data not saved)');
          navigate('/patients');
        }
      }

    } catch (err) {
      const errorMsg = `Unexpected error: ${err.message}`;
      setError(errorMsg);
      console.error('❌ Unexpected error:', err);
    } finally {
      setLoading(false);
      console.log('✅ End of submission');
    }
  };

  const handleChange = (e) => {
    setFormData({
      ...formData,
      [e.target.name]: e.target.value
    });
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div className="flex items-center space-x-4">
          <Link to="/patients" className="flex items-center space-x-2 text-gray-600 hover:text-blue-600">
            <ArrowLeft size={20} />
            <span>Back</span>
          </Link>
          <h1 className="text-3xl font-bold text-gray-800">Add a Patient</h1>
        </div>
        <UserPlus className="text-blue-600" size={32} />
      </div>

      {/* Error message */}
      {error && (
        <div className="bg-red-50 border border-red-200 rounded-lg p-4">
          <div className="text-red-700 font-medium">{error}</div>
        </div>
      )}

      {/* Form */}
      <div className="card p-6">
        <form onSubmit={handleSubmit} className="space-y-6">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
            {/* RFID ID */}
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">
                RFID ID *
              </label>
              <input
                type="text"
                name="rfid"
                value={formData.rfid}
                onChange={handleChange}
                required
                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
                placeholder="Ex: RFID001"
              />
            </div>

            {/* First Name */}
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">
                First Name *
              </label>
              <input
                type="text"
                name="firstName"
                value={formData.firstName}
                onChange={handleChange}
                required
                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
                placeholder="Patient first name"
              />
            </div>

            {/* Last Name */}
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">
                Last Name *
              </label>
              <input
                type="text"
                name="lastName"
                value={formData.lastName}
                onChange={handleChange}
                required
                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
                placeholder="Patient last name"
              />
            </div>

            {/* Age */}
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">
                Age *
              </label>
              <input
                type="number"
                name="age"
                value={formData.age}
                onChange={handleChange}
                required
                min="0"
                max="120"
                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
                placeholder="Age in years"
              />
            </div>

            {/* Email */}
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">
                Email
              </label>
              <input
                type="email"
                name="email"
                value={formData.email}
                onChange={handleChange}
                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
                placeholder="email@example.com"
              />
            </div>

            {/* Phone */}
            <div>
              <label className="block text-sm font-medium text-gray-700 mb-2">
                Phone Number
              </label>
              <input
                type="tel"
                name="phone"
                value={formData.phone}
                onChange={handleChange}
                className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
                placeholder="+33 1 23 45 67 89"
              />
            </div>
          </div>

          {/* Address */}
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">
              Address
            </label>
            <input
              type="text"
              name="address"
              value={formData.address}
              onChange={handleChange}
              className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
              placeholder="Full address"
            />
          </div>

          {/* Emergency Contact */}
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">
              Emergency Contact
            </label>
            <input
              type="text"
              name="emergencyContact"
              value={formData.emergencyContact}
              onChange={handleChange}
              className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
              placeholder="Contact name and phone number"
            />
          </div>

          {/* Medical History */}
          <div>
            <label className="block text-sm font-medium text-gray-700 mb-2">
              Medical History
            </label>
            <textarea
              name="medicalHistory"
              value={formData.medicalHistory}
              onChange={handleChange}
              rows="4"
              className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:outline-none focus:ring-2 focus:ring-blue-500"
              placeholder="Medical history, allergies, current treatments..."
            />
          </div>

          {/* Action buttons */}
          <div className="flex justify-end space-x-4 pt-6 border-t border-gray-200">
            <Link
              to="/patients"
              className="px-6 py-2 border border-gray-300 rounded-lg text-gray-700 hover:bg-gray-50 transition-colors"
            >
              Cancel
            </Link>
            <button
              type="submit"
              disabled={loading}
              className="btn-primary flex items-center space-x-2 disabled:opacity-50 disabled:cursor-not-allowed"
            >
              <Save size={20} />
              <span>{loading ? 'Saving...' : 'Save Patient'}</span>
            </button>
          </div>
        </form>
      </div>

      {/* Information */}
      <div className="card p-6 bg-blue-50 border border-blue-200">
        <h3 className="text-lg font-semibold text-blue-800 mb-2">Important Information</h3>
        <ul className="text-blue-700 space-y-1 text-sm">
          <li>• RFID ID must be unique for each patient</li>
          <li>• Fields marked with an asterisk (*) are required</li>
          <li>• The patient can be identified by the medical robot via their RFID card</li>
          <li>• Measurements will be automatically associated with this student profile</li>
        </ul>
      </div>
    </div>
  );
};

export default AddPatient;