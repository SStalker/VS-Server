//create a new websocket
var url = "127.0.0.1";
var urll = "192.168.2.107";
var userid;
var sessionid;

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
	console.log('WebSocket Error');
};

//receive messages from server
websocket.onmessage = function(event){
	//process the received server data
	processMessage(event.data);
};

//Set Connection Status
function setStatus(){
	if(websocketOk === true ){
		$("#insert-status").html("Mit Server Verbunden!");
	}else{
		$("#insert-status").html("Nicht mit Server Verbunden!");
	}
};

function setStatusMsg(status){
	setStatus();
	$("#insert-status").append("<span class=\"alert\">"+ status +"</span> ");
}

//Send a message
function sendMessage(msg){
	if(websocketOk){
		websocket.send(JSON.stringify(msg));
	}else{
		//Error -->  do something
		alert("Keine Verbindung zum Server");
	}
};

//function to process received data
function processMessage(data){
	//do something on the data....
	var dataArray = JSON.parse(data);
	//get the message type
	if(dataArray.response !== 'undefined'){
		var type = dataArray.response;
		console.log(dataArray);
		 if(type === "registration"){
			console.log(dataArray.values.message);
			if(dataArray.values.message === "success"){
				//Load Login page and set status
				loadLogin();
				setStatusMsg("Erfolgreich Registriert");
			}
		}else if(type === "login"){
			if(dataArray.values.login === "failed"){
				setStatusMsg("Login fehlgeschlagen, E-Mail Adresse und/oder Passwort überprüfen!");
			}else if(dataArray.values.login === "success"){
				userid = dataArray.values.uid;
				sessionid = dataArray.values.sid;
				//remove comment when template is complete
				$(".wrapper").html(dataArray.values.template);
			}
		}else if(type === "registersite"){
			//console.log(dataArray.values.message);
			$(".wrapper").html(dataArray.values.message);
		}
	}else{
		console.log(dataArray);
	}

};

function confirmPW(password, confirm){
	if(password.val() === confirm.val() && password.val()!== "" && password.val().toString().length >= 6){
		setStatus();
		return true;
	}else{
		setStatusMsg("Passwort zu kurz");
		return false;
	}
};

function loadLogin(){
	$(".wrapper").load("webclient-templates/login.html", function( response, status, xhr ) {
		if ( status == "error" ) {
		    	console.log("Sorry but there was an error: "+ xhr.status + " " + xhr.statusText );
			$(".wrapper").html("<div class=\"error\">\n" 			+
						"<h3>Konnte Template nicht laden</h3>\n"+
						"<a href=\"newIndex.html\">Zurück</a>"	+
						"</div>");
		}
	})
}

$(document).ready(function(){
	console.log("doc-ready");
	$(".wrapper").on('blur',"#confirm-password",function(){

		if(confirmPW($("#password"),$("#confirm-password"))){
			console.log("true");
		}else{
			console.log("false");
		}
	});
	//Funktion to seriallize login form data
	//ToDo: Add encryption to password and define the send message in documentation
	$(".wrapper").on('submit',"#registration-form",function(event){
		if(confirmPW($("#password"),$("#confirm-password"))){
			var data = {
				request: "registration",
				values: {
					webclient: true,
				}
			};

			$(this).serializeArray().map(function(x){
				if(x.name !== "confirm-password"){
					data.values[x.name] = x.value;
				}else{
					console.log("Ignored: confirm-password");
				}
			});

			//console.log( JSON.stringify(data));
			sendMessage(data);
		}else{
			setStatusMsg("Passwort stimmt nicht ueberein");
		}
		event.preventDefault();
	});

	$('.wrapper').on('submit',"#login", function(event){
		var logindata = {
			request: "login",
			values: {
				webclient: true,
			}
		};

		$(this).serializeArray().map(function(x){
			logindata.values[x.name] = x.value;
		});

		//console.log( JSON.stringify(logindata));
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
		loadLogin();
	});

	$('.wrapper').on('click', '#logout', function(){
		console.log("Logout");
		var data = {
			request: "logout",
			values: {
				uid: userid,
				sid: sessionid
			}
		};
		sendMessage(data);
		console.log(data);
		//reload page and / or reset ws
	});
});


