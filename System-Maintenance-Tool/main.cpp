#include <iostream>
#include <cstdlib>
#include <string>
#include <iomanip>
#include <windows.h>
#include <fstream>
#include <pdh.h>
#include <pdhmsg.h>
#include <wininet.h>

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "pdh.lib")

/*
 * System Maintenance Tool
 * Copyright (C) 2025 Greivin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

using namespace std;

//Logs
void logAction(const string& mensaje)
{
    // Crear el nombre del archivo basado en la fecha
    time_t now = time(nullptr);
    tm* t = localtime(&now);

    char fecha[20];
    strftime(fecha, sizeof(fecha), "%Y-%m-%d", t);

    string filename = string("logs_") + fecha + ".txt";

    // Abrir archivo en modo append
    ofstream logFile(filename, ios::app);

    if (!logFile)
        return;

    // Registrar fecha y hora
    char fechaHora[40];
    strftime(fechaHora, sizeof(fechaHora), "%Y-%m-%d %H:%M:%S", t);

    logFile << "[" << fechaHora << "] " << mensaje << "\n";
    logFile.close();
}

// Utilidad para pausar y limpiar
void pauseAndClear()
{
    cout << "\nPresione ENTER para continuar...";
    cin.ignore();
    cin.get();
    system("cls");
}

// ======================
// Funciones de mantenimiento
// ======================

void escaneoPuertos()
{
    logAction("Ejecutando escaneo de puertos (netstat -ano)...\n\n");
    cout << "Ejecutando escaneo de puertos (netstat -ano)...\n\n";
    system("netstat -ano");
}

void escaneoSFC()
{
    logAction("Ejecutando SFC/scannow");
    cout << "Ejecutando SFC /scannow...\n";
    system("sfc /scannow");
}

void reparacionDISM()
{
    logAction("Ejecutando DISM /RestoreHealth...\n");
    cout << "Ejecutando DISM /RestoreHealth...\n";
    system("DISM /Online /Cleanup-Image /RestoreHealth");
}

void borrarTemporales()
{
    logAction("Borrando archivos temporales...\n");
    cout << "Borrando archivos temporales...\n";
    system("del /q /f %TEMP%\\*");
    system("rd /s /q C:\\Windows\\Temp");
    cout << "Archivos temporales eliminados.";
}

void limpiarDNS()
{
    logAction("Limpiando cache DNS...\n");
    cout << "Limpiando cache DNS...\n";
    system("ipconfig /flushdns");
}

void resetWinsock()
{
    logAction("Reseteando Winsock...\n");
    cout << "Reseteando Winsock...\n";
    system("netsh winsock reset");
}

void informacionSistema()
{
    logAction("Mostrando informacion del sistema...\n\n");
    cout << "Mostrando informacion del sistema...\n\n";
    system("systeminfo");
}

void informacionRed()
{
    logAction("Mostrando informacion de red (ipconfig /all)...\n\n");
    cout << "Mostrando informacion de red (ipconfig /all)...\n\n";
    system("ipconfig /all");
}

void limpiezaAvanzada()
{
    logAction("Limpieza avanzada...\n");
    cout << "Limpieza avanzada...\n";
    system("del /q /f C:\\Windows\\Prefetch\\*");
    system("del /q /f C:\\Windows\\SoftwareDistribution\\Download\\*");
    system("cleanmgr /sagerun:1");
    cout << "Limpieza avanzada completada.";
}

//Uso de la CPU
double getCPUUsage()
{
    static PDH_HQUERY query = nullptr;
    static PDH_HCOUNTER counter = nullptr;
    static bool initialized = false;

    if (!initialized)
    {
        if (PdhOpenQuery(NULL, NULL, &query) != ERROR_SUCCESS)
            return -1.0;

        if (PdhAddCounter(query, TEXT("\\Processor(_Total)\\% Processor Time"), NULL, &counter) != ERROR_SUCCESS)
            return -1.0;

        PdhCollectQueryData(query);
        initialized = true;
        Sleep(100);
    }

    PdhCollectQueryData(query);

    PDH_FMT_COUNTERVALUE value;
    PDH_STATUS status = PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, NULL, &value);

    if (status != ERROR_SUCCESS || value.CStatus != ERROR_SUCCESS)
        return -1.0;

    return value.doubleValue;
}

//Uso de RAM
void getRAMUsage(double& usedPercent, DWORDLONG& totalMB, DWORDLONG& freeMB)
{
    MEMORYSTATUSEX mem;
    mem.dwLength = sizeof(mem);
    GlobalMemoryStatusEx(&mem);

    totalMB = mem.ullTotalPhys / (1024 * 1024);
    freeMB = mem.ullAvailPhys / (1024 * 1024);

    usedPercent = mem.dwMemoryLoad; // Windows lo calcula directamente
}

//Rendimiento de Dsco
void getDiskUsage(const string& drive, double& usedPercent, unsigned long long& totalGB, unsigned long long& freeGB)
{
    ULARGE_INTEGER freeBytes, totalBytes, freeBytesTotal;

    GetDiskFreeSpaceEx(drive.c_str(), &freeBytes, &totalBytes, &freeBytesTotal);

    totalGB = totalBytes.QuadPart / (1024ULL * 1024ULL * 1024ULL);
    freeGB  = freeBytesTotal.QuadPart / (1024ULL * 1024ULL * 1024ULL);

    usedPercent = 100.0 - ((double)freeBytesTotal.QuadPart * 100.0 / (double)totalBytes.QuadPart);
}

//Rendimiento del Sistema.
void mostrarRendimiento()
{
    system("cls");
    cout << "====================================\n";
    cout << " Rendimiento del Sistema - GreCK01\n";
    cout << "====================================\n\n";

    // CPU
    double cpu = getCPUUsage();

    // RAM
    double ramPercent;
    DWORDLONG totalMB, freeMB;
    getRAMUsage(ramPercent, totalMB, freeMB);

    // Disco C:
    double diskPercent;
    unsigned long long diskTotal, diskFree;
    getDiskUsage("C:\\", diskPercent, diskTotal, diskFree);

    cout << "CPU Usage:      " << cpu << "%\n";
    cout << "RAM Total:      " << totalMB << " MB\n";
    cout << "RAM Libre:      " << freeMB << " MB\n";
    cout << "RAM Uso:        " << ramPercent << "%\n";
    cout << "Disco C Total:  " << diskTotal << " GB\n";
    cout << "Disco C Libre:  " << diskFree << " GB\n";
    cout << "Disco C Uso:    " << diskPercent << "%\n";

    logAction("Consulta de rendimiento del sistema (CPU, RAM y Disco)");
}

//Velocidad de descarga
void testDownloadSpeed()
{
    HINTERNET hInternet = InternetOpen("SpeedTest", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet)
    {
        cout << "Error al iniciar InternetOpen" << endl;
        return;
    }

    HINTERNET hUrl = InternetOpenUrl(
                         hInternet,
                         "http://speedtest.tele2.net/100MB.zip",
                         NULL, 0,
                         INTERNET_FLAG_RELOAD,
                         0
                     );

    if (!hUrl)
    {
        cout << "Error al abrir URL" << endl;
        InternetCloseHandle(hInternet);
        return;
    }

    const int bufferSize = 8192;
    char buffer[bufferSize];
    DWORD bytesRead = 0;

    DWORD totalBytes = 0;
    DWORD start = GetTickCount();

    while (InternetReadFile(hUrl, buffer, bufferSize, &bytesRead) && bytesRead > 0)
    {
        totalBytes += bytesRead;
    }

    DWORD end = GetTickCount();
    double seconds = (end - start) / 1000.0;
    double Mbps = (totalBytes * 8.0 / 1'000'000) / seconds;
    double MBps = Mbps / 8.0;

    cout << "Descargado: " << totalBytes / 1'000'000 << " MB" << endl;
    cout << "Tiempo: " << seconds << " segundos" << endl;
    cout << "Velocidad: " << Mbps << " Mbps" << endl;
    cout << "Velocidad aproximada: " << MBps << " MB/s" << endl;

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
}

//Ping
void testPing(const string& host)
{
    string command = "ping " + host + " -n 4";
    system(command.c_str());
}

void verProcesos()
{
    cout << "Lista de procesos activos:\n\n";
    system("tasklist");
}

// ======================
// Menú principal
// ======================

void mostrarMenu()
{
    system("Title System Maintenance Tool - by GreCK01");

    cout << "=========================================\n";
    cout << "         System Maintenance Tool         \n";
    cout << "=========================================\n";
    cout << "01. Escaneo de puertos\n";
    cout << "02. Escanear archivos corruptos (SFC)\n";
    cout << "03. Reparar imagen del sistema (DISM)\n";
    cout << "04. Borrar archivos temporales\n";
    cout << "05. Limpiar cache DNS\n";
    cout << "06. Reset Winsock\n";
    cout << "07. Ver informacion del sistema\n";
    cout << "08. Ver informacion de red\n";
    cout << "09. Limpieza avanzada\n";
    cout << "10. Ver procesos activos\n";
    cout << "11. Rendimiento Sistema\n";
    cout << "12. Test de Velocidad (Descarga)\n";
    cout << "13. Test Ping\n";
    cout << "0.  Salir\n";
    cout << "Seleccione una opcion: ";
}

int main()
{
    system("Title System Maintenance Tool - GreCK01");

    while (true)
    {
        system("cls");
        mostrarMenu();
        int opcion;
        cout << "Seleccione una opción: ";
        if (!(cin >> opcion))
        {
            cin.clear();      // Restablece cin
            cin.ignore(10000, '\n');   // Limpia basura
            cout << "Entrada inválida. Intente de nuevo.\n";
            continue;// Vuelve al menú
        }

        system("cls");

        switch (opcion)
        {
        case 1:
            escaneoPuertos();
            //system("pause");
            break;
        case 2:
            escaneoSFC();
            //system("pause");
            break;
        case 3:
            reparacionDISM();
            //system("pause");
            break;
        case 4:
            borrarTemporales();
            //system("pause");
            break;
        case 5:
            limpiarDNS();
            system("pause");
            break;
        case 6:
            resetWinsock();
            //system("pause");
            break;
        case 7:
            informacionSistema();
            //system("pause");
            break;
        case 8:
            informacionRed();
            //system("pause");
            break;
        case 9:
            limpiezaAvanzada();
            system("pause");
            break;
        case 10:
            verProcesos();
            //system("pause");
            break;
        case 11:
            logAction("Mostrando rendimiento del sistema");
            mostrarRendimiento();
            break;
        case 12:
            logAction("Mostrando velocidad de descarga");
            testDownloadSpeed();
            break;
        case 13:
        {
            string salir = "S";
            do
            {
                logAction("Mostrando el ping");
                string host;

                cout << "------------------------------------------------------------" << endl;
                cout << "Ingrese la dirección a hacer ping (IP o dominio): ";
                cin >> host;
                cout << endl;
                cout << "+----------------------------------------------------------------+" << endl;
                testPing(host);

                cout << endl;
                cout << "+----------------------------------------------------------------+" << endl;

                cout << endl;
                cout << "=============================================================" << endl;
                cout << "¿Quiere seguir con otra direccion para testear el Ping? (S/N)" << endl;
                cin >> salir;
            }
            while(salir == "S" || salir == "s");

            break;
        }

        case 0:
        {
            cout << "Saliendo del programa...";
            return 0;
        }

        default:
        {
            cout << "Opcion invalida.";
        }
        }
        pauseAndClear();
    }
    return 0;
}
