const net = require('net');

const SERVER_ADDRESS = 'localhost';
const SERVER_PORT = 5060 * 3; 
const START_PATH_DESIGN_STRING = "START PATH DESIGN";
const NUM_OF_NODES = "5";

function connectToServer() {
    const clientSocket = net.createConnection({ host: SERVER_ADDRESS, port: SERVER_PORT }, () => {
        console.log('Connected to server');

        sendData(clientSocket, `${START_PATH_DESIGN_STRING}:${NUM_OF_NODES}`);

        clientSocket.on('data', (data) => {
            const dataSize = data.readUIntLE(0, 8);
            const message = data.slice(8, 8 + dataSize).toString();
            console.log('Received:', message);
        });

        clientSocket.on('error', (err) => {
            console.error('Error:', err);
        });

        clientSocket.on('close', () => {
            console.log('Connection closed');
        });

        process.stdin.on('data', (input) => {
            const message = input.toString().trim();
            if (message.toLowerCase() === 'quit') {
                clientSocket.end();
                process.exit();
            }
            sendData(clientSocket, message);
        });
    });

    return clientSocket;
}

function sendData(connection, data) {
    const dataSize = Buffer.byteLength(data);
    const dataBuffer = Buffer.allocUnsafe(8 + dataSize);

    dataBuffer.writeUIntLE(dataSize, 0, 8);
    dataBuffer.write(data, 8, dataSize, 'utf8');

    console.log('Sending data:', data);
    connection.write(dataBuffer);
}

module.exports = connectToServer;
