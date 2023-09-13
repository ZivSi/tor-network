import random
from math import gcd


class RSA:
    def __init__(self, key_size=2048):
        self.p = self.generate_prime(key_size)
        self.q = self.generate_prime(key_size)
        # Modulus
        self.modulus = self.p * self.q
        self.t = self.totient(self.modulus)
        # Public
        self.publicKey = self.generate_e(self.t)
        # Private
        self.privateKey = self.calculate_d(self.publicKey, self.t)

        # Client key
        self.client_public_key = None
        self.client_modulus = None

    def setClientPublicKey(self, clientPublicKey: int):
        self.client_public_key = clientPublicKey

    def setClientModulus(self, clientModulus: int):
        self.client_modulus = clientModulus

    def generate_prime(self, big_num):
        while True:
            number = random.randint(int(big_num / 2), big_num)

            if self.is_prime(number):
                return number

    def totient(self, n):
        result = n
        i = 2
        while i * i <= n:
            if n % i == 0:
                result -= result // i
                while n % i == 0:
                    n //= i
            i += 1
        if n > 1:
            result -= result // n
        return result

    def is_prime(self, n):
        if n < 2:
            return False

        for i in range(2, int(n ** 0.5) + 1):
            if n % i == 0:
                return False

        return True

    def generate_e(self, num):
        for e in range(2, num):
            if gcd(e, num) == 1:  # (p - 1) * (q - 1)
                return e

        raise Exception("Did not find e that is acceptable")

    def calculate_d(self, e, totient):
        k = 1
        while (k * totient + 1) % e != 0:
            k += 1
        d = (k * totient + 1) // e
        return d

    def encrypt(self, m):
        if self.client_public_key is None or self.client_modulus is None:
            return

        m_ascii = [ord(x) for x in m]
        cipher = []
        for x in m_ascii:
            encrypted_value = pow(x, self.client_public_key, self.client_modulus)
            cipher.append(encrypted_value)
        return cipher

    def decrypt(self, encrypted):
        plaintext_ascii = []
        for x in encrypted:
            decrypted_value = pow(x, self.privateKey, self.modulus)
            ascii_value = chr(decrypted_value)
            plaintext_ascii.append(ascii_value)
        return ''.join(plaintext_ascii)


def encrypt(m, client_public_key, client_modulus):
    if client_public_key is None or client_modulus is None:
        return

    m_ascii = [ord(x) for x in m]
    cipher = []
    for x in m_ascii:
        encrypted_value = pow(x, client_public_key, client_modulus)
        cipher.append(encrypted_value)
    return cipher


def main():
    rsa = RSA()

    # Set client public key and modulus to encrypt message!!!

    message = "Hello World!"
    encrypted = rsa.encrypt(message)
    decrypted = rsa.decrypt(encrypted)
    print(f"Encrypted message: {encrypted}")
    print(f"Decrypted message: {decrypted}")


if __name__ == '__main__':
    main()
