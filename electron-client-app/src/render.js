const { ipcRenderer } = require('electron');
const { connectToServer, sendData } = require('./socket.js');

document.addEventListener("DOMContentLoaded", function () {
    var connectButton = document.querySelector('.connect-button');
    var ipAddressInput = document.getElementById('ip-address');
    var portInput = document.getElementById('port');
    var usernameInput = document.getElementById('username');
    var messageInput = document.getElementById('message');

    connectButton.addEventListener('click', function(event) {
        var ip = ipAddressInput.value.trim();
        var port = portInput.value.trim();
        var username = usernameInput.value.trim();
        var message = messageInput.value.trim();

        if (!ip || !port) {
            alert('Please enter IP address and port number');
            return;
        }

        connectToServer(ip, parseInt(port));

        if (username) {
            sendData(`USERNAME::::${username}::::${message}`);
        } else {
            sendData(`IP::::${ip}::::${port}::::${message}`);
        }
    });
});
