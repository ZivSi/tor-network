const net = require('net');

const START_PATH_DESIGN_STRING = "START PATH DESIGN";
const NUM_OF_NODES = "5";


function sendData(connection, data) {
    try {
        // First send the size of the data in 8 bytes
        const dataSizeBuffer = Buffer.alloc(8);
        dataSizeBuffer.writeBigUInt64LE(BigInt(data.length));

        console.log("Sending data size:", dataSizeBuffer);

        connection.write(dataSizeBuffer);

        // Send the actual data
        console.log("Sending data:", data);
        connection.write(data);
    } catch (error) {
        console.error("Error sending data:", error);
    }
}


class Connection {
    constructor(serverIp = "127.0.0.1", serverPort = 5060 * 3) {
        this.serverIp = serverIp;
        this.serverPort = serverPort;

        this.socket = connectTo(this.serverIp, this.serverPort);
    }

    connectTo(ip, port) {
        return net.createConnection({ host: ip, port: port }, () => {
            console.log('Connected to: ' + ip + ':' + port);
        });
    }

    handshake(numOfNodes) {
        sendData(this.socket, `${START_PATH_DESIGN_STRING}:${numOfNodes}`);
    }

    sendMessage(ip, port, message) {
        sendData(this.socket, `${ip}::::${port}::::${message}`);
    }

    sendMessage(username, message) {
        sendData(this.socket, `${username}::::${message}`);
    }

    receiveInThread() {
        this.socket.on('data', (data) => {
            const dataSize = data.readUIntLE(0, 8);
            const message = data.slice(8, 8 + dataSize).toString();
            console.log('Received:', message);
        });

        this.socket.on('error', (err) => {
            console.error('Error:', err);
        });

        this.socket.on('close', () => {
            console.log('Connection closed');
        });
    }

    getSocket() {
        return this.socket;
    }
}

module.exports = Connection, sendData;