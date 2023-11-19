import socket

import RSA
from Log import *
from Python_POC import AES
from Python_POC.Server import SPLITER

CLIENT_DATA_FORMAT = "{}" + SPLITER + "{}"
aes_object = AES.AES()
server_public_key = None
server_modulus = None


def send_to(connection, destination, data):
    message = CLIENT_DATA_FORMAT.format(destination, data)

    encrypted_data = str(aes_object.encrypt(message))

    connection.sendall(encrypted_data.encode())


def main(CLIENT_TO_SEND=5062):
    global server_public_key, server_modulus

    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(("127.0.0.1", 5060))

    log("Client connected to server", CLIENT_COLOR)

    # Receive the public key and modulus from the server
    server_public_key, server_modulus = client.recv(2048).decode().split(SPLITER)

    log("Client received public key and modulus from server", CLIENT_COLOR)

    server_public_key = int(server_public_key)
    server_modulus = int(server_modulus)

    # Send the public key and modulus to the server
    send_key_to_server(client)

    log("Client sent public key and modulus to server", CLIENT_COLOR)

    send_to(client, CLIENT_TO_SEND, "Hello, World!")

    log(f"Client sent data to server. Destination is {CLIENT_TO_SEND}", CLIENT_COLOR)


def send_key_to_server(client):
    global server_public_key, server_modulus
    message = f"{aes_object.key}" + SPLITER + f"{aes_object.iv}"
    encrypted_message = str(RSA.encrypt(message, server_public_key, server_modulus))
    client.send(encrypted_message.encode())


if __name__ == '__main__':
    main()
