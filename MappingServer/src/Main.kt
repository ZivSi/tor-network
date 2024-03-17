
fun main() {
    // Test the rsa encryption
    val rsaHandler = RSAHandler()
    val rsaHandler2 = RSAHandler()

    rsaHandler.setClientPublicKey(rsaHandler2.getPublicKey())
    rsaHandler.setClientModulus(rsaHandler2.getModulus())

    rsaHandler2.setClientPublicKey(rsaHandler.getPublicKey())
    rsaHandler2.setClientModulus(rsaHandler.getModulus())

    val message = "Hello, world!"
    val encrypted = rsaHandler.encryptToString(message)

    println("Encrypted: $encrypted")

    val decrypted = rsaHandler2.decrypt(encrypted)

    println("Decrypted: $decrypted")

    val server = Server()
    server.start()
}