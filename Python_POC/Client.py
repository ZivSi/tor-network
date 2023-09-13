import socket

from Log import *
import RSA

CLIENT_DATA_FORMAT = "{}:{}"
rsa_object = RSA.RSA()
server_public_key = None
server_modulus = None


def sendTo(connection, destination, data):
    message = CLIENT_DATA_FORMAT.format(destination, data)

    encrypted_data = str(RSA.encrypt(message, server_public_key, server_modulus))

    connection.send(encrypted_data.encode())


def main(CLIENT_TO_SEND=5062):
    global rsa_object, server_public_key, server_modulus

    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(("127.0.0.1", 5060))

    log("Client connected to server", CLIENT_COLOR)

    # Receive the public key and modulus from the server
    server_public_key, server_modulus = client.recv(2048).decode().split(":")

    log("Client received public key and modulus from server", CLIENT_COLOR)

    server_public_key = int(server_public_key)
    server_modulus = int(server_modulus)

    # Send the public key and modulus to the server
    sendKeyToServer(client)

    log("Client sent public key and modulus to server", CLIENT_COLOR)

    sendTo(client, CLIENT_TO_SEND, "Hello, World!")

    log(f"Client sent data to server. Destination is {CLIENT_TO_SEND}", CLIENT_COLOR)


def sendKeyToServer(client):
    global rsa_object, server_public_key, server_modulus
    message = f"{rsa_object.publicKey}:{rsa_object.modulus}"
    encrypted_message = str(RSA.encrypt(message, server_public_key, server_modulus))
    client.send(encrypted_message.encode())


if __name__ == '__main__':
    main()
