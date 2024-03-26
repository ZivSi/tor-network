const MIN_PATH_LENGTH = 3;
const SPLITER = "::::";

document.addEventListener("DOMContentLoaded", function () {
    var connectButton = document.querySelector('.connect-button');
    var pathLengthInput = document.getElementById('pathLen');
    var additionalInputs = document.getElementById('additional-inputs');
    var fileInput = document.getElementById('file-upload');
    var fileNameDisplay = document.getElementById('file-name');
    var welcomeText = document.getElementById('welcome-text');
    var sendButton = document.querySelector('.connect2-button');
    var messageBox = document.getElementById('message');
    var ipField = document.getElementById('ip-address');
    var portField = document.getElementById('port');
    var connection = null;
    var usernameField = document.getElementById('username');

    pathLengthInput.value = "3";
    ipField.value = "127.0.0.1";
    portField.value = "10210";
    messageBox.value = "Hello from client";


    connectButton.addEventListener('click', function (event) {
        console.log("connectButton clicked");

        if (!pathValueValid(pathLengthInput.value)) {
            alert("Please enter a valid path length");

            location.reload();

            return;
        }

        hideElements(pathLengthInput, additionalInputs, welcomeText);

        //connecting to local server
        connection = new Connection();
        console.log("connected");

        console.log("Sending handshake message");
        connection.handshake(pathLengthInput.value);

        // Start listening for incoming messages
        connection.receiveInThread();
    });

    fileInput.addEventListener('change', function (event) {
        var fileName = event.target.files[0].name;
        fileNameDisplay.textContent = 'Selected file: ' + fileName;
    });

    sendButton.addEventListener('click', function (event) {
        console.log("sendButton clicked");

        if (connection === null) {
            alert("Please connect to the server first");

            return;
        }

        // ip and port given?
        if ((ipField.value === "" || portField.value === "") && (usernameField.value === "")) {
            alert("Please enter the IP and port of the server or the username of the user you want to send a message to");

            return;
        }

        var messageText = messageBox.value;

        messageText = formatMessage(messageText) + "\n";

        if (usernameField.value !== "") {
            connection.sendMessageToUser(usernameField.value, messageText);

            return;
        }

        connection.sendMessage(ipField.value, portField.value, messageText);
    });
});


function hideElements(pathLengthInput, additionalInputs, welcomeText) {
    pathLengthInput.style.visibility = 'hidden';
    additionalInputs.style.display = 'block';
    welcomeText.style.display = 'none';
}

function pathValueValid(pathValue) {
    return pathValue >= MIN_PATH_LENGTH && pathValue !== null;
}

function buildMessage(ip, port, messageText) {
    return `${ip}${SPLITER}${port}${SPLITER}${messageText}`;
}

function formatMessage(messageText) {
    // Replace \n with newline character
    const formattedMessage = messageText.replace(/\\n/g, '\n');
    return formattedMessage;
}

const BackToFront = {
    MESSAGE: 0,
    INFO: 1,
    ERROR_NODES_LENGTH: 2,
    ERROR_USERNAME_NOT_FOUND: 3,
    ERROR_HOST_UNREACHABLE: 4,
    ERROR_PATH_NOT_COMPLETE: 5,
    ERROR_INVALID_ARGS: 6,
    USERNAME: 7,
    ERROR_CONNECTION_TIMEOUT: 8,
    ERROR_PATH_TIMEOUT: 9
};

function isErrorCode(code) {
    return code === BackToFront.ERROR_NODES_LENGTH || code === BackToFront.ERROR_USERNAME_NOT_FOUND || code === BackToFront.ERROR_HOST_UNREACHABLE || code === BackToFront.ERROR_PATH_NOT_COMPLETE || code === BackToFront.ERROR_INVALID_ARGS || code === BackToFront.ERROR_CONNECTION_TIMEOUT || code === BackToFront.ERROR_PATH_TIMEOUT;
}

class JsonResponse {

    constructor(nodeIp, nodePort, nodeConversationId, messageCode, senderIp, senderPort, message) {
        if (arguments.length === 1) {
            // If only one argument is provided, assume it's a JSON string
            this.fromString(nodeIp);
        } else {
            // Otherwise, assume individual parameters are provided
            this.nodeIp = nodeIp;
            this.nodePort = nodePort;
            this.nodeConversationId = nodeConversationId;
            this.messageCode = parseInt(messageCode);


            if (!isErrorCode(messageCode)) {
                this.senderIp = senderIp;
                this.senderPort = senderPort;
            }

            this.message = message;
        }
    }

    fromString(jsonString) {
        // Parse the JSON string
        let parsedJson = JSON.parse(jsonString);

        // Check if the message contains another JSON string
        let innerMessage = parsedJson.message;
        try {
            let innerJson = JSON.parse(innerMessage);
            // Update the properties with values from the inner JSON
            this.nodeIp = innerJson.nodeIp;
            this.nodePort = innerJson.nodePort;
            this.nodeConversationId = innerJson.nodeConversationId;
            this.messageCode = parseInt(innerJson.messageCode);
            if (!isErrorCode(innerJson.messageCode)) {
                this.senderIp = innerJson.senderIp;
                this.senderPort = innerJson.senderPort;
            }
            this.message = innerJson.message;
        } catch (error) {
            // If parsing the inner JSON fails, use the outer JSON
            this.nodeIp = parsedJson.nodeIp;
            this.nodePort = parsedJson.nodePort;
            this.nodeConversationId = parsedJson.nodeConversationId;
            this.messageCode = parseInt(parsedJson.messageCode);
            if (!isErrorCode(parsedJson.messageCode)) {
                this.senderIp = parsedJson.senderIp;
                this.senderPort = parsedJson.senderPort;
            }
            this.message = parsedJson.message;
        }
    }

    isError() {
        return isErrorCode(this.messageCode);
    }

    getMessage() {
        if (!this.isError()) { return this.message; }

        // switch between code and message
        switch (this.messageCode) {
            case BackToFront.ERROR_NODES_LENGTH:
                return "The path is not complete";
            case BackToFront.ERROR_USERNAME_NOT_FOUND:
                // 	string errorMessage = "Error: Could not find username \"" + username + "\"";
                var username = this.message.substring(this.message.indexOf('"') + 1, this.message.lastIndexOf('"'));
                return "The username was not found: " + username;
            case BackToFront.ERROR_HOST_UNREACHABLE:
                return "The host is unreachable: " + this.nodeIp + ":" + this.nodePort;
            case BackToFront.ERROR_PATH_NOT_COMPLETE:
                return "The path is not complete";
            case BackToFront.ERROR_INVALID_ARGS:
                return "Invalid arguments";
            case BackToFront.ERROR_CONNECTION_TIMEOUT:
                return "Connection timeout";
            case BackToFront.ERROR_PATH_TIMEOUT:
                return "Path timeout";
            default:
                return "Unknown error";
        }
    }

    getCode() {
        return this.messageCode;
    }

    printJsonResponse() {
        console.log("nodeIp:", this.nodeIp);
        console.log("nodePort:", this.nodePort);
        console.log("nodeConversationId:", this.nodeConversationId);
        console.log("messageCode:", this.messageCode);
        console.log("senderIp:", this.senderIp);
        console.log("senderPort:", this.senderPort);
        console.log("message:", this.message);
    }
}