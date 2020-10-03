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

  int character_len = f_contents.size();
  int length_of_payload = 0;

  int i = 0;
  int chars_remaining = character_len - i;
  string data_append;
  int seq_num = 0;

  while (1) {

    if (chars_remaining == 0)
      break;

    if (chars_remaining >= 30) {

      while (i < 30) {

        data_append += f_contents[i];
        i++;

      }

      length_of_payload = 30;

    }

    else {

      while (i < chars_remaining) {

        data_append += f_contents[i];
        i++;

      }

      length_of_payload = chars_remaining;

    }

    seq_num++;

    if (seq_num > 7)
      seq_num = 0;

  }


  char payload[512];
  vector <string> data_array;
  packet *spacket;
  char* data = (char*)f_contents.c_str();
  spacket = new packet(1,0,30,data);

  memset(payload,0,sizeof(payload));
  spacket->serialize(payload);

  data_array.push_back((string)payload);






  close(mysocket);

  return 0;
}
