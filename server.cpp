
#include <fstream> 
#include <sys/types.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <cstdlib>
#include <cstring>
#include <unistd.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <mutex>
#include <thread>
#include <errno.h>
#include <stdlib.h>
#include <sstream>

struct message{
    int type;
    std::string value;
    std::string ip_adress;
    time_t time;
    std::string sTime;
};

std::vector<message> messages;
std::ofstream file;
std::mutex s_lock;

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

void readLog(){
    std::ifstream log("log.txt");
    std::string s; 
    if (log){
        while(getline(log, s)){
            std::vector<std::string> elems = split(s, '|');
            if (elems[0] == "0" || elems[0] == "1" || elems[0] == "2" || elems[0] == "6" ){
                message m;
                m.type = stoi(elems[0]);
                m.value = elems[1];
                m.ip_adress = elems[2];
                m.sTime = elems[3];
                struct tm tt;
                strptime(elems[3].c_str(), "%a %b %d %X %Y", &tt);
                m.time = mktime(&tt);
                messages.push_back(m);
            }
            if (elems[0] == "8" || elems[0] == "3" || elems[0] == "4" || elems[0] == "5" ){
                message m;
                m.type = stoi(elems[0]);
                m.ip_adress = elems[1];
                m.sTime = elems[2];
                struct tm tt;
                strptime(elems[2].c_str(), "%a %b %d %X %Y", &tt);
                m.time = mktime(&tt);
                messages.push_back(m);
            }
            if (elems[0] == "7")
            {
                message m;
                m.type = stoi(elems[0]);
                m.value = elems[1] + "|" + elems[2];
                m.ip_adress = elems[3];
                m.sTime = elems[4];
                struct tm tt;
                strptime(elems[4].c_str(), "%a %b %d %X %Y", &tt);
                m.time = mktime(&tt);
                messages.push_back(m);
            }
        }
    }
    file.close();
}


std::string getDate()
{
    char *array = (char*)malloc(sizeof(char)*25);
    time_t result;
    result = time(NULL);
    sprintf(array, "%s", asctime(localtime(&result)));
    array[25] = '\0';
    return std::string(array);
}

void acceptMessage(int t, time_t from, time_t to, int answerPort){

    int answerSock;
    struct sockaddr_in answerAddr;
    answerSock = socket(AF_INET, SOCK_DGRAM, 0);
    if(answerSock < 0)
    {
        perror("socket");
        exit(1);
    }
    answerAddr.sin_family = AF_INET;
    answerAddr.sin_port = htons(answerPort);
    answerAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);


    if ((t >= 3) & (t <= 5)){
        for (message msg : messages){
            if (msg.type == t-3){
                std::string c = std::to_string(msg.type) + "|" +msg.value + "|" + msg.ip_adress + "|" + msg.sTime + "\n";
                sendto(answerSock, c.c_str(), 1024, 0,(struct sockaddr *)&answerAddr, sizeof(answerAddr));
            }
        }
        
        char c1[] = "end\n";
        sendto(answerSock, c1, sizeof(c1), 0,(struct sockaddr *)&answerAddr, sizeof(answerAddr));
    }else if(t == 6){
        for(auto msg : messages){
            if (from >= msg.time){
                std::string c = std::to_string(msg.type) + "|" +msg.value + "|" + msg.ip_adress + "|" + msg.sTime + "\n";
                sendto(answerSock, c.c_str(), 1024, 0,(struct sockaddr *)&answerAddr, sizeof(answerAddr));
            }
        }
        char c1[] = "end\n";
        sendto(answerSock, c1, sizeof(c1), 0,(struct sockaddr *)&answerAddr, sizeof(answerAddr));
    }else if(t == 7){
        for(auto msg : messages){
            if (from >= msg.time && msg.time <= to){
                std::string c = std::to_string(msg.type) + "|" +msg.value + "|" + msg.ip_adress + "|" + msg.sTime + "\n";
                sendto(answerSock, c.c_str(), 1024, 0,(struct sockaddr *)&answerAddr, sizeof(answerAddr));
            }
        }
        char c1[] = "end\n";
        sendto(answerSock, c1, sizeof(c1), 0,(struct sockaddr *)&answerAddr, sizeof(answerAddr));
    }else if(t == 8){
        s_lock.lock();
        file.open("log.txt", std::fstream::out | std::fstream::trunc);
        file.close();
        s_lock.unlock();
        messages.clear();
        char c1[] = "end\n";
        sendto(answerSock, c1, sizeof(c1), 0,(struct sockaddr *)&answerAddr, sizeof(answerAddr));
    }
    
    

}


