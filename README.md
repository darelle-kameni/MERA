# Robot Medical - Real-Time Medical Monitoring System

Modern web application for real-time monitoring of patient medical parameters.

## Features

- Real-time dashboard with medical metrics
- Patient management with RFID tracking
- Parameter monitoring (temperature, heart rate, SpO2, weight)
- Automatic alerts for critical values
- Responsive and accessible interface
- Secure authentication

## Technologies

- **Frontend**: React 18, Vite, Tailwind CSS, Lucide Icons
- **Backend**: PHP 8+, MySQL
- **API Server**: Node.js/Express (ESP32 data collection)
- **Security**: JWT, CORS, Security Headers
- **Performance**: useReducer, useMemo, Optimized Polling

## Prerequisites

- Node.js 18+
- PHP 8.0+
- MySQL 8.0+
- Web server (Apache/Nginx)

## Platform-Specific Notes

### Windows

For Windows users, ensure the following are installed:
- Download PHP from [php.net](https://www.php.net/downloads)
- Download MySQL from [mysql.com](https://dev.mysql.com/downloads/)
- Use `cmd.exe` or PowerShell for commands
- Replace `sudo` with Administrator privileges (right-click and "Run as administrator")
- Use backslashes `\` for file paths or double slashes `\\`

### macOS / Linux

Standard Unix commands apply. Use `sudo` for system-level operations.

## Quick Start with XAMPP

If you're using XAMPP (includes Apache, PHP, and MySQL), follow these simplified steps:

### 1. Start XAMPP Services

1. Open XAMPP Control Panel
2. Start: Apache, MySQL, and PHP (if available)
3. Open MySQL Admin to configure the database

### 2. Setup Database in XAMPP

```sql
-- In phpMyAdmin or MySQL command line
CREATE DATABASE robot_medical;
CREATE USER 'dark-linux'@'localhost' IDENTIFIED BY 'password';
GRANT ALL PRIVILEGES ON robot_medical.* TO 'dark-linux'@'localhost';
FLUSH PRIVILEGES;

-- Import the schema
-- Upload backend/databases/robot_medical.sql through phpMyAdmin
-- Or use: mysql -u dark-linux -p robot_medical < backend/databases/robot_medical.sql
```

### 3. Configure .env for XAMPP

```bash
cp .env.example .env
```

Edit `.env`:
```
VITE_API_BASE_URL=http://localhost:8000
DB_HOST=localhost
DB_USER=dark-linux
DB_PASS=password
DB_NAME=robot_medical
```

### 4. Start Everything

**Option A - Separate terminals:**

Terminal 1 - Frontend:
```bash
npm install
npm run dev
```

Terminal 2 - Backend (if not using Apache):
```bash
php -S 0.0.0.0:8000
```

Terminal 3 - API Server:
```bash
cd robot-api
npm install
npm start
```

**Option B - All in one npm command:**

Update `package.json` scripts to:
```json
{
  "scripts": {
    "dev": "concurrently \"npm run dev:frontend\" \"npm run dev:backend\" \"npm run dev:api\"",
    "dev:frontend": "vite",
    "dev:backend": "php -S 0.0.0.0:8000",
    "dev:api": "cd robot-api && npm start"
  }
}
```

Then install concurrently:
```bash
npm install --save-dev concurrently
```

And run:
```bash
npm run dev
```

This will start all three services in parallel.

## Installation & Setup

### 1. Database Configuration

**Linux/macOS:**
```bash
# Create the database
sudo mysql -e "CREATE DATABASE IF NOT EXISTS robot_medical;"

# Create the user
sudo mysql -e "CREATE USER IF NOT EXISTS 'dark-linux'@'localhost'; GRANT ALL PRIVILEGES ON robot_medical.* TO 'dark-linux'@'localhost'; FLUSH PRIVILEGES;"

# Import the schema
sudo mysql robot_medical < backend/databases/robot_medical.sql
```

**Windows (PowerShell or CMD as Administrator):**
```powershell
# Open MySQL and run commands
mysql -u root -p

# In MySQL prompt:
CREATE DATABASE IF NOT EXISTS robot_medical;
CREATE USER IF NOT EXISTS 'dark-linux'@'localhost' IDENTIFIED BY 'password';
GRANT ALL PRIVILEGES ON robot_medical.* TO 'dark-linux'@'localhost';
FLUSH PRIVILEGES;

# Exit MySQL
EXIT;

# Import the schema
mysql -u dark-linux -p robot_medical < backend\databases\robot_medical.sql
```

### 2. Environment Setup

```bash
# Copy environment file
cp .env.example .env

# Edit configuration
nano .env
```

### 3. Frontend Installation

```bash
# Install dependencies
npm install

# Start development server
npm run dev
```

Frontend will be available at `http://localhost:5173`

### 4. Backend Configuration

**Linux/macOS:**
```bash
# Start PHP built-in server (from root directory)
php -S 0.0.0.0:8000
```

**Windows (PowerShell or CMD):**
```powershell
# Navigate to root directory if not already there
cd path\to\robot-medical

# Start PHP built-in server
php -S 0.0.0.0:8000
```

Backend will be available at `http://localhost:8000`

### 5. API Server (ESP32 Data Collection)

```bash
# Navigate to API server
cd robot-api

# Install dependencies
npm install

# Start the server
npm start
```

API Server will be available at `http://localhost:3001`

### 6. Full Production Build

```bash
# Build the application
npm run build

# Preview production build
npm run preview
```

## Configuration

### Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `VITE_API_BASE_URL` | Backend URL | `http://localhost:8000` |
| `DB_HOST` | MySQL host | `localhost` |
| `DB_NAME` | Database name | `robot_medical` |
| `DB_USER` | MySQL user | `dark-linux` |
| `JWT_SECRET` | JWT secret key | (to be defined) |

### Ports Used

- Frontend: 5173 (development)
- Backend: 8000 (PHP server)
- API Server: 3001 (Express server)
- Database: 3306 (MySQL)

## Usage

1. Access the application at `http://localhost:5173`
2. View real-time metrics on the dashboard
3. Manage patient records
4. Monitor medical measurements
5. Check critical value alerts

## Security

- JWT authentication
- CORS security headers
- Data validation
- SQL injection protection
- Rate limiting (to be implemented)

## Responsive Design

The application is optimized for:
- Desktop (1024px+)
- Tablet (768px - 1023px)
- Mobile (< 768px)

## Troubleshooting

### Common Issues

1. **CORS Error**
   - Verify backend is listening on the correct port
   - Check CORS headers in `backend/config/cors.php`

2. **MySQL Connection Failed**
   - Verify `dark-linux` user exists
   - Confirm permissions on `robot_medical` database

3. **Frontend Won't Load**
   - Verify Node.js 18+ is installed
   - Run `npm install` to install dependencies

### Logs

- Frontend: Browser console
- Backend: File specified in `LOG_FILE`
- Database: MySQL logs

## Monitoring

The application includes:
- Performance metrics
- Structured logging
- Automatic alerts
- Loading skeletons

## Contributing

1. Fork the project
2. Create a feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is under the MIT License. See the `LICENSE` file for details.

## Support

For questions or issues:
1. Check the documentation
2. Review error logs
3. Open an issue on GitHub
