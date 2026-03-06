import React from 'react';
import { User, Settings, Mail, Database } from 'lucide-react';

const Administration = () => {
  const adminSections = [
    {
      icon: User,
      title: 'User Management',
      description: 'Manage admin accounts and permissions',
      items: ['Add user', 'Modify permissions', 'Connection log']
    },
    {
      icon: Mail,
      title: 'Communication',
      description: 'Send data by email and manage templates',
      items: ['Send reports', 'Email templates', 'Sending history']
    },
    {
      icon: Settings,
      title: 'System Settings',
      description: 'Configure application settings',
      items: ['Alert thresholds', 'RFID settings', 'API configuration']
    },
    {
      icon: Database,
      title: 'Data Management',
      description: 'Database backup and maintenance',
      items: ['Automatic backup', 'Data cleanup', 'Full export']
    },
  ];

  return (
    <div className="space-y-6">
      <h1 className="text-3xl font-bold text-gray-800">Administration</h1>

      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        {adminSections.map((section, index) => {
          const Icon = section.icon;
          return (
            <div key={index} className="card p-6">
              <div className="flex items-center space-x-3 mb-4">
                <Icon className="text-blue-600" size={24} />
                <h3 className="text-xl font-semibold text-gray-800">{section.title}</h3>
              </div>
              <p className="text-gray-600 mb-4">{section.description}</p>
              <ul className="space-y-2">
                {section.items.map((item, itemIndex) => (
                  <li key={itemIndex} className="flex items-center space-x-2 text-gray-700">
                    <div className="w-1 h-1 bg-blue-600 rounded-full"></div>
                    <span>{item}</span>
                  </li>
                ))}
              </ul>
              <button className="w-full mt-4 btn-primary">
                Manage {section.title}
              </button>
            </div>
          );
        })}
      </div>

      {/* Statistiques système */}
      <div className="card p-6">
        <h3 className="text-lg font-semibold text-gray-800 mb-4">System Statistics</h3>
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4 text-center">
          <div>
            <div className="text-2xl font-bold text-blue-600">99.8%</div>
            <div className="text-gray-600 text-sm">Availability</div>
          </div>
          <div>
            <div className="text-2xl font-bold text-green-600">2.3s</div>
            <div className="text-gray-600 text-sm">Response time</div>
          </div>
          <div>
            <div className="text-2xl font-bold text-purple-600">15.2K</div>
            <div className="text-gray-600 text-sm">Stored measurements</div>
          </div>
          <div>
            <div className="text-2xl font-bold text-orange-600">24/7</div>
            <div className="text-gray-600 text-sm">Monitoring</div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default Administration;