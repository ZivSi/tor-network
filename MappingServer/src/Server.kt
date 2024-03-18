import java.net.ServerSocket
import java.net.Socket
import kotlin.math.abs
import kotlin.math.min

class Server(port: Int = 5060 * 2) {
    private var usernamesMap = mutableMapOf<String, ConnectionPair>()
    private var port = min(abs(port), 65535)
    private val rsaHandler = RSAHandler()

    init {
    }

    fun start() {
        println("Server started")

        usernamesMap["test"] = ConnectionPair("127.0.0.1", 10210)

        val serverSocket = createServerSocket()
        listenForConnections(serverSocket)
    }

    private fun createServerSocket(): ServerSocket {
        return ServerSocket(port)
    }

    private fun listenForConnections(serverSocket: ServerSocket) {
        println("Server listening on port $port...")

        while (true) {
            val clientSocket = serverSocket.accept()
            println("Client connected: (${clientSocket.inetAddress.hostAddress}:${clientSocket.port})")

            val thread = Thread {
                handleConnection(clientSocket)
            }

            thread.isDaemon = true
            thread.start()
        }
    }

    private fun handleConnection(clientSocket: Socket) {
        try {
            val (clientPublicKey, clientModulus) = receivePublicKey(clientSocket)

            rsaHandler.setClientPublicKey(clientPublicKey)
            rsaHandler.setClientModulus(clientModulus)

            println("Received client public key: ($clientPublicKey, $clientModulus)")

            sendPublicKey(clientSocket)

            val data = clientSocket.getInputStream().bufferedReader().readLine()
            println("Type of data: ${data::class.simpleName}")
            println("Received data: $data")

            val decrypted = rsaHandler.decrypt(data)

            if (isExitNode(decrypted)) {
                println("Exit node asks for \"$decrypted\"")

                val connectionPair = usernamesMap.get(decrypted)

                if (connectionPair == null) {
                    val notFoundEncrypted = rsaHandler.encryptToString("Not found")

                    clientSocket.getOutputStream().write(notFoundEncrypted.toByteArray())
                }

                val encrypted = rsaHandler.encryptToString(connectionPair.toString())

                clientSocket.getOutputStream().write(encrypted.toByteArray())
                return
            }

            val connectionPair = ConnectionPair(decrypted)

            var username = Utility.generateUsername()

            while (usernamesMap.containsKey(username)) {
                username = Utility.generateUsername()
            }

            usernamesMap[username] = connectionPair

            println("Usernames map: $usernamesMap")

            val encrypted = rsaHandler.encryptToString(username)

            clientSocket.getOutputStream().write(encrypted.toByteArray())
        } catch (e: Exception) {
            println("Error handling connection: ${e.message}")
        } finally {
            clientSocket.close()
        }
    }

    private fun sendPublicKey(clientSocket: Socket) {
        val publicKey = rsaHandler.foramtForSending()
        clientSocket.getOutputStream().write(publicKey.toByteArray())
    }

    private fun receivePublicKey(clientSocket: Socket): Pair<ULong, ULong> {
        val publicKey = clientSocket.getInputStream().bufferedReader().readLine()
        return Utility.splitRsaKey(publicKey)
    }

    private fun isExitNode(decrypted: String): Boolean {
        try {
            ConnectionPair(decrypted)

            // If you could create a ConnectionPair, it's a client which sent its connection details
            return false
        } catch (e: Exception) {
            return true
        }
    }
}