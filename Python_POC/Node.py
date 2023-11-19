import socket
import threading
import time
from typing import Tuple

import RSA
from Log import *
from Python_POC import AES
from Server import SPLITER, SERVER_PORT

ALIVE_FORMAT = "{}" + SPLITER + "{}" + SPLITER + "{}"

rsa_object = RSA.RSA()
aes_object = AES.AES()

server: socket.socket  # The server that the node is the host
parent_server: socket.socket = None  # The server the node connects to
parent_public_key: int = None
parent_modulus: int = None

my_server_port = -1

first_message = True
received_parent_keys = False


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
    global received_parent_keys

    keys = parent.recv(2048).decode().split(SPLITER)
    public_key = int(keys[0])
    modulus = int(keys[1])

    received_parent_keys = True

    return public_key, modulus


def send_alive():
    global parent_server, my_server_port, first_message

    aes_key, aes_iv = AES.format_key_for_sending(aes_object.key), AES.format_key_for_sending(aes_object.iv)

    alive_message = ALIVE_FORMAT.format(my_server_port, aes_key, aes_iv)

    while not received_parent_keys:
        time.sleep(0.01)

    while True:
        if first_message:
            alive_message_encrypted = str(RSA.encrypt(alive_message, parent_public_key, parent_modulus))
            first_message = False
        else:
            alive_message_encrypted = str(AES.encrypt(alive_message, aes_object.key, aes_object.iv))

        if my_server_port != -1:
            parent_server.sendall(alive_message_encrypted.encode())
        else:
            log("Node's server not established yet", ERROR_COLOR)

        time.sleep(2)


def extract_data(decrypted):
    return decrypted.split(SPLITER)


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
            decrypted = aes_object.decrypt(received)
        except SyntaxError:
            continue

        if reached_destination(decrypted):
            log(f"NODE {my_server_port} Data reached destination: {received}", ACTION_COLOR)

            continue

        destination_port, data_to_send = extract_data(decrypted)

        log(f"NODE {my_server_port} received {data_to_send} to send to {destination_port}", DATA_COLOR)

        send_to_node(data_to_send, int(destination_port))


def start_with_port(decrypted):
    splitter_index = decrypted.find(SPLITER)
    try:
        port = int(decrypted[:splitter_index])  # Try to cast the port to int
        return True
    except ValueError:
        return False


def reached_destination(decrypted: str) -> bool:
    return not start_with_port(decrypted)


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
