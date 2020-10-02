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

  char server_name[512];
  strcpy(server_name, argList[1].c_str());

  stringstream ss;
  ss << argList[2];
  int n_port;
  ss >> n_port;

  char filename[512];
  strcpy(filename, argList[3].c_str());

  struct hostent *s;
  s = gethostbyname(server_name);

  struct sockaddr_in server;
  int mysocket = 0;
  socklen_t slen = sizeof(server);

  // sets up UDP socket for file transmission
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    cout << "Error in creating UDP socket.\n";

  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(n_port);
  bcopy((char *)s->h_addr,
	(char *)&server.sin_addr.s_addr,
	s->h_length);

  char c;
  string f_contents;
  ifstream file(filename);
  file >> noskipws;
  while ( file >> c ) f_contents += c;
  file.close();

  char f_payload[f_contents.size() + 1];
	strcpy(f_payload, f_contents.c_str());
  char * spacket;
  packet packet(1, 0, 30, f_payload);
  memset(spacket, 0, sizeof(packet));
  packet.serialize(spacket);

  packet.printContents();

  if (sendto(mysocket, &packet, sizeof(packet), 0, (struct sockaddr *)&server, slen)==-1)
    cout << "Error in sendto function.\n";

  char ack[4];

  recvfrom(mysocket, ack, 4, 0, (struct sockaddr *)&server, &slen);
  cout << ack << endl;

  close(mysocket);

  return 0;
}
