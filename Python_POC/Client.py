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


def read_content(filename):
    with open(filename, "rb") as file:
        return file.read()


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

    # Send the aes keys to the server
    send_key_to_server(client)

    log("Client AES keys", CLIENT_COLOR)

    send_to(client, CLIENT_TO_SEND, "HELLO WORLD!")

    log(f"Client sent data to server. Destination is {CLIENT_TO_SEND}", CLIENT_COLOR)


def send_key_to_server(client):
    global server_public_key, server_modulus

    aes_key, aes_iv = AES.format_key_for_sending(aes_object.key), AES.format_key_for_sending(aes_object.iv)

    message = f"{aes_key}" + SPLITER + f"{aes_iv}"
    encrypted_message = str(RSA.encrypt(message, server_public_key, server_modulus))
    client.send(encrypted_message.encode())


if __name__ == '__main__':
    main()
