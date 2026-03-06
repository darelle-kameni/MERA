@echo off
echo Configuration IP fixe pour Serveur Robot Medical
netsh interface ip set address "Wi-Fi" static 192.168.1.100 255.255.255.0 192.168.1.1
echo ✅ IP fixe configuree : 192.168.1.100
pause