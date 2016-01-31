//create a new websocket
var websocket = new WebSocket("ws://iknowit.ddns.net:1919");
var websocketOk;
websocket.onopen = function(event){
	websocketOk = true;
	//DEBUGG
	console.log("open");
};

websocket.onerror = function (error) {

	//If the websocket is closed due some issues set websocketOk to false
	//DEBUGG
	console.log('WebSocket Error ' + error);
};

//receive messages from server
websocket.onmessage = function(event){
	//process the received server data
	processMessage(event.data);
	//DEBUGG
	console.log(event.data);
};


//Send a message
sendMessage = function(msg){
	if(websocketOk){
		websocket.send(JSON.stringify(msg));
	}else{
		//Error -->  do something
	}
};

//function to process received data
processMessage = function(data){
	//do something on the data....
	var dataArray = JSON.parse(data);
	//get the message type
	var type = dataArray.type;
	if(type === "hmtl"){
		//insert html to its target
	}
};

$(document).ready(function(){
	console.log("doc-ready");

	//Funktion to seriallize login form data
	//ToDo: Add encryption to password and define the send message in documentation
	$("form").submit(function(event){
		var logindata = {
			type: "logindata",
			date: Date.now(),
		};
		$(this).serializeArray().map(function(x){logindata[x.name] = x.value;});
		console.log( JSON.stringify(logindata));

		//sendMessage(logindata);

		event.preventDefault();
	});
});


