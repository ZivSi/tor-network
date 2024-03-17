import kotlin.math.min
import kotlin.math.pow
import kotlin.math.sqrt


class RSAHandler(keySize: Int = 2048 * 2 * 2) {
    private var p: ULong = 0UL
    private var q: ULong = 0UL
    private var modulus: ULong = 0UL
    private var t: ULong = 0UL
    private var publicKey: ULong = 0UL
    private var privateKey: ULong = 0UL
    private var clientPublicKey: ULong = 0UL
    private var clientModulus: ULong = 0UL

    init {
        p = generatePrime(keySize)
        q = generatePrime(keySize)

        modulus = p * q
        t = totient(modulus)

        publicKey = generateE(t)
        privateKey = calculateD(publicKey, t)
    }

    fun setClientPublicKey(clientPublicKey: ULong) {
        this.clientPublicKey = clientPublicKey
    }

    fun setClientModulus(clientModulus: ULong) {
        this.clientModulus = clientModulus
    }

    fun getPublicKey(): ULong {
        return publicKey
    }

    fun getModulus(): ULong {
        return modulus
    }

    fun getPrivateKey(): ULong {
        return privateKey
    }

    fun encrypt(m: String): List<Long> {
        if (clientPublicKey == 0UL || clientModulus == 0UL) {
            throw RuntimeException("Client public key or modulus not set")
        }

        val encryptedMessage = mutableListOf<Long>()

        for (c in m) {
            val encryptedChar = modPow(c.code.toLong().toULong(), clientPublicKey, clientModulus)
            encryptedMessage.add(encryptedChar)
        }

        return encryptedMessage
    }

    fun encryptToString(m: String): String {
        // Encrypt to "[1, 2, 3, 4]"
        val encryptedList = encrypt(m)
        return encryptedList.joinToString(separator = ", ", prefix = "[", postfix = "]")
    }

    private fun decrypt(encrypted: List<ULong>): String {
        val decryptedMessage = StringBuilder()

        for (encryptedChar in encrypted) {
            val decryptedChar = modPow(encryptedChar, privateKey, modulus).toChar()
            decryptedMessage.append(decryptedChar)
        }

        return decryptedMessage.toString()
    }

    fun decrypt(encrypted: String): String {
        // Remove brackets and split by comma and space
        val encryptedList = encrypted.substring(1, encrypted.length - 1).split(", ").map { it.toULong() }
        return decrypt(encryptedList)
    }


    private fun generatePrime(bigNum: Int): ULong {
        while (true) {
            val number = (bigNum / 2..bigNum).random().toULong()

            if (isPrime(number)) {
                return number
            }
        }
    }

    private fun totient(n: ULong): ULong {
        var result = n
        var i = 2UL
        var n = n

        while (i * i <= n) {
            if (n % i == 0UL) {
                result -= result / i
                while (n % i == 0UL) {
                    n /= i
                }
            }
            i += 1UL
        }

        if (n > 1UL) {
            result -= result / n
        }

        return result
    }

    private fun isPrime(n: ULong): Boolean {
        if (n < 2UL) return false

        for (i in 2UL..sqrt(n.toDouble()).toULong() + 1UL) {
            if (n % i == 0UL) return false
        }

        return true
    }

    private fun generateE(num: ULong): ULong {
        for (e in 2UL..<num) {
            if (gcd(e, num) == 1UL) {
                return e
            }
        }

        throw RuntimeException("Couldn't generate e")
    }

    private fun modPow(base: ULong, exponent: ULong, modulus: ULong): Long {
        var result = 1UL
        var exp = exponent
        var baseVar = base

        while (exp > 0UL) {
            if (exp % 2UL == 1UL) {
                result = (result * baseVar) % modulus
            }
            exp /= 2UL
            baseVar = (baseVar * baseVar) % modulus
        }

        return result.toLong()
    }

    private fun calculateD(e: ULong, totient: ULong): ULong {
        var k = 1UL

        while ((k * totient + 1UL) % e != 0UL) {
            k += 1UL
        }

        return (k * totient + 1UL) / e
    }


    private fun gcd(a: ULong, b: ULong): ULong {
        var x = a
        var y = b
        while (y != 0UL) {
            val temp = y
            y = x % y
            x = temp
        }
        return x
    }

    fun foramtForSending(): String {
        return "$publicKey::::${modulus}"
    }
}
