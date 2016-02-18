//create a new websocket
var url = "127.0.0.1";
var urll = "192.168.2.107";
var urlll = "iknowit.ddns.net";

//var websocket = new WebSocket("ws://iknowit.ddns.net:1919");
//var websocket = new WebSocket("ws://192.168.178.31:1919");
var websocket = new WebSocket("ws://"+ urlll +":1919");
var websocketOk;
var myChatModel;
var myResultModel;
var usermail;

console.log("doc-ready");

websocket.onopen = function(event){
    websocketOk = true;
    setStatus();
    //DEBUGG
   //console.log("open");
};

websocket.onerror = function (error) {

    //If the websocket is closed due some issues set websocketOk to false
    websocketOk = false;
    setStatus();
    //DEBUG
   //console.log('WebSocket Error');
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
        //console.log(dataArray);
         if(type === "registration"){
           //console.log(dataArray.values.message);
            if(dataArray.values.message === "success"){
                //Load Login page and set status
                loadLogin();
                setStatusMsg("Erfolgreich Registriert");
            }
        }else if(type === "login"){
            if(dataArray.values.login === "failed"){
                setStatusMsg("Login fehlgeschlagen, E-Mail Adresse und/oder Passwort überprüfen!");
            }else if(dataArray.values.login === "success"){
                $(".wrapper").html(dataArray.values.template);

                ko.applyBindings(myChatModel,$("#chatarea")[0]);
                ko.applyBindings(myResultModel,$("#searchFriends")[0]) ;
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

            myResultModel.users.removeAll();
            $.each(dataArray.values, function(){
                myResultModel.addUser($(this)[0]);
            });
            if(myResultModel.users().length < 1){
                $("#results").hide();
                setStatusMsg("Niemanden Gefunden");
            }else{
                $("#results").show();
                setStatus();
            }

        }else if(type === "addFriend"){
            if(dataArray.values.friendRequest === "success"){
                setStatusMsg("Freundesanfrage gessendet");
                myResultModel.users.removeAll();
            }
        }else if(type === "acceptFriend"){
            if(dataArray.values.acceptRequest === "success"){
               //console.log(dataArray.values.friendMail);
                myResultModel.deleteRequest(dataArray.values.friendMail);
            }
        }else if (type === "newMessage"){
            myChatModel.addMessageToChat(dataArray.values);
        }

    }else if(dataArray.pushmsg !== undefined){
       //console.log(dataArray);
        var pushtype = dataArray.pushmsg;
        if(pushtype === "friendRequest"){
           //console.log("friendRequest");
            $.each(dataArray.values, function(){
//				//console.log($(this)[0]);
                myResultModel.addRequest($(this)[0]);
            });
        }else if(pushtype === "friendlist"){
           //console.log(dataArray);
            $.each(dataArray.values, function(){
//				//console.log($(this)[0]);
                myChatModel.addFriend($(this)[0]);
            });
        }else if(pushtype === "newFriendListUser"){
            myChatModel.addFriend(dataArray.values[0]);
        }else if(pushtype === "notifyOnline"){
           //console.log(myChatModel.friends());
            $.each(myChatModel.friends(), function(){
                if($(this)[0].email() === dataArray.values.email){
                    if(dataArray.values.online === "true"){
                        $(this)[0].online(true);
                    }else{
                        $(this)[0].online(false);
                    }
                }
            });
        }else if(pushtype === "chatlist"){
            $.each(dataArray.values, function(){
                myChatModel.addChat($(this)[0]);
            });
        }else if(pushtype === "newchat"){
            $.each(dataArray.values, function(){
                myChatModel.addChat($(this)[0]);
            });
        }

    }else{
       //console.log(dataArray);
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
        if ( status === "error" ) {
               //console.log("Sorry but there was an error: "+ xhr.status + " " + xhr.statusText );
            $(".wrapper").html("<div class=\"error\">\n" 			+
                        "<h3>Konnte Template nicht laden</h3>\n"+
                        "<a href=\"newIndex.html\">Zurück</a>"	+
                        "</div>");
        }
    })
};

var ChatModel = function(){

    var self =this;

    self.friends= ko.observableArray();
    self.chats= ko.observableArray();


    self.addFriend = function(user){
        self.friends.push(new friend(user.nickname, user.email, user.online, user.firstname, user.lastname,
                    user.birthday, user.imageb64, user.cid, user.chatname, user.chatstatusmsg));
    };

    self.addChat = function(friendChat){
        var messages = [];
        $.each(friendChat.messages, function(){
            var own = usermail === this.messageFrom;
            var newmessage = new message(this.id, this.messageFrom, this.messageTo, this.message, this.created_at, own);
            messages.push(newmessage);
        });
        //foreach chat.messages
        // messages.push(new message(this.id, this.messageFrom, this.messageTo, this.message, this.created_at));


        //console.log(friendChat);
        //console.log(friendChat.messages);
        self.chats.push(new chat(friendChat.id, friendChat.name, friendChat.status, messages));
    };

    self.addMessageToChat = function(newMessage){

        var correctChat = ko.utils.arrayFirst(self.chats(), function(chat) {
           return chat.id === parseInt(newMessage.messageTo, 10);
        });
        var own = usermail === newMessage.messageFrom;
        correctChat.messages.push(new message(parseInt(newMessage.id, 10), newMessage.messageFrom, parseInt(newMessage.messageTo, 10), newMessage.message, newMessage.created_at, own));
    };

};

