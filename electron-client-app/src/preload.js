/*
Use preload.js in Electron applications to execute code before the main window starts, for tasks that need to be done in the main process.
*/

const aesKey = "secret key 123";
const iv = "1234567890123456";

console.log("preload.js is being executed");

CryptoJS = require('crypto-js');


// Encrypt "Hello World!"
// initialize the key and iv
var ciphertext = CryptoJS.AES.encrypt('Hello World!', aesKey, { iv: iv }).toString();

// Decrypt "Hello World!"
var bytes  = CryptoJS.AES.decrypt(ciphertext, aesKey);

var originalText = bytes.toString(CryptoJS.enc.Utf8);

console.log(ciphertext); // 'U2FsdGVkX18D2qUZv4Fb8R5K5Z5
console.log(originalText); // 'Hello World!'

console.log("Key is " + aesKey);
console.log("IV is " + iv);