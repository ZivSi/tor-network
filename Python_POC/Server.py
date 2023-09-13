import ast
import random
import socket
import threading
import time
from typing import *

from Log import *
import RSA


class NodeIndexes:
    SERVER_PORT_INDEX = 0
    TIME_INDEX = 1
    CONNECTION_INDEX = 2
    PUBLIC_KEY_INDEX = 3
    MODULUS_INDEX = 4


clients_map = {}
nodes_map: Dict[int, Tuple[int, int, socket.socket, int, int]] = {}

ALIVE_TIME = 10


class ClientIndexes:
    CONNECTION_INDEX = 0
    PUBLIC_KEY_INDEX = 1
    MODULUS_INDEX = 2


rsa_object = RSA.RSA()
RSA_KEY = f"{rsa_object.publicKey}:{rsa_object.modulus}"


def create_server(port: int):
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(("127.0.0.1", port))

    log(f"Running on {port}", SERVER_COLOR)

    return server


def is_node(response) -> bool:
    return len(response.split(":")) == 3


def extract_node_data(response) -> Tuple[int, int, int]:
    server_port, public_key, modulus = response.split(":")

    server_port = int(server_port)
    public_key = int(public_key)
    modulus = int(modulus)

    return server_port, public_key, modulus


def extract_client_data(response, first_message=True):
    if not first_message:
        return response.split(":")

    public_key, modulus = response.split(":")

    public_key = int(public_key)
    modulus = int(modulus)

    return public_key, modulus


def check_alive(port: int):
    global nodes_map, clients_map

    while port in nodes_map.keys():
        time.sleep(ALIVE_TIME)

        current_time_milliseconds = int(time.time() * 1000)

        node_last_time = nodes_map[port][NodeIndexes.TIME_INDEX]

        log(f"Checking {port}...", ACTION_COLOR)

        if current_time_milliseconds - node_last_time > 10000:
            log(f"{port} is dead", ERROR_COLOR)

            terminate_node(nodes_map, port)

            break


def terminate_node(nodes_map, port):
    nodes_map[port][NodeIndexes.CONNECTION_INDEX].close()
    del nodes_map[port]


def generate_random_path(length=2) -> List[int]:
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
        node_public_key = nodes_map[node][NodeIndexes.PUBLIC_KEY_INDEX]
        node_modulus = nodes_map[node][NodeIndexes.MODULUS_INDEX]
        node_server_port = nodes_map[node][NodeIndexes.SERVER_PORT_INDEX]

        encrypted_data = str(node_server_port) + ":" + str(RSA.encrypt(encrypted_data, node_public_key, node_modulus))

    encrypted_data = encrypted_data[encrypted_data.find(":") + 1:]

    return encrypted_data


def send(destination_port, data_to_send):
    global nodes_map, clients_map, rsa_object

    random_path = generate_random_path()

    print_path(random_path)

    # print(f"Sending {data_to_send} to {destination_port} through {random_path}")
    log(f"Sending {data_to_send} to {destination_port} through {random_path}", ACTION_COLOR)

    encrypted_data = encrypt(random_path, str(destination_port) + ":" + data_to_send)

    first_node = random_path[0]
    node_properties = nodes_map[first_node]
    first_node_connection_server_port = node_properties[NodeIndexes.SERVER_PORT_INDEX]

    send_to_first_node(encrypted_data, first_node_connection_server_port)


def print_path(random_path):
    # print the path (by the order of the nodes) in a nice way
    path = ""

    for node in random_path:
        path += str(nodes_map[node][NodeIndexes.SERVER_PORT_INDEX]) + " -> "

    path = path[:-4]

    log(f"Path: {path}", ACTION_COLOR)


def send_to_first_node(encrypted_data, first_node_connection_server_port):
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(("127.0.0.1", first_node_connection_server_port))
    client.sendall(encrypted_data.encode())
    client.close()

    log(f"Sent {encrypted_data} to {first_node_connection_server_port}", ACTION_COLOR)


def handle_connection(connection: socket.socket, node: bool, port: int):
    global nodes_map, clients_map, rsa_object

    if node:
        check_alive_in_thread(port)

    try:

        while True:
            received = connection.recv(2048).decode()
            try:
                received = decrypt_response(received, rsa_object)
            except SyntaxError:
                continue

            if node:
                server_port, public_key, modulus = extract_node_data(received)

                current_time_milliseconds = int(time.time() * 1000)

                nodes_map[port] = (server_port, current_time_milliseconds, connection, public_key, modulus)
            elif received.strip() != "":
                destination_port, data_to_send = extract_client_data(received, False)
                log(f"Extracted {destination_port} and {data_to_send}", DATA_COLOR)

                send(destination_port, data_to_send)

    except ConnectionResetError as e:
        log(f"{port} disconnected", ERROR_COLOR)
        terminate_node(nodes_map, port)

    except Exception as e:
        raise e


def check_alive_in_thread(port):
    check_alive_thread = threading.Thread(target=check_alive, args=(port,))
    check_alive_thread.daemon = True
    check_alive_thread.start()


def decrypt_response(received, rsa_object):
    try:
        return rsa_object.decrypt(ast.literal_eval(received))
    except (SyntaxError, TypeError, ValueError):
        # log(f"Tried to decrypt {received} but failed", ERROR_COLOR)
        return ""


def listen(server: socket.socket):
    global nodes_map, clients_map, rsa_object

    log("Listening...", SERVER_COLOR)
    server.listen()

    while True:
        connection, address = server.accept()
        _, client_port = address

        send_public_key(connection)

        try:
            response = connection.recv(1024).decode()

            try:
                decrypted = decrypt_response(response, rsa_object)
            except SyntaxError:
                continue

            log("Received: " + response, DATA_COLOR)
            log("Decrypted: " + decrypted, DATA_COLOR)

        except ConnectionResetError:
            print("Connection ended")
            return

        if is_node(decrypted):
            log(f"Node connected: {client_port}", SERVER_COLOR)

            server_port, public_key, modulus = extract_node_data(decrypted)

            current_time_milliseconds = int(time.time() * 1000)

            nodes_map[client_port] = (server_port, current_time_milliseconds, connection, public_key, modulus)
        else:
            log(f"Client connected: {client_port}", CLIENT_COLOR)

            public_key, modulus = extract_client_data(decrypted)

            clients_map[client_port] = (connection, public_key, modulus)

        # Start receiving
        my_thread = threading.Thread(target=handle_connection, args=(connection, is_node(decrypted), client_port))
        my_thread.daemon = True
        my_thread.start()


def send_public_key(connection):
    connection.sendall(RSA_KEY.encode())


def main():
    server = create_server(5060)
    listen(server)


if __name__ == '__main__':
    main()
