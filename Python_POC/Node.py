import socket
import threading
import time
from typing import Tuple

import RSA
from Log import *
from Server import decrypt_response

ALIVE_FORMAT = "{}:{}:{}"
SERVER_PORT = 5060

rsa_object = RSA.RSA()
server: socket.socket  # The server that the node is the host
parent_server: socket.socket = None  # The server the node connects to
parent_public_key: int = None
parent_modulus: int = None

my_server_port = -1


def run_server(port) -> Tuple[socket.socket, int]:
    try:

        server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        server.bind(("127.0.0.1", port))

        log(f"Running on {port}", SERVER_COLOR)

        return server, port
    except OSError:
        log(f"Port {port} is taken", ERROR_COLOR)
        return run_server(port + 1)


def connect_to(port: int):
    global parent_public_key, parent_modulus
    parent = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    parent.connect(("127.0.0.1", port))

    # Receive the public key and modulus of the parent
    parent_public_key, parent_modulus = receive_parent_key(parent)

    return parent


def receive_parent_key(parent) -> Tuple[int, int]:
    keys = parent.recv(2048).decode().split(":")
    public_key = int(keys[0])
    modulus = int(keys[1])

    return public_key, modulus


def send_alive():
    global parent_server, my_server_port

    public_key, modulus = rsa_object.publicKey, rsa_object.modulus

    alive_message = ALIVE_FORMAT.format(my_server_port, public_key, modulus)
    alive_message_encrypted = str(RSA.encrypt(alive_message, parent_public_key, parent_modulus))

    while True:
        time.sleep(2)
        if my_server_port != -1:
            parent_server.sendall(alive_message_encrypted.encode())
        else:
            log("Not connected to a server", ERROR_COLOR)


def extract_data(decrypted):
    return decrypted.split(":")


def receive():
    """
    Wait for other nodes to connect to this node (or the parent - meaning this is the first node in the path)
    """

    global server, rsa_object

    server.listen()

    while True:
        connection, address = server.accept()

        received = connection.recv(2048).decode()
        try:
            decrypted = decrypt_response(received, rsa_object)
        except SyntaxError:
            continue

        if reached_destination(decrypted):
            log(f"NODE {my_server_port} Data reached destination: {received}", ACTION_COLOR)

            continue

        destination_port, data_to_send = extract_data(decrypted)

        log(f"NODE {my_server_port} received {data_to_send} to send to {destination_port}", DATA_COLOR)

        send_to_node(data_to_send, int(destination_port))


def reached_destination(decrypted) -> bool:
    return decrypted.strip() == ""


def send_to_node(data_to_send, destination_port):
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(("127.0.0.1", destination_port))
    client.sendall(data_to_send.encode())
    client.close()


def main():
    global server, parent_server, rsa_object, my_server_port

    server, my_server_port = run_server(SERVER_PORT + 1)

    receive_thread = threading.Thread(target=receive)
    receive_thread.daemon = True
    receive_thread.start()

    parent_server = connect_to(SERVER_PORT)
    log("Connected to parent", ACTION_COLOR)

    send_alive_thread = threading.Thread(target=send_alive)
    send_alive_thread.daemon = True
    send_alive_thread.start()

    time.sleep(10000)


if __name__ == '__main__':
    main()
