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


    connectButton.addEventListener('click', function(event) {
        console.log("connectButton clicked");

        if(!pathValueValid(pathLengthInput.value)) {
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

    fileInput.addEventListener('change', function(event) {
        var fileName = event.target.files[0].name;
        fileNameDisplay.textContent = 'Selected file: ' + fileName;
    });

    sendButton.addEventListener('click', function(event) {
        console.log("sendButton clicked");

        if(connection === null) {
            alert("Please connect to the server first");

            return;
        }

        // ip and port given?
        if((ipField.value === "" || portField.value === "") && (usernameField.value === "")) {
            alert("Please enter the IP and port of the server or the username of the user you want to send a message to");

            return;
        }

        var messageText = messageBox.value;

        messageText = formatMessage(messageText) + "\n";

        if(usernameField.value !== "") {
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