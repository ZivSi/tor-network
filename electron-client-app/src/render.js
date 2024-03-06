/*
Use render.js for client-side code in web applications, particularly for interacting with the DOM and creating dynamic web pages.
*/



console.log("Render.js is being called");

const MIN_LENGTH = 3;
var showError = false;
var errorShown = false;

// ----------- On start -----------
document.addEventListener("DOMContentLoaded", function () {
    initializeInputListener();
    initializeButtonListener();

    document.getElementById('errormessage').style.display = 'none';
});

function initializeInputListener() {
    var pathLengthInput = document.getElementById('pathLen');
    var errorTextView = document.getElementById('errormessage');

    pathLengthInput.addEventListener('input', function (event) {
        const lengthValue = parseInt(event.target.value);

        showError = lengthValue < MIN_LENGTH || isNaN(lengthValue);

        console.log(showError ? "Less than min len!" : "");
        event.target.value = showError ? 3 : event.target.value;


        // Conditions will have better runtime than each time set the visibilty every time
        if (showError && !errorShown) {
            setVisibility(errorTextView, true);
            errorShown = true;
        } else if (!showError && errorShown) {
            setVisibility(errorTextView, false);
            errorShown = false;
        }
    });
}

function initializeButtonListener() {
    var plusButton = document.getElementById('increase');
    var minusButton = document.getElementById('decrease');

    var pathLengthInput = document.getElementById('pathLen');

    plusButton.addEventListener('click', function (event) {
        pathLengthInput.value = parseInt(pathLengthInput.value) + 1;
    }

    );

    minusButton.addEventListener('click', function (event) {
        pathLengthInput.value = parseInt(pathLengthInput.value) - 1;
    }

    );
}

function setVisibility(element, visible) {
    if (visible) {
        element.style.display = 'inline';
    } else {
        element.style.display = 'none';
    }
}