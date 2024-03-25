const net = window.electron.require('net');
const pack = window.electron.require('buffer-pack');

const START_PATH_DESIGN_STRING = "START PATH DESIGN";


function sendData(socket, data) {
    // Convert the data to a string
    var jsonData = JSON.stringify(data);
    // Remove the quotes from the string
    jsonData = jsonData.substring(1, jsonData.length - 1);

    const dataSize = Buffer.alloc(8); // Allocate buffer for size_t (8 bytes)

    // Convert jsonData length to a buffer
    dataSize.writeBigUInt64LE(BigInt(jsonData.length));

    // Send the size of the data
    socket.write(dataSize);

    // Send the actual data
    socket.write(jsonData);
    console.log('Sent:', jsonData);
}

class Connection {
    constructor(serverIp = "127.0.0.1", serverPort = 5060 * 3) {
        this.serverIp = serverIp;
        this.serverPort = serverPort;

        this.socket = this.connectTo(this.serverIp, this.serverPort);
    }

    connectTo(ip, port) {
        return net.createConnection({ host: ip, port: port }, () => {
            console.log('Connected to: ' + ip + ':' + port);
        });
    }

    handshake(numOfNodes) {
        sendData(this.socket, START_PATH_DESIGN_STRING + ":" + numOfNodes);
    }

    sendMessage(ip, port, message) {
        console.log("Hello from sendMessage");
        var completeMessage = `${ip}::::${port.toString()}::::${message}`;
        sendData(this.socket, completeMessage);
    }

    sendMessageToUser(username, message) {
        console.log("Hello from sendMessageToUser");
        var completeMessage = `${username}::::${message}`;
        sendData(this.socket, completeMessage);
    }

    receiveInThread() {
        let dataSizeBuffer = Buffer.alloc(8); // Buffer to hold the size of the data

        this.socket.on('data', (data) => {
            if (dataSizeBuffer.length > 0) {
                // If dataSizeBuffer still has data, it means we're receiving the size
                if (data.length >= dataSizeBuffer.length) {
                    // Received enough data to extract the size
                    dataSizeBuffer = data.slice(0, 8); // Extract the size buffer
                    const dataSize = dataSizeBuffer.readBigUInt64LE(); // Convert buffer to size_t

                    // Prepare to receive the actual data
                    let remainingData = data.slice(8); // Extract remaining data
                    const message = remainingData.toString(); // Convert remaining data to string
                    console.log('Received:', message);

                    this.buildJsonReponseObject(message);

                    // Reset dataSizeBuffer for future use
                    dataSizeBuffer = Buffer.alloc(8);
                } else {
                    // Not enough data received yet, append to dataSizeBuffer
                    dataSizeBuffer = Buffer.concat([dataSizeBuffer, data]);
                }
            } else {
                // dataSizeBuffer is empty, meaning we've already received the size
                const message = data.toString();
                console.log('Received:', message);
            }
        });

        this.socket.on('error', (err) => {
            console.error('Error:', err);

            alert("Error: " + err);
        });

        this.socket.on('close', () => {
            console.log('Connection closed');

            alert("Connection closed");
            location.reload();
        });
    }

    buildJsonReponseObject(message) {
        try {
            var jsonResponse = new JsonResponse(message);

            console.error("Message code: " + jsonResponse.getCode());
            if (jsonResponse.isError()) {                
                alert("Error: " + jsonResponse.getMessage() + " (code: " + jsonResponse.getCode() + ")");
            }
            else if (jsonResponse.getMessage() == "Path design completed") {
                alert("Path design completed!");
            }

        } catch (e) {
            return;
        }
    }

    getSocket() {
        return this.socket;
    }
}