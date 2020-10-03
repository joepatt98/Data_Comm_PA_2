// Authors: Joseph Patterson and Stephanie Shisler
// NetIDs: jhp232 and xxxxxx

#include <stdlib.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <sys/types.h>   // defines types (like size_t)
#include <sys/socket.h>  // defines socket class
#include <netinet/in.h>  // defines port numbers for (internet) sockets, some address structures, and constants
#include <netdb.h>
#include <fstream>
#include <arpa/inet.h>   // if you want to use inet_addr() function
#include "packet.h"
#include <math.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <sstream>

using namespace std;

int main(int argc, char *argv[]){

  vector <string> argList(argv, argv + argc);

  stringstream ss;
  ss << argList[1];
  int n_port;
  ss >> n_port;
  //int n_port = stoi(argList[1]);

  struct sockaddr_in server;
  struct sockaddr_in client;

  int mysocket = 0;
  socklen_t clen = sizeof(client);

  // sets up UDP socket for file transmission
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    cout << "Error in creating UDP socket.\n";

  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(n_port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(mysocket, (struct sockaddr *)&server, sizeof(server)) == -1)
    cout << "Error in binding.\n";

  char s_payload[4];

  if (recvfrom(mysocket, s_payload, 4, 0, (struct sockaddr *)&client, &clen)==-1)
    cout << "Failed to receive.\n";

  //cout << "Received size of file:\n" << s_payload << endl;

  stringstream ssss;
  ssss << s_payload;
  int f_payload_size;
  ssss >> f_payload_size;
  //int f_payload_size = stoi(s_payload);
  char f_payload[f_payload_size];

  if (recvfrom(mysocket, f_payload, f_payload_size, 0, (struct sockaddr *)&client, &clen)==-1)
    cout << "Failed to receive.\n";

  //cout << "Received file:\n" << f_payload << endl;

  ofstream file("dataReceived.txt");
  file << f_payload;
  file.close();

  //char ack[512] = "Got all that data, thanks!";

  if (sendto(mysocket, f_payload, f_payload_size, 0, (struct sockaddr *)&client, clen)==-1){
    cout << "Error in sendto function.\n";
  }

  close(mysocket);

  return 0;
}
