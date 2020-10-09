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

  stringstream ss;
  ss << argList[1];
  int n_port;
  ss >> n_port;

  char filename[512];
  strcpy(filename, argList[2].c_str());

  struct sockaddr_in server;
  struct sockaddr_in client;

  int mysocket = 0;
  socklen_t clen = sizeof(client);
  socklen_t slen = sizeof(server);

  // sets up UDP socket for file transmission
  if ((mysocket=socket(AF_INET, SOCK_DGRAM, 0))==-1)
    cout << "Error in creating UDP socket.\n";

  memset((char *) &server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(n_port);
  server.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(mysocket, (struct sockaddr *)&server, sizeof(server)) == -1)
    cout << "Error in binding.\n";

  ofstream arrival_log("arrival.log", ios_base::out | ios_base::trunc);
  arrival_log.close();

  char payload[512];
  string f_payload = "";
  int fileout;
  bool isDone = true;
  while (isDone) {

      memset(payload,0,sizeof(payload));

      int bytes_received = recvfrom(mysocket, payload, sizeof(payload), 0,
        (struct sockaddr *) &client, &clen);

      if (bytes_received > 0) {

          packet *rcv_packet = new packet(0, 0 , 30, payload);
          rcv_packet->deserialize(payload);

          ofstream arrival_log("arrival.log", ios_base::out | ios_base::app);
          arrival_log << rcv_packet->getSeqNum() << endl;

          switch (rcv_packet->getType()) {

              case 1: {

                f_payload += rcv_packet->getData();

                packet *spacket;
                spacket = new packet(0,rcv_packet->getSeqNum(),0,NULL);

                // Serialize the ACK packet
                memset(payload,0,sizeof(payload));
                spacket->serialize(payload);

                // Send ACK packet to emulator from server
                //elen = sizeof(emulator);
                fileout = sendto(mysocket, payload, sizeof(payload), 0, (struct sockaddr *)&client, clen);
                if (fileout < 0) {cout << "Could not send to server\n";
                exit(1);
                }
                //isDone = false;
                break;

              }

              case 3: {

                packet *spacket;
                spacket = new packet(2,rcv_packet->getSeqNum(),0,NULL);

                // Serialize the EOT packet
                memset(payload,0,sizeof(payload));
                spacket->serialize(payload);

                // Send EOT packet to emulator from server
                //elen = sizeof(emulator);
                fileout = sendto(mysocket, payload, sizeof(payload), 0, (struct sockaddr *)&client, clen);
                if (fileout < 0) {
                    cout << "Could not send to server\n";
                    exit(1);
                }
                isDone = false;
                break;

              }

              default: {

                  cout << "Error Occurred\n";
                  isDone = false;
                  break;

              }
              isDone = false;
              break;

          }
          /*
          if (rcv_packet->getType() == 1) {

              f_payload += rcv_packet->getData();

              packet *spacket = new packet(0,rcv_packet->getSeqNum(),0,NULL);

              memset(payload,0,sizeof(payload));
              spacket->serialize(payload);

              if (sendto(mysocket, payload, sizeof(payload), 0,
                (struct sockaddr *) &client, clen) == -1) {

                    cout << "Error in sendto function.\n";

              }

              break;

          }

          if (rcv_packet->getType() == 3) {

              packet *spacket = new packet(2,rcv_packet->getSeqNum(),0,NULL);

              memset(payload,0,sizeof(payload));
              spacket->serialize(payload);

              if (sendto(mysocket, payload, sizeof(payload), 0,
                (struct sockaddr *) &client, clen) == -1) {

                    cout << "Error in sendto function.\n";

              }

              break;

          }*/

      }

      else {

          cout << "Failed to receive.\n";

      }

  } // end while loop

  ofstream file(filename);
  file << f_payload;
  file.close();

  close(mysocket);

  return 0;

}
