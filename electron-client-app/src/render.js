console.log("Render.js is being called");

const MIN_LENGTH = 3;

// ----------- On start -----------
document.addEventListener("DOMContentLoaded", function () {
    const pathLengthInput = document.getElementById('pathLen');
    const errorTextView = document.getElementById('errormessage');
    const minusButton = document.querySelector('.number-input button.minus');
    const plusButton = document.querySelector('.number-input button.plus');
    
    hideError(errorTextView); // Hide error message initially
    
    // Check input value on input event
    pathLengthInput.addEventListener('input', function(event) {
        const lengthValue = parseInt(pathLengthInput.value);
        
        if (lengthValue < MIN_LENGTH || isNaN(lengthValue)) {
            showError(errorTextView);
        } else {
            hideError(errorTextView);
        }
    });
    
    // Disable minus button if path length is already at minimum
    if (parseInt(pathLengthInput.value) === MIN_LENGTH) {
        minusButton.disabled = true;
    }
    
    // Event listener for minus button click
    minusButton.addEventListener('click', function(event) {
        const lengthValue = parseInt(pathLengthInput.value);
        if (lengthValue === MIN_LENGTH) {
            event.preventDefault(); // Prevent default action (decreasing value)
        }
    });
    
    // Event listener for plus button click
    plusButton.addEventListener('click', function(event) {
        const lengthValue = parseInt(pathLengthInput.value);
        if (lengthValue === MIN_LENGTH) {
            minusButton.disabled = false; // Enable minus button if path length is increased from minimum
        }
    });
});

function showError(element) {
    element.style.display = 'inline';
}

function hideError(element) {
    element.style.display = 'none';
}
