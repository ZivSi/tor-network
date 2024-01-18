#include "StatsData.h"

StatsData::StatsData(long long int total_bytes_sent, long long int total_bytes_received, long long int total_connections, long long int total_nodes, long long int total_clients) {
    this->total_bytes_sent = total_bytes_sent;
    this->total_bytes_received = total_bytes_received;
    this->total_connections = total_connections;
    this->total_nodes = total_nodes;
    this->total_clients = total_clients;
}

StatsData::StatsData() {
    this->total_bytes_sent = 0;
    this->total_bytes_received = 0;
    this->total_connections = 0;
    this->total_nodes = 0;
    this->total_clients = 0;
}

long long int StatsData::get_total_bytes_sent() {
    return this->total_bytes_sent;
}

long long int StatsData::get_total_bytes_received() {
    return this->total_bytes_received;
}

long long int StatsData::get_total_connections() {
    return this->total_connections;
}

long long int StatsData::get_total_nodes() {
    return this->total_nodes;
}

long long int StatsData::get_total_clients() {
    return this->total_clients;
}
