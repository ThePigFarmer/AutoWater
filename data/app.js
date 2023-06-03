console.log("starting js...");

window.addEventListener("DOMContentLoaded", function() {
    let boxes = document.querySelectorAll(".box");

    Array.from(boxes, function(box) {
        box.addEventListener("click", function () {
            alert(this.classList[1]);
            console.log(this.classList[1]);
        });
    });
});