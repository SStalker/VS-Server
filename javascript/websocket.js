//create a new websocket
var url = "127.0.0.1";
var urll = "192.168.2.107";

//var websocket = new WebSocket("ws://iknowit.ddns.net:1919");
//var websocket = new WebSocket("ws://192.168.178.31:1919");
var websocket = new WebSocket("ws://"+ url +":1919");
var websocketOk;
websocket.onopen = function(event){
	websocketOk = true;
	setStatus();
	//DEBUGG
	console.log("open");
};

websocket.onerror = function (error) {

	//If the websocket is closed due some issues set websocketOk to false
	websocketOk = false;
	setStatus();
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

//Set Connection Status
setStatus = function(){
	if(websocketOk === true ){
		$("#insert-status").html("Mit Server Verbunden!");
	}else{
		$("#insert-status").html("Nicht mit Server Verbunden!");
	}
};

//Send a message
sendMessage = function(msg){
	if(websocketOk){
		websocket.send(JSON.stringify(msg));
	}else{
		//Error -->  do something
		console.log("no open websocket connection");
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
			console.log(dataArray.Value1.message);
			$(".wrapper").html(dataArray.Value1.message);
		}
	}
};

$(document).ready(function(){
	console.log("doc-ready");
	$(".wrapper").on('blur',"#confirm-password",function(){
		if($("#password").val() === $("#confirm-password").val() && !$(password).val()){
			console.log("test");
		}else{
			console.log("false");
		}
	});
	//Funktion to seriallize login form data
	//ToDo: Add encryption to password and define the send message in documentation
	$(".wrapper").on('submit',"#registration",function(event){
		var data = {
			request: "registration",
			values: {
			}
		};

		$(this).serializeArray().map(function(x){
			if(x.name !== "confirm-password"){
				data.values[x.name] = x.value;
			}else{
				console.log("Ignored: confirm-password");
			}
		});

		console.log( JSON.stringify(logindata));
		sendMessage(logindata);
		event.preventDefault();
	});

	$('.wrapper').on('submit',"#login", function(event){
		var logindata = {
			request: "login",
			values: {
			}
		};

		$(this).serializeArray().map(function(x){logindata.values[x.name] = x.value;});

		console.log( JSON.stringify(logindata));
		sendMessage(logindata);
		event.preventDefault();
	});

	$('.wrapper').on('click', "#register" ,function(){
		console.log("Load template");
		$(".wrapper").load("webclient-templates/register.html", function( response, status, xhr ) {
			if ( status == "error" ) {
			    	console.log("Sorry but there was an error: "+ xhr.status + " " + xhr.statusText );
				$(".wrapper").html("<h3>Konnte Template nicht laden</h3>\n" +
							"<a href=\"newIndex.html\">Zurück</a>");
			}
		})
		/*
		var data = {
			request: "registersite"
		}
		sendMessage(data);
		*/
	});

	$('.wrapper').on('click', '#back',function(){
		console.log("clicked back");
		$(".wrapper").load("webclient-templates/login.html", function( response, status, xhr ) {
			if ( status == "error" ) {
			    	console.log("Sorry but there was an error: "+ xhr.status + " " + xhr.statusText );
				$(".wrapper").html("<div class=\"error\">\n" 			+
							"<h3>Konnte Template nicht laden</h3>\n"+
							"<a href=\"newIndex.html\">Zurück</a>"	+
							"</div>");
			}
		})
	});
});


