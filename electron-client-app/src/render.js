const { ipcRenderer } = require('electron');
const { sendData } = require('./socket.js');

// Get the Connection class from ./socket.js
const Connection = require('./socket.js');

// Open developer tools
document.addEventListener("keydown", (e) => {
    if (e.key === "F12") {
        ipcRenderer.send('open-dev-tools');
    }
});

document.addEventListener("DOMContentLoaded", function () {
    var connectButton = document.querySelector('.connect-button');
    var ipAddressInput = document.getElementById('ip-address');
    var portInput = document.getElementById('port');
    var usernameInput = document.getElementById('username');
    var messageInput = document.getElementById('message');

    connectButton.addEventListener('click', function (event) {
        var ip = ipAddressInput.value.trim();
        var port = portInput.value.trim();
        var username = usernameInput.value.trim();
        var message = messageInput.value.trim();

        console.log("Clicked connect button:", ip, port, username, message);

        // We don't need the user to tell us the local server's IP and port

        var connection = Connection()

        if (username) {
            sendData(connection.getSocket(), `USERNAME::::${username}::::${message}`);
        } else {
            sendData(connection.getSocket(), `IP::::${ip}::::${port}::::${message}`);
        }
    });
});
