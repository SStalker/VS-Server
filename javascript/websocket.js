

//create a new websocket
var url = "127.0.0.1";
var urll = "192.168.2.107";
var sessionid;

//var websocket = new WebSocket("ws://iknowit.ddns.net:1919");
//var websocket = new WebSocket("ws://192.168.178.31:1919");
var websocket = new WebSocket("ws://"+ url +":1919");
var websocketOk;
var ChatModel;
var ResultModel;



console.log("doc-ready");

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
	if(dataArray.response !== undefined){
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
				$(".wrapper").html(dataArray.values.template);

				ko.applyBindings(ChatModel,$("#chatarea")[0]);
				ko.applyBindings(ResultModel,$("#searchFriends")[0]) ;
			}
		}else if(type === "registersite"){
			//console.log(dataArray.values.message);
			$(".wrapper").html(dataArray.values.message);
		}else if(type === "logout"){
			if(dataArray.values.logout === "success"){
				alert("Erfolgeich ausgeloggt");
				websocket.close();
				window.location.reload(true);
			}
		}else if(type === "searchUser"){

			ResultModel.users.removeAll();
			$.each(dataArray.values, function(){
				ResultModel.addUser($(this)[0]);
			});
			if(ResultModel.users().length < 1){
				$("#results").hide();
				setStatusMsg("Niemanden Gefunden");
			}else{
				$("#results").show();
				setStatus();
			}

		}else if(type === "addFriend"){
			if(dataArray.values.friendRequest === "success"){
				setStatusMsg("Freundesanfrage gessendet");
				ResultModel.users.removeAll();
			}
		}else if(type === "acceptFriend"){
			if(dataArray.values.acceptRequest === "success"){
				console.log(dataArray.values.friendMail);
				ResultModel.deleteRequest(dataArray.values.friendMail);
			}
		}

	}else if(dataArray.pushmsg !== undefined){
		console.log(dataArray);
		var type = dataArray.pushmsg;
		if(type === "friendRequest"){
			console.log("friendRequest");
			$.each(dataArray.values, function(){
				console.log($(this)[0]);
				ResultModel.addRequest($(this)[0]);
			});
		}else if(type === "friendlist"){
			console.log(dataArray);
			$.each(dataArray.values, function(){
				console.log($(this)[0]);
				ChatModel.addFriend($(this)[0]);
			});
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

	ChatModel = {
		friends: ko.observableArray(),

		addFriend: function(user){
			var friend ={
				'nickname' : user.nickname ,
				'email': user.email,
				'online': user.online,
				'firstname': user.firstname,
				'lastname': user.lastname,
				'birthday': user.birthday,
				'imageb64': user.imageb64,
				'cid': user.cid,
				'chatname': user.chatname,
				'chatstatusmsg': user.chatstatusmsg
			}
			this.friends.push(friend);
		}
	}

	ResultModel = {
		users: ko.observableArray(),
		requests: ko.observableArray(),

		addUser: function(result){
			var user = {
				'nickname': result.nickname,
				'email':  result.email
			};
			this.users.push(user);
		},

		addRequest: function(result){
			var user = {
				'nickname': result.nickname,
				'email':  result.email
			};
			this.requests.push(user);
		},

		deleteRequest: function(email){
			console.log("friendmail: " + email );
			console.log(ResultModel.requests());
			requests.remove(function (user){
				return user.email === email;
			});
		}

	}

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

	});

	$('.wrapper').on('submit', "#searchFriend-form", function(event){
		event.preventDefault();

		var data = {
			request: "searchUser",
			values: {
			}
		};

		$(this).serializeArray().map(function(x){
			data.values[x.name] = x.value;
		});
		console.log(data);
		sendMessage(data);
	});

	$('.wrapper').on('click', '#back',function(){
		console.log("clicked back");
		loadLogin();
	});

	$(".wrapper").on('click', '#searchFriend', function(event){
		console.log("friend");
		$("#chatarea").hide();
		$("#searchFriends").show();
		event.preventDefault();
	});

	$(".wrapper").on('click', '#toChatarea', function(event){
		console.log("chat");
		$("#chatarea").show();
		$("#searchFriends").hide();
		event.preventDefault();
	});

	$('.wrapper').on('click', '#logout', function(event){
		console.log("Logout");

		var data = {
			request: "logout",
			values: {

			}
		};
		sendMessage(data);
		console.log(data);
		event.preventDefault();
	});

	$('.wrapper').on('click', '.add', function(event){

		var friendMail = $(this).parent().find(".email").text();

		var data = {
			request: "addFriend",
			values: {
				friendMail: friendMail
			}
		}
		sendMessage(data);
		console.log(data);
		event.preventDefault();
	});

	$('.wrapper').on('click', '.accept', function(event){

		var friendMail = $(this).parent().find(".email").text();

		var data = {
			request: "acceptFriend",
			values: {
				friendMail: friendMail,
				answer: true
			}
		}
		sendMessage(data);
		console.log(data);
		event.preventDefault();
	});



});