void clientTreatment(int sock, int answerPort){
    struct sockaddr_in fromAddr;
    socklen_t fromAddrLen = sizeof fromAddr;
    char buf[1024];
    int bytes_read;
    while(1)
    {
        bytes_read = recvfrom(sock, buf, 1024, 0, (struct sockaddr *)&fromAddr, &fromAddrLen);
        char display[16] = {0};
        inet_ntop(AF_INET, &fromAddr.sin_addr.s_addr, display, sizeof display);
        buf[bytes_read-1] = '\0';
        for (int i = 0; i < 1023; ++i)
        {
            if (buf[i] == '\n'){
                buf[i] = 0;
            }
        }

        std::string tmp = std::string(buf);

        if ((buf[0] - '0' >= 3 && buf[0] - '0' <= 5) || (buf[0] - '0' == 8)){
            acceptMessage(buf[0] - '0', NULL, NULL, answerPort);
        }else if(buf[0] - '0' == 6){
            std::string from = tmp.substr(2, 24);
            struct tm time1;
            strptime(from.c_str(), "%a %b %d %X %Y", &time1);
            acceptMessage(buf[0] - '0', mktime(&time1), NULL, answerPort);
        }else if(buf[0] - '0' == 7){
            std::string from = tmp.substr(2, 24);
            struct tm time1;
            strptime(from.c_str(), "%a %b %d %X %Y", &time1);
            std::string to = tmp.substr(27, 24);
            struct tm time2;
            strptime(from.c_str(), "%a %b %d %X %Y", &time2);
            acceptMessage(buf[0] - '0', mktime(&time1), mktime(&time2), answerPort);
        }
        
        
        message msg;
        msg.type = tmp[0] - '0';
        msg.value = tmp.substr(2);
        msg.value = msg.value.substr(0, msg.value.size() - 1);

        msg.ip_adress = display;
        asctime(localtime(&msg.time));
        msg.sTime = getDate();
        messages.push_back(msg);

        tmp+=display;
        tmp+="|";
        tmp+=getDate() + "\0";
        s_lock.lock();
        file.open("log.txt", std::ios::out | std::ios::app );
        file << tmp;
        std::cout << tmp;
        file.close();
        s_lock.unlock();
    }
}

int main()
{

    readLog();
    int sock, sock2, sock3;
    struct sockaddr_in addr, addr2, addr3;
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3425);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {   
        perror("bind");            
        exit(2);
    }

    sock2 = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock2 < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr2.sin_family = AF_INET;
    addr2.sin_port = htons(3427);
    addr2.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock2, (struct sockaddr *)&addr2, sizeof(addr2)) < 0)
    {   
        perror("bind");            
        exit(2);
    }

    sock3 = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock3 < 0)
    {
        perror("socket");
        exit(1);
    }
    
    addr3.sin_family = AF_INET;
    addr3.sin_port = htons(3429);
    addr3.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(sock3, (struct sockaddr *)&addr3, sizeof(addr3)) < 0)
    {   
        perror("bind");            
        exit(2);
    }


    std::thread sock1_thread(clientTreatment, sock, 3426); 
    std::thread sock2_thread(clientTreatment, sock2, 3428);
    std::thread sock3_thread(clientTreatment, sock3, 3430);
    sock1_thread.join();
    sock2_thread.join();
    sock3_thread.join();
    
    return 0;
}