var ResultModel = function(){
    var self = this;
    self.users= ko.observableArray();
    self.requests= ko.observableArray();

    self.addUser= function(result){
        var user = {
            'nickname': result.nickname,
            'email':  result.email
        };
        self.users.push(user);
    };

    self.addRequest= function(result){
        var user = {
            'nickname': result.nickname,
            'email':  result.email
        };
        self.requests.push(user);
    };

    self.deleteRequest= function(email){
       //console.log("friendmail: " + email );
       //console.log(self.requests());
        requests.remove(function (user){
            return user.email === email;
        });
    };

};

//define friendsobject
function friend( nickname, email, online, firstname, lastname, birthday, image, cid, chatname, chatstatusmsg ){
    return{
        'nickname' : ko.observable(nickname) ,
        'email': ko.observable(email),
        'online': ko.observable(online),
        'firstname': ko.observable(firstname),
        'lastname': ko.observable(lastname),
        'birthday': ko.observable(birthday),
        'imageb64': ko.observable(image),
        'cid': ko.observable(cid),
        'chatname': ko.observable(chatname),
        'chatstatusmsg': ko.observable(chatstatusmsg)
    };
};

function message(id, messageFrom,messageTo, message, created_at, own){
    return{
        'id': id,
        'messageFrom': messageFrom,
        'messageTo': messageTo,
        'message': message,
        'created_at': created_at,
        'own': own
    };
};

function chat(id, name, status, messages){
    return{
        'id': id,
        'name': ko.observable(name),
        'status': ko.observable(status),
        'messages': ko.observableArray(messages)
    };
};


$(document).ready(function(){

    myChatModel = new ChatModel();
    myResultModel = new ResultModel();

	$(".wrapper").on('blur',"#confirm-password",function(){

		if(confirmPW($("#password"),$("#confirm-password"))){
            //console.log("true");
		}else{
            //console.log("false");
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
                    //console.log("Ignored: confirm-password");
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

        usermail = logindata.values.email;
       //console.log(usermail);
        //console.log( JSON.stringify(logindata));
		sendMessage(logindata);
		event.preventDefault();
	});

	$('.wrapper').on('click', "#register" ,function(){
        //console.log("Load template");
		$(".wrapper").load("webclient-templates/register.html", function( response, status, xhr ) {
            if ( status === "error" ) {
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
        //console.log(data);
		sendMessage(data);
	});

    $('.wrapper').on('submit', ".input", function(event){
        event.preventDefault();

        var data = {
            request: "newMessage",
            values: {
                messageFrom: usermail,
            }
        };

        $(this).serializeArray().map(function(x){
            data.values[x.name] = x.value;
        });

        //Clear Textarea
        $("#send_"+data.values.messageTo).val('');

        var tmp = data.values.messageTo;
        data.values.messageTo = parseInt(tmp,10);

        //console.log(data);
        sendMessage(data);
    });

	$('.wrapper').on('click', '#back',function(){
        //console.log("clicked back");
		loadLogin();
	});

	$(".wrapper").on('click', '#searchFriend', function(event){
        //console.log("friend");
		$("#chatarea").hide();
        $(".chatwindow").hide();
		$("#searchFriends").show();
		event.preventDefault();
	});

	$(".wrapper").on('click', '#toChatarea', function(event){
        //console.log("chat");
		$("#chatarea").show();
		$("#searchFriends").hide();
		event.preventDefault();
	});

    $(".wrapper").on('click', '.toChat', function(event){
        event.preventDefault();
       //console.log("tochat");
       //console.log($(this).attr("value"));
        var chatid = $(this).attr("value");
        $(".chatwindow").hide();
        $("#chat_"+chatid).show();
    });

	$('.wrapper').on('click', '#logout', function(event){
        //console.log("Logout");

		var data = {
			request: "logout",
			values: {

			}
		};
		sendMessage(data);
        //console.log(data);
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
        //console.log(data);
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
        //console.log(data);
		event.preventDefault();
	});

    $('.wrapper').on('click', '.decline', function(event){

        var friendMail = $(this).parent().find(".email").text();

        var data = {
            request: "acceptFriend",
            values: {
                friendMail: friendMail,
                answer: false
            }
        }
        sendMessage(data);
        //console.log(data);
        event.preventDefault();
    });


});

