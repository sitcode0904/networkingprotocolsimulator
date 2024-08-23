#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Packet {
    int seq_num;
    int ack_num;
    bool syn;
    bool ack;
    bool fin;
    bool lost;
} Packet;

typedef struct TCPConnection {
    int seq_num;
    int ack_num;
    int cwnd;        // Congestion window size
    int ssthresh;    // Slow start threshold
    int duplicate_acks;
    char state[20];
} TCPConnection;

typedef struct UDPPacket {
    int src_port;
    int dest_port;
    Packet packet;
} UDPPacket;
typedef struct ICMPPacket {
    int type_code;
    Packet packet;
} ICMPPacket;

void three_way_handshake(TCPConnection* connection) {
    printf("Initiating three-way handshake...\n");

    // Step 1: Client sends SYN
    Packet syn_packet = {0, 0, true, false, false, false};
    printf("Client: Sending SYN (seq=%d)\n", syn_packet.seq_num);

    // Step 2: Server responds with SYN-ACK
    connection->seq_num = 1;
    Packet syn_ack_packet = {1, syn_packet.seq_num + 1, true, true, false, false};
    printf("Server: Sending SYN-ACK (seq=%d, ack=%d)\n", syn_ack_packet.seq_num, syn_ack_packet.ack_num);

    // Step 3: Client sends ACK
    connection->ack_num = syn_ack_packet.seq_num + 1;
    Packet ack_packet = {syn_packet.seq_num + 1, syn_ack_packet.seq_num + 1, false, true, false, false};
    printf("Client: Sending ACK (seq=%d, ack=%d)\n", ack_packet.seq_num, ack_packet.ack_num);

    strcpy(connection->state, "ESTABLISHED");
    printf("Connection established.\n");
}
bool send_packet(Packet* packet) {
    // Simulate a 10% chance of packet loss
    if (rand() % 10 < 1) {
        packet->lost = true;
          printf("Packet (seq=%d) lost!\n", packet->seq_num);
        return false;
    } else {
        printf("Packet (seq=%d) sent successfully.\n", packet->seq_num);
        return true;
    }
}
void send_data(TCPConnection* connection, Packet* data_packets, int num_packets) {
    for (int i = 0; i < num_packets; i++) {
        if (!send_packet(&data_packets[i])) {
            printf("Retransmitting packet (seq=%d)...\n", data_packets[i].seq_num);
            send_packet(&data_packets[i]);  // Retransmit until success
        }
    }
}
void tcp_tahoe(TCPConnection* connection, Packet* data_packets, int num_packets) {
    for (int i = 0; i < num_packets; i++) {
        if (send_packet(&data_packets[i])) {
            if (connection->cwnd < connection->ssthresh) {
                connection->cwnd += 1;  // Slow start
            } else {
                connection->cwnd += 1 / connection->cwnd;  // Congestion avoidance
            }
            connection->duplicate_acks = 0;
        } else {
            connection->ssthresh = (connection->cwnd / 2 > 1) ? connection->cwnd / 2 : 1;
            connection->cwnd = 1;  // Reset to 1 on loss (Tahoe)
            printf("Packet loss detected! ssthresh=%d, cwnd=%d\n", connection->ssthresh, connection->cwnd);
            send_packet(&data_packets[i]);  // Retransmit
             }
        printf("Current cwnd=%d\n", connection->cwnd);
    }
}

void simulate_udp() {
    printf("Simulating UDP Packet Transmission...\n");

    UDPPacket udp_packet = {1234, 80, {0, 0, false, false, false, false}};
    printf("UDP Packet: src_port=%d, dest_port=%d\n", udp_packet.src_port, udp_packet.dest_port);
    send_packet(&udp_packet.packet);
}

void simulate_icmp() {
    printf("Simulating ICMP Packet Transmission...\n");

    ICMPPacket icmp_packet = {8, {0, 0, false, false, false, false}};  // Type 8 is Echo Request
    printf("ICMP Packet: type_code=%d\n", icmp_packet.type_code);
    send_packet(&icmp_packet.packet);
}
int main() {
    srand(time(NULL));  // Seed for random packet loss

    TCPConnection tcp_connection = {0, 0, 1, 16, 0, "CLOSED"};
    three_way_handshake(&tcp_connection);

    // Create some data packets
    Packet data_packets[10];
    for (int i = 0; i < 10; i++) {
        data_packets[i] = (Packet){i + 1, 0, false, false, false, false};
    }

    // Simulate TCP Tahoe
    printf("Running TCP Tahoe...\n");
    tcp_tahoe(&tcp_connection, data_packets, 10);

    // Simulate UDP Transmission
    simulate_udp();

    // Simulate ICMP Transmission
    simulate_icmp();

    return 0;
}
