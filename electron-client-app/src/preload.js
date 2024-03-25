/*
Use preload.js in Electron applications to execute code before the main window starts, for tasks that need to be done in the main process.
*/

console.log("preload.js is being executed");

const { ipcRenderer, contextBridge } = require('electron');

/*
contextBridge.exposeInMainWorld('electron', {
    ipcRenderer: ipcRenderer,
    require: require
});
*/

// We want to make the requre avaiable in the socket.js file so we have to expose it to the window object
window.electron = {
    require: require
};