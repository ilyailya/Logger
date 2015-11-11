#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h> 
#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>


using namespace std;
int type;
std::string getData(){
    std::string message;
    cout << "Enter type of message:" << endl << "0 - info"<< endl << "1 - warn" << endl << "2 - error" << endl;
    cout << "3 - get info messages" << endl << "4 - get warn messages" << endl << "5 - get error messages" << endl;
    cout << "6 - get messages from date" << endl << "7 - get messages from time interval" << endl;
    cout << "8 - delete all messages" << endl;
    cin >> type;
    if (type >= 0 && type <= 2){
        cout << "Enter message:" << endl;
        cin >> message;
        message = to_string(type) + "|" + message + "|\n";
    }else if((type >= 3 && type <= 5) || type == 8){
        message = to_string(type) + "|\n";
    }else if(type == 6){
        string date;
        string globDate;
        cout << "Enter date in ASCII format (like Tue Nov 10 22:59:35 2015)" << endl;
        for (int i = 0; i < 4; ++i)
        {
            cin >> date;
            globDate += date + " ";
        }
            cin >> date;
            globDate += date;
        message = to_string(type) + "|" + globDate + "|\n";
    }else if(type == 7){
        string from, to, globfrom, globto;
        cout << "Enter start date in ASCII format (like Tue Nov 10 22:59:35 2015)" << endl;
        for (int i = 0; i < 4; ++i){
            cin >> from;
            globfrom += from + " ";
        }
            cin >> from;
            globfrom += from;
        cout << "Enter end date in ASCII format" << endl;
        for (int i = 0; i < 4; ++i){
            cin >> to;
            globto += to + " ";
        }
            cin >> to;
            globto += to;
        message = to_string(type) + "|" + globfrom + "|" + globto + "|\n";
    }else{
        cout << endl << endl << endl << "Wrong type, try again" << endl << endl << endl;
        message = getData();
    }
    return message;
}



int main()
{
    int sock, answerSock,pairOfPorts, port, answerPort;
    struct sockaddr_in addr, answerAddr;
    cout << "Select pair of ports:" << endl << "1 (3425 and 3426)" << endl << "2 (3427 and 3428)" << endl << "3 (3429 and 3430)"<< endl;
    cin >> pairOfPorts;
    switch(pairOfPorts){
        case 1: 
            port = 3425;
            answerPort = 3426;
            break;
        case 2: 
            port = 3427;
            answerPort = 3428;
            break;
        case 3: 
            port = 3429;
            answerPort = 3430;
            break;
        default:
            port = 3425;
            answerPort = 3426;
    }
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);



    answerSock = socket(AF_INET, SOCK_DGRAM, 0);
    if(answerSock < 0)
    {
        perror("socket");
        exit(1);
    }
    answerAddr.sin_family = AF_INET;
    answerAddr.sin_port = htons(answerPort);
    answerAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if(bind(answerSock, (struct sockaddr *)&answerAddr, sizeof(answerAddr)) < 0)
    {   
        perror("bind");            
        exit(2);
    }


    while(1){
    const char * c  = getData().c_str();
    
    sendto(sock, c, 1024, 0,
           (struct sockaddr *)&addr, sizeof(addr));
    char buf[1024];
    if (type >= 3){
        
        while(1){
            
            int bytes_read = recvfrom(answerSock, buf, 1024, 0, NULL, NULL);
            buf[bytes_read] = '\0';
            cout << buf << endl;
            if(string(buf) == "end\n"){
                break;
            }
        }
    }
    }
    close(sock);
    close(answerSock);
    return 0;
}