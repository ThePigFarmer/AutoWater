// var serverIP = `${window.location.hostname}`
var serverIP = "10.0.0.150";
var gateway = "ws://" + serverIP +"/ws";
var websocket;

var valveMaxNumber = 4;
var valveMaxTimesPerDay = 4;

var activeValveID;
var activeTimeInDay;

function setActiveTimeElement(valveID, timeInDay) {
  activeValveID = valveID;
  activeTimeInDay = timeInDay;
}

window.addEventListener('load', onLoad);

// init on load ----------------------------------------------------------------

function onLoad(event) {
    initButtons();
    initForm();
    initValveSelector();
    initWebSocket();
}

// websocket stuff -------------------------------------------------------------

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
    getValvesTimesFromServer();
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
    let data = JSON.parse(event.data);
    handleJson(data);
}

function handleJson(data)
{
   console.log("JSON from server -------------------------------------------------------");
   console.log(data);
   console.log("------------------------------------------------------------------------");

  // parse json
  var action = data.action;
  if (action == "cfgValve") {
    console.log("setting the valve times");
    var valveID = data.valveID;
    if ((valveID == null) || (valveID < 0) || (valveID > (valveMaxNumber - 1))) {
      console.log("invalid valve");
      return;

    } else { // valve is valid
      console.log("valveID: " + valveID);
      console.log("setting valve times");

      var startTimes = data.startTimes;
      var stopTimes = data.stopTimes;

      updateValveTimes(valveID, startTimes, stopTimes);
    }
  }

  /*
  if (action == "rtcTime") {
    var timeStr = data.timeStr;
    document.getElementById("rtc-time").innerHTML = timeStr;
  }
  */
}

// get a valve's times from server
function getValveTimesFromServer(valveID) {
  websocket.send(JSON.stringify({'action':'getValveCfg','valveID':valveID}));
}

// get ALL valve's times from server
function getValvesTimesFromServer() {
  for (var i = 0; i < valveMaxNumber; i++) {
    getValveTimesFromServer(i);
  }
}

// init the buttons
function initButtons() {
  document.getElementById('refresh').addEventListener('click', () => {
    getValvesTimesFromServer();
  });
  document.getElementById('commit').addEventListener('click', () => {
    commitValveTimesToEEPROM();
  })
}

// update the gui values
function updateValveTimes(valveID, startTimes, stopTimes) {
  console.log("in function updateValveTimes()");

  // illiterate through the valve times in day, setting the gui
  for (var thisTime = 0; thisTime < valveMaxTimesPerDay; thisTime++) {

    // the string showing the valve times ("12:00 - 23:59")
    var valveStartStopTimeStr;

    // if start and stop time are both at 00:00, the time is unused
    if ((startTimes[thisTime] === "00:00") && (stopTimes[thisTime] === "00:00")) {
     valveStartStopTimeStr = "unused";
    } else {
     valveStartStopTimeStr = "" + startTimes[thisTime] + " - " + stopTimes[thisTime];
    }

    // console.log(valveStartStopTimeStr);

    var thisElement = valveID * valveMaxNumber + thisTime;

    // I can't think of a good name to call the variable... x isn't intuitive. But that's why you have a BRAIN
    var x = document.querySelectorAll(".time");
    console.log(thisElement);
    x.item(thisElement).innerHTML = valveStartStopTimeStr;
  }
}

function sendSetTimesFromInputToServer(valveID, timeInDay, startTime, stopTime) {
  console.log("valveID: " + valveID);
  websocket.send(JSON.stringify({'action':'setValveCfg','valveID':valveID, 'timeInDay':timeInDay, 'startTime':startTime, 'stopTime': stopTime}));
}

function initValveSelector() {
  let boxes = document.querySelectorAll(".time");

  Array.from(boxes, function(box) {
    box.addEventListener("click", function() {
      var elementIndexStr = this.classList[1];
      var elementIndex = Number(elementIndexStr);
      console.log("elementIndex: " + elementIndex);

      var xy = computeXYFromIndex(elementIndex);

      setActiveTimeElement(xy[0], xy[1]);
      console.log("active element: (" + xy[0] + ", " + xy[1] + ")");
    });
  });
}


function initForm() {
  const form = document.querySelector('#duration-entry-form');
  form.addEventListener('submit', (e) => {
    e.preventDefault();

    console.log("click: submit");
    var startTime = form.querySelector('#start-time').value;
    var stopTime = form.querySelector('#stop-time').value;

    console.log(activeValveID + ", " + activeTimeInDay + ", " + startTime + ", " + stopTime);
    sendSetTimesFromInputToServer(activeValveID, activeTimeInDay, startTime, stopTime);
  });
}

function commitValveTimesToEEPROM()
{
  websocket.send(JSON.stringify({'action':'commitValveTimes'}));
}

function computeXYFromIndex(i) {
  var x = Math.floor(i / 4);
  var y = i % 4;
  return [x, y];
}
