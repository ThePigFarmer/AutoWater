console.log("starting app.js...");

// check times for clicks
window.addEventListener("DOMContentLoaded", function () {
    let boxes = document.querySelectorAll(".valve");

    Array.from(boxes, function (box) {
        box.addEventListener("click", function () {
            var valveNum = this.classList[1];
            var valveNumInDay = this.id;
            console.log("clicked: valve " + valveNum + ",   id " + valveNumInDay);
            openDurationEntryForm(valveNum, valveNumInDay);
        });
    });
});