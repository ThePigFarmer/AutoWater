console.log("starting form.js...");

// open and close form
function openDurationEntryForm(valveNum, valveNumInDay) {
    document.getElementById("duration-input-form").style.display = "block";
}

/*
  // form actions
const form = document.querySelector('form');
form.addEventListener('submit', function(e) {
  e.preventDefault();
  const formData = new FormData(form);
  for (const pair of formData.entries()) {
    console.log(pair);
  }
});
*/
