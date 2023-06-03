var gateway = `ws://${window.location.hostname}/ws`;
var websocket;

// init --------

window.addEventListener('load', onLoad);

function onLoad() {
    initWebSocket();
    initValveTimesTable();
}

// web socket handleing

function initWebSocket() {
    console.log('trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

function onMessage(event) {
    console.log(`Received a notification from ${event.origin}`);
    console.log(event);
}

// handle valve times on load -------------

function initValveTimesTable() {

}
