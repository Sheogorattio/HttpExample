#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

#include<fstream>
#include <iostream>
#include <string>
using namespace std;


struct wetherInfo {
    string id;
    string city;
    string country;
    string coord;
    string tempMin;
    string tempMax;
    string sunrise;
    string sunset;
};

int main()
{
    setlocale(0, "ru");

    //1. инициализация "Ws2_32.dll" для текущего процесса
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {

        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }  

    //инициализация структуры, для указания ip адреса и порта сервера с которым мы хотим соединиться
   
    char hostname[255] = "api.openweathermap.org";
    
    addrinfo* result = NULL;    
    
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo(hostname, "http", &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << endl;
        WSACleanup();
        return 3;
    }     

    SOCKET connectSocket = INVALID_SOCKET;
    addrinfo* ptr = NULL;

    //Пробуем присоединиться к полученному адресу
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        //2. создание клиентского сокета
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

       //3. Соединяемся с сервером
        iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    //4. HTTP Request
    cout << "Enter city:\t";
    string city;
    cin >> city;
    char lpszRequest[255];
    sprintf_s(lpszRequest, "/data/2.5/weather?q=%s&units=metric&appid=75f6e64d49db78658d09cb5ab201e483&mode=JSON", city.c_str());

    string uri = lpszRequest;

    string request = "GET " + uri + " HTTP/1.1\n"; 
    request += "Host: " + string(hostname) + "\n";
    request += "Accept: */*\n";
    request += "Accept-Encoding: gzip, deflate, br\n";   
    request += "Connection: close\n";   
    request += "\n";

    //отправка сообщения
    if (send(connectSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
        cout << "send failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 5;
    }
    cout << "send data" << endl;

    //5. HTTP Response

    string response;

    const size_t BUFFERSIZE = 1024;
    char resBuf[BUFFERSIZE];

    int respLength;

    do {
        respLength = recv(connectSocket, resBuf, BUFFERSIZE, 0);
        if (respLength > 0) {
            response += string(resBuf).substr(0, respLength);           
        }
        else {
            cout << "recv failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 6;
        }

    } while (respLength == BUFFERSIZE);

    wetherInfo weather;
    int place;
    place = response.rfind("\"id\":", response.size());                          //id
    for (int i = place + sizeof("\"id\":") - 1; response[i] != ','; i++) {
        weather.id.push_back(response[i]);
    }
    place = response.rfind("\"name\":\"", response.size());                     //city
    for (int i = place + sizeof("\"name\":\"") - 1; response[i] != '\"'; i++) {
        weather.city.push_back(response[i]);
    }
    place = response.rfind("\"country\":\"", response.size());                      //country
    for (int i = place + sizeof("\"country\":\"") - 1; response[i] != '\"'; i++) {
        weather.country.push_back(response[i]);
    }
    place = response.find("\"coord\":{", 0);                                        //coord
    for (int i = place + sizeof("\"coord\":{") - 1; response[i] != '\}'; i++) {
        weather.coord.push_back(response[i]);
    }
    place = response.find("\"temp_min\":", 0);                                      //max/max temperature
    for (int i = place + sizeof("\"temp_min\":") - 1; response[i] != ','; i++) {
        weather.tempMin.push_back(response[i]);
    }
    place = response.find("\"temp_max\":", 0);
    for (int i = place + sizeof("\"temp_max\":") - 1; response[i] != ','; i++) {
        weather.tempMax.push_back(response[i]);
    }
    place = response.find("\"sunrise\":", 0);                                         //sunrise
    for (int i = place + sizeof("\"sunrise\":") - 1; response[i] != ','; i++) {
        weather.sunrise.push_back(response[i]);
    }
    place = response.find("\"sunset\":", 0);                                         //sunset
    for (int i = place + sizeof("\"sunset\":") - 1; response[i] != ','; i++) {
        weather.sunset.push_back(response[i]);
    }



    cout << "id:\t\t" << weather.id << endl;
    cout << "city:\t\t" << weather.city << endl;
    cout << "country:\t" << weather.country << endl;
    cout << "coord:\t\t" << weather.coord << endl;
    cout << "t(min/max):\t" << weather.tempMin << " / " << weather.tempMax << endl;
    cout << "sunrise:\t" << stol(weather.sunrise) / 3600 % 24 << ":" << stol(weather.sunrise) / 60 % 60 << endl;
    cout << "sunset:\t\t" << stol(weather.sunset) / 3600 % 24 << ":" << stol(weather.sunset) / 60 % 60 <<endl;


    ofstream ToFile("Weather.txt", ios::app | ios::out);
    if (ToFile.is_open())
    {
        ToFile << "id:\t\t" << weather.id << endl;
        ToFile << "city:\t\t" << weather.city << endl;
        ToFile << "country:\t" << weather.country << endl;
        ToFile << "coord:\t\t" << weather.coord << endl;
        ToFile << "t(min/max):\t" << weather.tempMin << " / " << weather.tempMax << endl;
        ToFile << "sunrise:\t" << stol(weather.sunrise) / 3600 % 24 << ":" << stol(weather.sunrise) / 60 % 60 << endl;
        ToFile << "sunset:\t\t" << stol(weather.sunset) / 3600 % 24 << ":" << stol(weather.sunset) / 60 % 60 << endl;
        ToFile << "---------------------------------------------------------------\n";
        ToFile.close();
    }

    //отключает отправку и получение сообщений сокетом
    iResult = shutdown(connectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 7;
    }

    closesocket(connectSocket);
    WSACleanup();
}