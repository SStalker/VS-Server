//create a new websocket
var url = "127.0.0.1";
var urll = "192.168.2.107";

//var websocket = new WebSocket("ws://iknowit.ddns.net:1919");
//var websocket = new WebSocket("ws://192.168.178.31:1919");
var websocket = new WebSocket("ws://"+ url +":1919");
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
	if(dataArray.response !== 'undefined'){
		var type = dataArray.response;
		console.log(dataArray);
		if(type === "registersite"){
			console.log(dataArray.message);
			//$(".wrapper").html(dataArray.message);
		}
	}
};

$(document).ready(function(){
	console.log("doc-ready");

	//Funktion to seriallize login form data
	//ToDo: Add encryption to password and define the send message in documentation
	$("#login").submit(function(event){
		var logindata = {
			request: "login",
		};
		$(this).serializeArray().map(function(x){logindata[x.name] = x.value;});

		console.log( JSON.stringify(logindata));
		sendMessage(logindata);
		event.preventDefault();
	});

	$("#register").click(function(){
		console.log("Load template");
		/*$(".wrapper").load("webclient-templates/register.html", function( response, status, xhr ) {
			if ( status == "error" ) {
			    	console.log("Sorry but there was an error: "+ xhr.status + " " + xhr.statusText );
			}
		})*/
		var data = {
			request: "registersite"
		}

		sendMessage(data);
	});
});


