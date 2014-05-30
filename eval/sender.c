/*  Copyright (C) 2011-2013  P.D. Buchan (pdbuchan@yahoo.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Send an IPv4 UDP packet via raw socket.
// Stack fills out layer 2 (data link) information (MAC addresses) for us.
// Includes some UDP data.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>           // close()
#include <string.h>           // strcpy, memset(), and memcpy()

#include <netdb.h>            // struct addrinfo
#include <sys/types.h>        // needed for socket(), uint8_t, uint16_t, uint32_t
#include <sys/socket.h>       // needed for socket()
#include <netinet/in.h>       // IPPROTO_RAW, IPPROTO_IP, IPPROTO_UDP, INET_ADDRSTRLEN
#include <netinet/ip.h>       // struct ip and IP_MAXPACKET (which is 65535)
#include <netinet/udp.h>      // struct udphdr
#include <arpa/inet.h>        // inet_pton() and inet_ntop()
#include <sys/ioctl.h>        // macro ioctl is defined
#include <bits/ioctls.h>      // defines values for argument "request" of ioctl.
#include <net/if.h>           // struct ifreq

#include <errno.h>            // errno, perror()
#include <inttypes.h>

// Define some constants.
#define IP4_HDRLEN 20         // IPv4 header length
#define UDP_HDRLEN  8         // UDP header length, excludes data

// Function prototypes
uint16_t checksum (uint16_t *, int);
uint16_t udp4_checksum (struct ip, struct udphdr, uint8_t *, int);
char *allocate_strmem (int);
uint8_t *allocate_ustrmem (int);
int *allocate_intmem (int);


void print_usage_and_exit(char* prog_name) {
  printf("\nUsage: %s <TRACE FILE> <LOOP yes|no>\n\n", prog_name);
  exit(1);
}


typedef struct Input_t {
  size_t num_packets;
  uint32_t* src_ips;
  uint32_t dst_ip;
  uint16_t* src_ports;
  uint16_t* dst_ports;
} Input;


Input* alloc_input(size_t num) {
  Input* input = (Input*) malloc(sizeof(Input));
  input->num_packets = num;
  input->src_ips = (uint32_t*) malloc(num * sizeof(uint32_t));
  input->src_ports = (uint16_t*) malloc(num * sizeof(uint16_t));
  input->dst_ports = (uint16_t*) malloc(num * sizeof(uint16_t));
  return input;
}


void free_input(Input* input) {
  free(input->src_ips);
  free(input->src_ports);
  free(input->dst_ports);
  free(input);
}


Input* read_trace_file(char* filename) {
  FILE* file = fopen(filename, "r");
  char buf[512];
  size_t num_lines = 0;
  while (fgets(buf, 511, file) != NULL)
    ++num_lines;
  rewind(file);
  Input* input = alloc_input(num_lines);
  uint32_t src_ip, dst_ip;
  uint16_t src_port, dst_port;
  size_t i;
  for (i = 0; i < num_lines; ++i) {
    char* ret = fgets(buf, 511, file);
    sscanf(buf, "%"SCNu32"\t%"SCNu32"\t%"SCNu16"\t%"SCNu16"\t",
        &src_ip, &dst_ip,
        &src_port, &dst_port);

    input->src_ips[i] = htonl(src_ip);
    input->dst_ip = htonl(dst_ip);
    input->src_ports[i] = htons(src_port);
    input->dst_ports[i] = htons(dst_port);
  }
  fclose(file);
  return input;
}


int
main (int argc, char **argv)
{

  /* Handle input data */
  if (argc != 3)
    print_usage_and_exit(argv[0]);
  Input* input = read_trace_file(argv[1]);

  int loop_forever;
  if (strcmp("yes", argv[2]) == 0)
    loop_forever = 1;
  else if (strcmp("no", argv[2]) == 0)
    loop_forever = 0;
  else
    print_usage_and_exit(argv[0]);
  //const uint16_t dst_port = atoi(argv[2]);

  // set random seed
  //srand(atoi(argv[4]));

  int status, datalen, sd, *ip_flags;
  const int on = 1;
  char *interface, *target, *src_ip, *dst_ip;
  struct ip iphdr;
  struct udphdr udphdr;
  uint8_t *data, *packet;
  struct addrinfo hints, *res;
  struct sockaddr_in *ipv4, sin;
  struct ifreq ifr;
  void *tmp;

  // Allocate memory for various arrays.
  data = allocate_ustrmem (IP_MAXPACKET);
  packet = allocate_ustrmem (IP_MAXPACKET);
  interface = allocate_strmem (40);
  target = allocate_strmem (40);
  src_ip = allocate_strmem (INET_ADDRSTRLEN);
  dst_ip = allocate_strmem (INET_ADDRSTRLEN);
  ip_flags = allocate_intmem (4);

  // Interface to send packet through.
  strcpy (interface, "eth0");

  // Submit request for a socket descriptor to look up interface.
  if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
    perror ("socket() failed to get socket descriptor for using ioctl() ");
    exit (EXIT_FAILURE);
  }

  // Use ioctl() to look up interface index which we will use to
  // bind socket descriptor sd to specified interface with setsockopt() since
  // none of the other arguments of sendto() specify which interface to use.
  memset (&ifr, 0, sizeof (ifr));
  snprintf (ifr.ifr_name, sizeof (ifr.ifr_name), "%s", interface);
  if (ioctl (sd, SIOCGIFINDEX, &ifr) < 0) {
    perror ("ioctl() failed to find interface ");
    return (EXIT_FAILURE);
  }
  close (sd);
  //printf ("Index for interface %s is %i\n", interface, ifr.ifr_ifindex);

  // Submit request for a raw socket descriptor.
  if ((sd = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0) {
    perror ("socket() failed ");
    exit (EXIT_FAILURE);
  }

  if (setsockopt (sd, IPPROTO_IP, IP_HDRINCL, &on, sizeof (on)) < 0) {
    perror ("setsockopt() failed to set IP_HDRINCL ");
    exit (EXIT_FAILURE);
  }
  // Bind socket to interface index.
  if (setsockopt (sd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof (ifr)) < 0) {
    perror ("setsockopt() failed to bind to interface ");
    exit (EXIT_FAILURE);
  }

  printf("input->num_packets = %zu\n", input->num_packets);
  size_t packet_count;
  for (packet_count = 0; packet_count < input->num_packets; ++packet_count) {
    //strcpy (dst_ip, argv[1]);

    // UDP data
    datalen = 0;
    //data[0] = 'T';
    //data[1] = 'e';
    //data[2] = 's';
    //data[3] = 't';

    // IPv4 header

    // IPv4 header length (4 bits): Number of 32-bit words in header = 5
    iphdr.ip_hl = IP4_HDRLEN / sizeof (uint32_t);

    // Internet Protocol version (4 bits): IPv4
    iphdr.ip_v = 4;

    // Type of service (8 bits)
    iphdr.ip_tos = 0;

    // Total length of datagram (16 bits): IP header + UDP header + datalen
    iphdr.ip_len = htons (IP4_HDRLEN + UDP_HDRLEN + datalen);

    // ID sequence number (16 bits): unused, since single datagram
    iphdr.ip_id = htons (0);

    // Flags, and Fragmentation offset (3, 13 bits): 0 since single datagram

    // Zero (1 bit)
    ip_flags[0] = 0;

    // Do not fragment flag (1 bit)
    ip_flags[1] = 0;

    // More fragments following flag (1 bit)
    ip_flags[2] = 0;

    // Fragmentation offset (13 bits)
    ip_flags[3] = 0;

    iphdr.ip_off = htons ((ip_flags[0] << 15)
                        + (ip_flags[1] << 14)
                        + (ip_flags[2] << 13)
                        +  ip_flags[3]);

    // Time-to-Live (8 bits): default to maximum value
    iphdr.ip_ttl = 255;

    // Transport layer protocol (8 bits): 17 for UDP
    iphdr.ip_p = IPPROTO_UDP;

    // Source IPv4 address (32 bits)
    //iphdr.ip_src.s_addr = rand() | (rand() << 16);
    iphdr.ip_src.s_addr = input->src_ips[packet_count];
    //inet_aton("8.8.9.255", &(iphdr.ip_src));

    // Destination IPv4 address (32 bits)
    //inet_aton(dst_ip, &(iphdr.ip_dst));
    iphdr.ip_dst.s_addr = input->dst_ip;

    // IPv4 header checksum (16 bits): set to 0 when calculating checksum
    iphdr.ip_sum = 0;
    iphdr.ip_sum = checksum ((uint16_t *) &iphdr, IP4_HDRLEN);

    // UDP header

    // Source port number (16 bits): pick a number
    //udphdr.source = htons (6000);
    //udphdr.source = htons (rand() % 65536);
    udphdr.source = input->src_ports[packet_count];

    // Destination port number (16 bits): pick a number
    //udphdr.dest = htons (dst_port);
    udphdr.dest = input->dst_ports[packet_count];

    // Length of UDP datagram (16 bits): UDP header + UDP data
    udphdr.len = htons (UDP_HDRLEN + datalen);

    // UDP checksum (16 bits)
    //udphdr.check = udp4_checksum (iphdr, udphdr, data, datalen);
    udphdr.check = 0;

    // Prepare packet.

    // First part is an IPv4 header.
    memcpy (packet, &iphdr, IP4_HDRLEN * sizeof (uint8_t));

    // Next part of packet is upper layer protocol header.
    memcpy ((packet + IP4_HDRLEN), &udphdr, UDP_HDRLEN * sizeof (uint8_t));

    // Finally, add the UDP data.
    memcpy (packet + IP4_HDRLEN + UDP_HDRLEN, data, datalen * sizeof (uint8_t));

    // The kernel is going to prepare layer 2 information (ethernet frame header) for us.
    // For that, we need to specify a destination for the kernel in order for it
    // to decide where to send the raw datagram. We fill in a struct in_addr with
    // the desired destination IP address, and pass this structure to the sendto() function.
    memset (&sin, 0, sizeof (struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = iphdr.ip_dst.s_addr;

    // Send packet.
    if (sendto (sd, packet, IP4_HDRLEN + UDP_HDRLEN + datalen, 0, (struct sockaddr *) &sin, sizeof (struct sockaddr)) < 0)  {
      perror ("sendto() failed ");
      exit (EXIT_FAILURE);
    }
  }
  // Close socket descriptor.
  close (sd);

  // Free allocated memory.
  free (data);
  free (packet);
  free (interface);
  free (target);
  free (src_ip);
  free (dst_ip);
  free (ip_flags);
  free_input(input);

  return (EXIT_SUCCESS);
}

// Checksum function
uint16_t
checksum (uint16_t *addr, int len)
{
  int nleft = len;
  int sum = 0;
  uint16_t *w = addr;
  uint16_t answer = 0;

  while (nleft > 1) {
    sum += *w++;
    nleft -= sizeof (uint16_t);
  }

  if (nleft == 1) {
    *(uint8_t *) (&answer) = *(uint8_t *) w;
    sum += answer;
  }

  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

// Build IPv4 UDP pseudo-header and call checksum function.
uint16_t
udp4_checksum (struct ip iphdr, struct udphdr udphdr, uint8_t *payload, int payloadlen)
{
  char buf[IP_MAXPACKET];
  char *ptr;
  int chksumlen = 0;
  int i;

  ptr = &buf[0];  // ptr points to beginning of buffer buf

  // Copy source IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_src.s_addr, sizeof (iphdr.ip_src.s_addr));
  ptr += sizeof (iphdr.ip_src.s_addr);
  chksumlen += sizeof (iphdr.ip_src.s_addr);

  // Copy destination IP address into buf (32 bits)
  memcpy (ptr, &iphdr.ip_dst.s_addr, sizeof (iphdr.ip_dst.s_addr));
  ptr += sizeof (iphdr.ip_dst.s_addr);
  chksumlen += sizeof (iphdr.ip_dst.s_addr);

  // Copy zero field to buf (8 bits)
  *ptr = 0; ptr++;
  chksumlen += 1;

  // Copy transport layer protocol to buf (8 bits)
  memcpy (ptr, &iphdr.ip_p, sizeof (iphdr.ip_p));
  ptr += sizeof (iphdr.ip_p);
  chksumlen += sizeof (iphdr.ip_p);

  // Copy UDP length to buf (16 bits)
  memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
  ptr += sizeof (udphdr.len);
  chksumlen += sizeof (udphdr.len);

  // Copy UDP source port to buf (16 bits)
  memcpy (ptr, &udphdr.source, sizeof (udphdr.source));
  ptr += sizeof (udphdr.source);
  chksumlen += sizeof (udphdr.source);

  // Copy UDP destination port to buf (16 bits)
  memcpy (ptr, &udphdr.dest, sizeof (udphdr.dest));
  ptr += sizeof (udphdr.dest);
  chksumlen += sizeof (udphdr.dest);

  // Copy UDP length again to buf (16 bits)
  memcpy (ptr, &udphdr.len, sizeof (udphdr.len));
  ptr += sizeof (udphdr.len);
  chksumlen += sizeof (udphdr.len);

  // Copy UDP checksum to buf (16 bits)
  // Zero, since we don't know it yet
  *ptr = 0; ptr++;
  *ptr = 0; ptr++;
  chksumlen += 2;

  // Copy payload to buf
  memcpy (ptr, payload, payloadlen);
  ptr += payloadlen;
  chksumlen += payloadlen;

  // Pad to the next 16-bit boundary
  for (i=0; i<payloadlen%2; i++, ptr++) {
    *ptr = 0;
    ptr++;
    chksumlen++;
  }

  return checksum ((uint16_t *) buf, chksumlen);
}

// Allocate memory for an array of chars.
char *
allocate_strmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_strmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (char *) malloc (len * sizeof (char));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (char));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_strmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of unsigned chars.
uint8_t *
allocate_ustrmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_ustrmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (uint8_t *) malloc (len * sizeof (uint8_t));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (uint8_t));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_ustrmem().\n");
    exit (EXIT_FAILURE);
  }
}

// Allocate memory for an array of ints.
int *
allocate_intmem (int len)
{
  void *tmp;

  if (len <= 0) {
    fprintf (stderr, "ERROR: Cannot allocate memory because len = %i in allocate_intmem().\n", len);
    exit (EXIT_FAILURE);
  }

  tmp = (int *) malloc (len * sizeof (int));
  if (tmp != NULL) {
    memset (tmp, 0, len * sizeof (int));
    return (tmp);
  } else {
    fprintf (stderr, "ERROR: Cannot allocate memory for array allocate_intmem().\n");
    exit (EXIT_FAILURE);
  }
}
