import random
import socket
import threading
from typing import Any

import RSA
from Log import SERVER_COLOR, CLIENT_COLOR, DATA_COLOR
from Python_POC.AES import *
from Utility import *

rsa_object = RSA.RSA()
RSA_KEYS_FORMAT = f"{rsa_object.publicKey}" + SPLITER + f"{rsa_object.modulus}"

clients_map = {}
# current connection port, (server_port, time, connection, aes_key, aes_iv)
nodes_map: Dict[int, Tuple[int, int, socket.socket, Any, Any]] = {}


def run_server(port: int) -> socket.socket:
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("127.0.0.1", port))

    return server


def generate_random_path(length=DEFAULT_LEN) -> List[int]:
    global nodes_map

    length = min(length, len(nodes_map.keys()))  # Choose the minimum between the length and the number of nodes

    path = []

    while len(path) < length:
        random_node = random.choice(list(nodes_map.keys()))

        if random_node not in path:
            path.append(random_node)

    return path


def encrypt(random_path: list, data):
    global nodes_map, clients_map, rsa_object
    encrypted_data = data

    reversed_path = random_path.copy()
    reversed_path.reverse()  # Start encrypting from the end of the path

    # A -> B -> C
    # encrypted = A.encrypt(B.encrypt(C.encrypt(data)))

    for node in reversed_path:
        node_aes_key = nodes_map[node][NodeIndexes.AES_KEY_INDEX]
        node_aes_iv = nodes_map[node][NodeIndexes.AES_IV_INDEX]
        node_server_port = nodes_map[node][NodeIndexes.SERVER_PORT_INDEX]

        encrypted_data = str(node_server_port) + SPLITER + str(AES.encrypt(encrypted_data, node_aes_key, node_aes_iv))

    # Remove first node's port (we don't need it since we are the one's who send it to him)
    encrypted_data = encrypted_data[encrypted_data.find(SPLITER) + len(SPLITER):]

    return encrypted_data


def send(destination_port: str, data_to_send: str):
    global nodes_map, clients_map, rsa_object

    random_path = generate_random_path()

    print_path(nodes_map, random_path)

    # print(f"Sending {data_to_send} to {destination_port} through {random_path}")
    log(f"Sending {data_to_send} to {destination_port} through {random_path}", ACTION_COLOR)

    encrypted_data = encrypt(random_path, str(destination_port) + SPLITER + data_to_send)

    first_node = random_path[0]
    node_properties = nodes_map[first_node]
    first_node_connection_server_port = node_properties[NodeIndexes.SERVER_PORT_INDEX]

    send_to_first_node(encrypted_data, first_node_connection_server_port)


def send_to_first_node(encrypted_data, first_node_connection_server_port):
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(("127.0.0.1", first_node_connection_server_port))
    client.sendall(encrypted_data.encode())
    client.close()

    log(f"Sent {encrypted_data} to {first_node_connection_server_port}", ACTION_COLOR)


def terminate_node(nodes_map, port):
    nodes_map[port][NodeIndexes.CONNECTION_INDEX].close()
    del nodes_map[port]


def handle_connection(connection: socket.socket, node: bool, port: int):
    global nodes_map, clients_map, rsa_object

    """
    if node:
        check_alive_in_thread(port)
    """

    aes_key = nodes_map[port][NodeIndexes.AES_KEY_INDEX] if node else clients_map[port][ClientIndexes.AES_KEY_INDEX]
    aes_iv = nodes_map[port][NodeIndexes.AES_IV_INDEX] if node else clients_map[port][ClientIndexes.AES_IV_INDEX]

    while True:
        try:
            received = connection.recv(RECEIVE_SIZE).decode()
            try:
                decrypted = decrypt(received, aes_key)
            except:
                if len(received) == 0: continue
                log(f"Couldn't decrypt {received}", ERROR_COLOR)
                continue

            if node:
                server_port, aes_key, aes_iv = extract_connection_data(decrypted)
                server_port = int(server_port)

                aes_key, aes_iv = format_keys(aes_key, aes_iv)

                current_time_milliseconds = int(time.time() * 1000)

                nodes_map[port] = (server_port, current_time_milliseconds, connection, aes_key, aes_iv)
            elif decrypted.strip() != "":
                destination_port, data_to_send = extract_connection_data(decrypted)
                log(f"Extracted {destination_port} and {data_to_send}", DATA_COLOR)

                send(destination_port, data_to_send)

        except ConnectionResetError as e:
            log(f"{port} disconnected", ERROR_COLOR)
            if node: terminate_node(nodes_map, port)

            return

        except Exception as e:
            raise e


def listen(server: socket.socket):
    global nodes_map, clients_map, rsa_object

    log("Listening...", SERVER_COLOR)
    server.listen()

    while True:
        connection, address = server.accept()
        _, client_port = address

        send_rsa_keys(connection)

        try:
            response = connection.recv(2048).decode()
            try:
                decrypted_response = rsa_object.decrypt(response)  # Only first message in RSA
            except TypeError:  # (The client sent the data as "[1, 2, 3]" and we turn it to [1, 2, 3])
                evaluated_list = eval_list(response)
                decrypted_response = rsa_object.decrypt(evaluated_list)
        except:
            log(f"Couldn't receive initial data from {address}", ERROR_COLOR)
            connection.close()

            continue

        try:
            if is_node(decrypted_response):
                log(f"Node connected: {client_port}", SERVER_COLOR)

                server_port, aes_key, aes_iv = extract_connection_data(decrypted_response)
                server_port = int(server_port)

                aes_key, aes_iv = format_keys(aes_key, aes_iv)

                current_time_milliseconds = int(time.time() * 1000)

                nodes_map[client_port] = (server_port, current_time_milliseconds, connection, aes_key, aes_iv)
            else:
                log(f"Client connected: {client_port}", CLIENT_COLOR)

                aes_key, aes_iv = extract_connection_data(decrypted_response)

                aes_key, aes_iv = format_keys(aes_key, aes_iv)

                current_time_milliseconds = int(time.time() * 1000)

                clients_map[client_port] = (connection, current_time_milliseconds, aes_key, aes_iv)
        except (ValueError, UnicodeEncodeError) as e:
            log(f"Couldn't receive initial data from the connection ({e})", ERROR_COLOR)
            continue

        my_thread = threading.Thread(target=handle_connection,
                                     args=(connection, is_node(decrypted_response), client_port))
        my_thread.daemon = True
        my_thread.start()


def send_rsa_keys(client: socket.socket):
    client.sendall(RSA_KEYS_FORMAT.encode())


def main():
    server_socket = run_server(SERVER_PORT)
    log("Server is running", SERVER_COLOR)
    listen(server_socket)


if __name__ == '__main__':
    main()
