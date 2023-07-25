var serverIP = "10.0.0.22"
var gateway = "ws://" + serverIP +"/ws";
var websocket;

var valveMaxNumber = 4;
var valveMaxTimesPerDay = 4;

window.addEventListener('load', onLoad);

// init on load ----------------------------------------------------------------

function onLoad(event) {
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
    initButton();
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
   // document.getElementById('led').className = data.status;

  // this is were the JSON is PROCCESSED >>> ...
  var action = data.action;
  if (action == "cfgValve") {
    console.log("setting the valve times");
    var valveID = data.valveID;
    if ((valveID == null) || (valveID < 0) || (valveID > (valveMaxNumber - 1))) {
      console.log("shoot! invalid valve STOP");
      return;

    } else { // valve is valid
      console.log("valveID: " + valveID);
      console.log("setting valve times");

      var startTimes = data.startTimes;
      var stopTimes = data.stopTimes;

      updateValveTimes(valveID, startTimes, stopTimes);
    }
  }
}

// get a valve's times from server
function requestValveTimesFromServer(valveID) {
  websocket.send(JSON.stringify({'action':'getValveCfg','valveID':valveID}));
}

// get ALL valve's times from server
// look! it runs the above function 4 times!
function requestAllValvesTimesFromServer() {
  for (var i = 0; i < valveMaxNumber; i++) {
    requestValveTimesFromServer(i);
  }
}

// init the 'refresh button'
function initButton() {
  document.getElementById('refresh').addEventListener('click', () => requestValveTimesFromServer(0));
}

// update the gui values
function updateValveTimes(valveID, startTimes, stopTimes) {
  console.log("in function updateValveTimes()");

  // illiterate through the valve times in day, setting the gui
  for (var thisTime = 0; thisTime < valveMaxTimesPerDay; thisTime++) {

    // used to find the right valve times in the gui
    var valveClassName = "valve" + valveID;
    console.log(valveClassName);

    // the string showing the valve times ("12:00 - 23:59")
    var valveStartStopTimeStr;

    // if start and stop time are both at 00:00, the time is unused
    if ((startTimes[thisTime] === "00:00") && (stopTimes[thisTime] === "00:00")) {
     valveStartStopTimeStr = "unused";
    } else {
     valveStartStopTimeStr = "" + startTimes[thisTime] + " - " + stopTimes[thisTime];
    }

    console.log(valveStartStopTimeStr);

    // I can't think of a good name to call the variable... x isn't intuitive. But that's why you have a BRAIN
    var x = document.getElementsByClassName(valveClassName);
    x.item(thisTime).innerHTML = valveStartStopTimeStr;
  }
}
