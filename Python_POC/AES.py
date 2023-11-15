import base64
import os

from cryptography.hazmat.primitives import padding as pad
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes


class AES:
    def __init__(self, key=os.urandom(16), iv=os.urandom(16)):
        self.key = key
        self.iv = iv  # Generate a random IV during initialization

    def encrypt(self, plaintext):
        cipher = Cipher(algorithms.AES(self.key), modes.CFB(self.iv))
        encryptor = cipher.encryptor()
        padder = pad.PKCS7(128).padder()
        padded_data = padder.update(plaintext.encode()) + padder.finalize()
        ciphertext = encryptor.update(padded_data) + encryptor.finalize()
        return base64.b64encode(self.iv + ciphertext).decode()

    def decrypt(self, ciphertext):
        ciphertext = base64.b64decode(ciphertext.encode())
        iv = ciphertext[:16]  # Extract the IV from the ciphertext
        ciphertext = ciphertext[16:]
        cipher = Cipher(algorithms.AES(self.key), modes.CFB(iv))
        decryptor = cipher.decryptor()
        plaintext = decryptor.update(ciphertext) + decryptor.finalize()
        unpadder = pad.PKCS7(128).unpadder()
        unpadded_data = unpadder.update(plaintext) + unpadder.finalize()
        return unpadded_data.decode()


def encrypt(plaintext: str, key, iv):
    cipher = Cipher(algorithms.AES(key), modes.CFB(iv))
    encryptor = cipher.encryptor()
    padder = pad.PKCS7(128).padder()
    padded_data = padder.update(plaintext.encode()) + padder.finalize()
    ciphertext = encryptor.update(padded_data) + encryptor.finalize()
    return base64.b64encode(iv + ciphertext).decode()


def decrypt(ciphertext: str, key, iv):
    ciphertext = base64.b64decode(ciphertext.encode())
    iv = ciphertext[:16]  # Extract the IV from the ciphertext
    ciphertext = ciphertext[16:]
    cipher = Cipher(algorithms.AES(key), modes.CFB(iv))
    decryptor = cipher.decryptor()
    plaintext = decryptor.update(ciphertext) + decryptor.finalize()
    unpadder = pad.PKCS7(128).unpadder()
    unpadded_data = unpadder.update(plaintext) + unpadder.finalize()
    return unpadded_data.decode()
