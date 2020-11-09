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

  char filename[512];
  strcpy(filename, argv[4]);

  struct hostent *s;
  s = gethostbyname(argv[1]);

  int mysocket = 0;

  // sets up UDP socket for file transmission
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1) {

    cout << "Error in creating UDP socket.\n";
    exit(1);

  }

  struct sockaddr_in client;
  memset((char *) &client, 0, sizeof(client));
  client.sin_family = AF_INET;
  client.sin_port = htons(atoi(argv[3]));
  bcopy((char *)s->h_addr, (char *)&client.sin_addr.s_addr, s->h_length);
  socklen_t clen = sizeof(client);
  if (bind(mysocket, (struct sockaddr *)&client, clen) == -1) {

    cout << "Error in binding.\n";
    exit(1);

  }

  struct sockaddr_in emulator;
  memset((char *) &emulator, 0, sizeof(emulator));
  emulator.sin_family = AF_INET;
  emulator.sin_port = htons(atoi(argv[2]));
  emulator.sin_addr.s_addr = htonl(INADDR_ANY);
  bcopy((char *) s->h_addr, (char *) &emulator.sin_addr.s_addr, s->h_length);
  socklen_t elen = sizeof(emulator);

  char c;
  string file_contents;
  ifstream file(filename);
  file >> noskipws;
  while ( file >> c ) file_contents += c;
  file.close();

  int seq_num = 0;
  int string_len = file_contents.size();
  file_contents.erase(string_len - 1);
  string_len -= 1;

  ofstream seqnum_log("clientseqnum.log", ios_base::out | ios_base::trunc);
  ofstream ack_log("clientack.log", ios_base::out | ios_base::trunc);

  char payload[512];
  vector <string> data_array;
  int counter = 0;

  while (1) {

      int length_of_payload = 0;
      string data_append;
      int i = 0;
      int chars_remaining = string_len - counter;

      if (chars_remaining == 0) {

        break;

      }

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

      char* data = (char*)data_append.c_str();
      packet *spacket = new packet(1, seq_num, length_of_payload, data);

      memset(payload, 0, sizeof(payload));
      spacket->serialize(payload);

      data_array.push_back(payload);

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

  data_array.push_back(payload);

  for (int i = 0; i < data_array.size(); i++) {

    memset(payload,0,sizeof(payload));
    strcpy(payload,data_array[i].c_str());

    if (sendto(mysocket, payload, sizeof(payload), 0,
      (struct sockaddr *) &emulator, elen) == -1) {

          cout << "Error in sendto function.\n";
          exit(1);

    }

    packet *sn_packet = new packet(0, 0, 0, payload);
    sn_packet->deserialize(payload);

    seqnum_log << sn_packet->getSeqNum() << endl;

    memset(payload,0,sizeof(payload));
    int bytes_received = recvfrom(mysocket, payload, sizeof(payload), 0, (sockaddr*)&client, &clen);

    char recv_buf[512];

    if (bytes_received > 0) {

        strcpy(recv_buf, payload);

        packet *rpacket = new packet(0, 0, 0, payload);
        rpacket->deserialize(payload);

        packet *sn_packet = new packet(0, 0, 0, recv_buf);

        switch(rpacket->getType()) {

            case 0: {

              sn_packet->deserialize(recv_buf);
              ack_log << (sn_packet->getSeqNum()) << endl;
              continue;

            }

            case 2: {

              sn_packet->deserialize(recv_buf);
              ack_log << (sn_packet->getSeqNum()) << endl;
              break;

            }

        }

    }

    else {

      cout << "Failed to receive.\n";
      exit(1);

    }

  } // end for loop

  seqnum_log.close();
  ack_log.close();

  close(mysocket);

  return 0;

}
