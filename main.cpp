
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <unordered_set>
#include <mutex>
#include <thread>


using uint64 = unsigned long long;
using byte = unsigned char;


constexpr size_t buf_size = 65536;
//udp port
constexpr int RELAY_PORT = 18720;
constexpr int RELAYS_SIZE = 32;

std::string addr_to_string(const in_addr& addr) {
    char client_str[128];
    inet_ntop(AF_INET, &(addr), client_str, INET_ADDRSTRLEN);
    return client_str;

}

std::string sockaddr_to_hostport(const sockaddr_in& addr) {
    return addr_to_string(addr.sin_addr) + ":" + std::to_string(addr.sin_port);
}

std::mutex peers_lock;


//port -> sock
std::unordered_map<uint64, sockaddr_in> peers;


//make port binds "permanent"?
bool add_peer(uint64 id, const sockaddr_in& peer_sock) {
    std::lock_guard lock(peers_lock);
    //peers.erase(id);
    auto [res0, res1] = peers.emplace(id, peer_sock);
    return res1;
}

const sockaddr_in* find_peer(uint64 id) {
    std::lock_guard lock(peers_lock);
    auto it = peers.find(id);
    if(it == peers.end())
        return nullptr;
    return &(it->second);
}







void start_on_port(int port) {
    int sockfd; 
    char packet_buffer[buf_size];
    
    struct sockaddr_in servaddr, cliaddr; 
       
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
       
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.2");
    servaddr.sin_port = htons(port); 
       
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    socklen_t len;
    int n; 

    //std::cout << "Listening on port " << port << std::endl;
    while(true) {
        len = sizeof(cliaddr);

        n = recvfrom(sockfd, &packet_buffer, buf_size,  
                    0, ( struct sockaddr *) &cliaddr, 
                    &len); 
        //biggest networking pitfall
        int client_port = ntohs(cliaddr.sin_port);
        
        //TODO: add authentication for client

        //add to list of peers
        bool can_bind = false;
        bool attempting_bind = (port == client_port);
        if(attempting_bind) 
            
            can_bind = add_peer(client_port, cliaddr);
        if(can_bind)
            std::cout << "New client " << sockaddr_to_hostport(cliaddr) << " bound to " << port << std::endl;

        //If src = dst, reply by setting first byte
        if(attempting_bind) {
            //success = 55, fail = ff
            if(can_bind)
                packet_buffer[0] = 0x55;
            else
                packet_buffer[0] = 0xff;
            //echo back to the client
            sendto(sockfd, &packet_buffer, n, 0, (sockaddr*)&cliaddr, sizeof(cliaddr));
            //sockaddr should equal cliaddr here
            continue;
                
        }
        

        //find requested peer
        const sockaddr_in* send_addr = find_peer(port);
        //drop if didnt find peer
        if(send_addr == nullptr)
            continue;


        int sent_n = sendto(sockfd, &packet_buffer, n, 0, (sockaddr*)send_addr, sizeof(*send_addr));
    }    
}


int main() {
    std::vector<std::thread> threads;
    for(int i = RELAY_PORT; i < RELAY_PORT + RELAYS_SIZE; i++) {
        threads.emplace_back(start_on_port, i);
    }
    //now just hang i guess?
    while(true) { sleep(10); }
}
