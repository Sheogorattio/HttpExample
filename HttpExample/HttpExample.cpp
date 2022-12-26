#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

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

    string uri = "/data/2.5/weather?q=Odessa&units=metric&appid=75f6e64d49db78658d09cb5ab201e483&mode=JSON";

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

  //  cout << response << endl;
    wetherInfo wether;
    int place;
    place = response.rfind("\"id\":", response.size());                          //id
    for (int i = place + sizeof("\"id\":") - 1; response[i] != ','; i++) {
        wether.id.push_back(response[i]);
    }
    place = response.rfind("\"name\":\"", response.size());                     //city
    for (int i = place + sizeof("\"name\":\"") - 1; response[i] != '\"'; i++) {
        wether.city.push_back(response[i]);
    }
    place = response.rfind("\"country\":\"", response.size());                      //country
    for (int i = place + sizeof("\"country\":\"") - 1; response[i] != '\"'; i++) {
        wether.country.push_back(response[i]);
    }
    place = response.find("\"coord\":{", 0);                                        //coord
    for (int i = place + sizeof("\"coord\":{") - 1; response[i] != '\}'; i++) {
        wether.coord.push_back(response[i]);
    }
    place = response.find("\"temp_min\":", 0);                                      //max/max temperature
    for (int i = place + sizeof("\"temp_min\":") - 1; response[i] != ','; i++) {
        wether.tempMin.push_back(response[i]);
    }
    place = response.find("\"temp_max\":", 0);
    for (int i = place + sizeof("\"temp_max\":") - 1; response[i] != ','; i++) {
        wether.tempMax.push_back(response[i]);
    }
    place = response.find("\"sunrise\":", 0);                                         //sunrise
    for (int i = place + sizeof("\"sunrise\":") - 1; response[i] != ','; i++) {
        wether.sunrise.push_back(response[i]);
    }
    place = response.find("\"sunset\":", 0);                                         //sunset
    for (int i = place + sizeof("\"sunset\":") - 1; response[i] != ','; i++) {
        wether.sunset.push_back(response[i]);
    }


    //tm* sunset = new tm, *sunrise = new tm;
    //gmtime_s(sunrise, (time_t*)stol(wether.sunrise));
    //gmtime_s(sunset, (time_t*)stol(wether.sunset));

    cout << "id:\t\t" << wether.id << endl;
    cout << "city:\t\t" << wether.city << endl;
    cout << "country:\t" << wether.country << endl;
    cout << "coord:\t\t" << wether.coord << endl;
    cout << "t(min/max):\t" << wether.tempMin << " / " <<wether.tempMax << endl;
    cout << "sunrise:\t" << stol(wether.sunrise) / 3600 % 24 << ":" << stol(wether.sunrise) / 60 % 60 << endl;
    cout << "sunset:\t\t" << stol(wether.sunset) / 3600 % 24 << ":" << stol(wether.sunset) / 60 % 60 <<endl;





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