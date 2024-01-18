#pragma once

class StatsData {
public:
    StatsData(long long int total_bytes_sent, long long int total_bytes_received, long long int total_connections, long long int total_nodes, long long int total_clients);
    StatsData();

    long long int get_total_bytes_sent();
    long long int get_total_bytes_received();
    long long int get_total_connections();
    long long int get_total_nodes();
    long long int get_total_clients();

private:
    long long int total_bytes_sent;
    long long int total_bytes_received;
    long long int total_connections;
    long long int total_nodes;
    long long int total_clients;
};
