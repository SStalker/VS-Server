//create a new websocket
var websocket = new WebSocket("ws://iknowit.ddns.net:1919");

websocket.onopen = function(event){
	console.log("open");
};

websocket.onerror = function (error) {
	console.log('WebSocket Error ' + error);
};

$(document).ready(function(){

	console.log("doc-ready");

});

