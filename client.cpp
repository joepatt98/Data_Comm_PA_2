// Authors: Joseph Patterson and Stephanie Shisler
// NetIDs: jhp232 and sas880

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
  string file_contents;
  ifstream file(filename);
  file >> noskipws;
  while ( file >> c ) file_contents += c;
  file.close();

  int character_len = file_contents.size();
  file_contents.erase(character_len - 1);
  character_len -= 1;

  int seq_num = 0;

  cout << "Character Count: " << character_len << "\n";
  cout << "All characters: " << file_contents << "\n";

  char payload[512];
  vector <string> data_array;
  int counter = 0;

  while (1) {

    int length_of_payload = 0;
    string data_append;
    int i = 0;
    int chars_remaining = character_len - counter;

      if (chars_remaining == 0)
        break;

      if (chars_remaining >= 30) {

        while (i < 30) {

          data_append += file_contents[counter];
          i++;
          counter++;

        }

        length_of_payload = 30;

      }

      else {

        while (i < chars_remaining) {

          data_append += file_contents[counter];
          i++;
          counter++;

        }

        length_of_payload = chars_remaining;

      }

      cout << "End to counter difference: " << chars_remaining << "\n";
      cout << "Length: " << length_of_payload << "\n";
      cout << "Character count: " << character_len << "\n";
      cout << "Counter: " << counter << "\n";
      cout << "Sequence Number: " << seq_num << "\n";
      cout << "--------------------------------------------------\n\n";

      char* data = (char*)data_append.c_str();
      packet *spacket = new packet(1, seq_num, length_of_payload, data);

      memset(payload, 0, sizeof(payload));
      spacket->serialize(payload);

      data_array.push_back((string)payload);

      seq_num++;

      if (seq_num > 7)
        seq_num = 0;

  } //end while loop

  packet *eot_packet;
  int len = 0;
  char* data = NULL;
  eot_packet = new packet(3, seq_num, len, data);

  memset(payload,0,sizeof(payload));
  eot_packet->serialize(payload);

  data_array.push_back((string)payload);

  for (int i = 0; i < data_array.size(); i++) {

    memset(payload,0,sizeof(payload));
    strcpy(payload,data_array[i].c_str());

    if (sendto(mysocket, payload, sizeof(payload), 0,
      (struct sockaddr *) &server, slen) == -1) {

          cout << "Error in sendto function.\n";

    }

    packet *sn_packet = new packet(0, 0, 0, payload);
    sn_packet->deserialize(payload);

    ofstream file("clientseqnum.log", ios_base::out | ios_base::app);
    file << sn_packet->getSeqNum() << endl;

    memset(payload,0,sizeof(payload));
    int bytes_received = recvfrom(mysocket, payload, sizeof(payload), 0, (sockaddr*)&server, &slen);

    char recv_buf[512];

    if (bytes_received > 0) {

        strcpy(recv_buf, payload);

        packet *rpacket = new packet(0, 0, 0, payload);
        rpacket->deserialize(payload);

        packet *sn_packet = new packet(0, 0, 0, recv_buf);

        switch(rpacket->getType()){

            case 0: {

              sn_packet->deserialize(recv_buf);
              ofstream file("clientack.log", ios_base::out | ios_base::app);
              file << (sn_packet->getSeqNum()) << endl;
              continue;

            }

            case 2: {

              sn_packet->deserialize(recv_buf);
              ofstream file("clientack.log", ios_base::out | ios_base::app);
              file << (sn_packet->getSeqNum()) << endl;
              break;

            }
        }

    }

    else {

      cout << "Failed to receive.\n";

    }

} // end for loop

  close(mysocket);

  return 0;

}
