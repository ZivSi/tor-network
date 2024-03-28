import java.net.ServerSocket
import java.net.Socket
import java.util.*
import kotlin.math.abs
import kotlin.math.min

class Server(port: Int = 5060 * 2) {
    private var usernamesMap = mutableMapOf<String, ConnectionPair>()
    private var port = min(abs(port), 65535)
    private val rsaHandler = RSAHandler()
    private val PEPPER3 = "thisIsAPepperForTheKotlinServerYe=s"

    init {
    }

    fun start() {
        println("Server started")

        usernamesMap["echo"] = ConnectionPair("45.79.112.203", 4242)
        usernamesMap["local"] = ConnectionPair("127.0.0.1", 10210)

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


            sendPublicKey(clientSocket)

            val data = clientSocket.getInputStream().bufferedReader().readLine()
            println("Received data: $data")

            val decrypted = rsaHandler.decrypt(data)

            if (isExitNode(decrypted)) {
                val (hashed, username) = decrypted.split(Utility.SPLITER)

                val connectionPair = usernamesMap.get(username.lowercase())

                println("Username: " + username + " -> " + username.lowercase())

                println("Exit node asks for \"$username\" : $connectionPair")

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

            clientSocket.close()
            return
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

            return false
        } catch (e: Exception) {
            val (hashed, username) = decrypted.split(Utility.SPLITER)
            val currentHash = Utility.hashStr(username + PEPPER3).uppercase(Locale.getDefault())

            return hashed == currentHash
        }
    }
}