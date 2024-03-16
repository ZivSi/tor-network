import kotlin.math.pow
import kotlin.math.sqrt

class RSAHandler(keySize: Int = 1024) {
    private var p: Long = 0
    private var q: Long = 0
    private var modulus: Long = 0
    private var t: Long = 0
    private var publicKey: Long = 0
    private var privateKey: Long = 0
    private var clientPublicKey: Long = 0
    private var clientModulus: Long = 0

    init {
        val biggestNum = 2.0.pow(keySize - 1).toLong()

        p = generatePrime(biggestNum)
        q = generatePrime(biggestNum)

        modulus = p * q
        t = totient(modulus)

        publicKey = generateE(t)
        privateKey = calculateD(publicKey, t)
    }

    fun setClientPublicKey(clientPublicKey: Long) {
        this.clientPublicKey = clientPublicKey
    }

    fun setClientModulus(clientModulus: Long) {
        this.clientModulus = clientModulus
    }

    fun getPublicKey(): Long {
        return publicKey
    }

    fun getModulus(): Long {
        return modulus
    }

    fun getPrivateKey(): Long {
        return privateKey
    }

    fun encrypt(m: String): List<Long> {
        if (clientPublicKey == 0L || clientModulus == 0L) {
            throw RuntimeException("Client public key or modulus not set")
        }

        val encryptedMessage = mutableListOf<Long>()

        for (c in m) {
            val encryptedChar = modPow(c.toLong(), clientPublicKey, clientModulus)
            encryptedMessage.add(encryptedChar)
        }

        return encryptedMessage
    }

    fun encryptToString(m: String): String {
        // Encrypt to "[1, 2, 3, 4]"
        val encryptedList = encrypt(m)
        return encryptedList.joinToString(separator = ", ", prefix = "[", postfix = "]")
    }

    private fun decrypt(encrypted: List<Long>): String {
        val decryptedMessage = StringBuilder()

        for (encryptedChar in encrypted) {
            val decryptedChar = modPow(encryptedChar, privateKey, modulus).toChar()
            decryptedMessage.append(decryptedChar)
        }

        return decryptedMessage.toString()
    }

    fun decrypt(encrypted: String): String {
        // Encrypted message is in format: "[1, 2, 3, 4]"
        val encryptedList = encrypted.substring(1, encrypted.length - 1).split(", ").map { it.toLong() }
        return decrypt(encryptedList)
    }

    private fun generatePrime(bigNum: Long): Long {
        var prime = (0..<bigNum).random()

        while (!isPrime(prime)) {
            prime = (0..<bigNum).random()
        }

        return prime
    }

    private fun totient(n: Long): Long {
        var result = n
        var nVar = n

        for (i in 2..sqrt(nVar.toDouble()).toLong()) {
            if (nVar % i == 0L) {
                while (nVar % i == 0L) {
                    nVar /= i
                }

                result -= result / i
            }
        }

        if (nVar > 1) {
            result -= result / nVar
        }

        return result
    }

    private fun isPrime(n: Long): Boolean {
        if (n <= 1) return false

        for (i in 2..sqrt(n.toDouble()).toLong()) {
            if (n % i == 0L) return false
        }

        return true
    }

    private fun generateE(num: Long): Long {
        for (e in 2..<num) {
            if (gcd(e, num) == 1L) {
                return e
            }
        }

        throw RuntimeException("Couldn't generate e")
    }

    private fun modPow(base: Long, exponent: Long, modulus: Long): Long {
        var result = 1L
        var exp = exponent
        var baseVar = base

        while (exp > 0) {
            if (exp % 2 == 1L) {
                result = (result * baseVar) % modulus
            }
            exp /= 2
            baseVar = (baseVar * baseVar) % modulus
        }

        return result
    }

    private fun calculateD(e: Long, totient: Long): Long {
        val d = modInverse(e, totient)
        return d
    }

    private fun modInverse(a: Long, m: Long): Long {
        var aVar = a
        var mVar = m
        val m0 = m
        var y: Long = 0
        var x: Long = 1

        if (m == 1L) return 0

        while (aVar > 1) {
            val q = aVar / mVar
            val t = mVar

            mVar = aVar % mVar
            aVar = t
            val tVar = y

            y = x - q * y
            x = tVar
        }

        if (x < 0) x += m0

        return x
    }

    private fun gcd(a: Long, b: Long): Long {
        var aVar = a
        var bVar = b

        while (bVar != 0L) {
            val temp = bVar
            bVar = aVar % bVar
            aVar = temp
        }
        return aVar
    }

    fun foramtForSending(): String {
        return "$publicKey::::${modulus}"
    }
}
