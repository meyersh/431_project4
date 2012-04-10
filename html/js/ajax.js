/*
 * AJAX library and helper functions for CSCI-170. 
 * Boilerplate by Randy, etc by Shaun.
 * Mar, 2010
 */

var INFO    = 0;
var WARNING = 1;
var ERROR   = 2;
var SUCCESS = 3;

// define the messages types 
var myMessages = ['info','warning','error','success']; 

var dir = {
    "W"  : 0,
    "NW" : 1,
    "N"  : 2,
    "NE" : 3,
    "E"  : 4,
    "SE" : 5,
    "S"  : 6,
    "SW" : 7
};

var board_size = 19;

function ge(id) {
	return document.getElementById(id);
}

function getXMLHttpReq()
{ 
    var req = null;

    if (window.XMLHttpRequest)
		req = new XMLHttpRequest();
    else if (window.ActiveXObject)
		req = new ActiveXObject(Microsoft.XMLHTTP);
    return req;
}

function hideAllMessages()
{
    var messagesHeights = new Array(); // this array will store height for each
    
    for (i=0; i<myMessages.length; i++)
    {
	messagesHeights[i] = $('.' + myMessages[i]).outerHeight();
	$('.' + myMessages[i]).css('top', -messagesHeights[i]); //move element outside viewport	  
    }
}

function initMessage(type)
{
    $('.'+ type +'-trigger').click(function(){
	hideAllMessages();				  
	$('.'+type).animate({top:"0"}, 500);
    });
}

function showMessageDiv(type)
{
	//hideAllMessages();
	$('.'+type).animate({top:"0"}, 500);
}

function showMessage(type, header, message) {
	ge(type).innerHTML = '<h3>' + header + '</h3>\n' + '<p>' + message + '</p>\n';
	showMessageDiv(type);
}

function showTurn(ourturn) {
	if (ourturn == window.ourTurn)
		return; // Do nothing if there is no change

	window.ourTurn = ourturn;

	if (window.ourTurn == true) {
		showMessage('info', "Your Turn!", 
					"It's your turn to move.");
	}

	else {
		showMessage('info', 'Waiting for the other player',
					"It's the other players move. We're waiting on them");
	}
	
	$('#info').effect("pulsate", {times: 2}, 1500);

	
}


/*
 * I think I'll need to be urlEncoding my output from here on out since
 * I want to be able to do more advanced things and not worry about my =
 * signs or html or etc...
 * thank you http://www.albionresearch.com/
 */

function urlDecode(encoded)
{
    if (encoded == null)
		return null;

    var plaintext = "";
    var HEXCHARS = "0123456789ABCDEFabcdef";
    var i = 0;
    for (i=0; i < encoded.length; i++)
    {
		var ch = encoded.charAt(i);
		if (ch == "+") {
			plaintext += " ";
			//i++;
		}
		else if (ch == "%") {
			if (i < (encoded.length-2) 
				&& HEXCHARS.indexOf(encoded.charAt(i+1)) != -1
				&& HEXCHARS.indexOf(encoded.charAt(i+2)) != -1 ) {
				plaintext += unescape( encoded.substr(i,3) );
				i += 2;
			}
		}
		else {
			plaintext += ch;
			//i++;
		}
		
    } // while
    return plaintext;
}

/*
 * updateLog(html) - append some nice data to the log area
 * (default: responseSpot) and keep the div scrolled to the 
 * bottom or latest text.
 */

function updateLog(html, outputDivId, append)
{
    /* These are our default values: */

    /* default div Id: 'responseSpot' */
    var outputDivId = (outputDivId == null) ? 'responseSpot' : outputDivId;
    /* default: should we append? TRUE */
    var append = (append == null) ? true : false;

    divObj = document.getElementById(outputDivId);
    if (append)
    {
		divObj.innerHTML += html;
		divObj.scrollTop = divObj.scrollHeight;
    }
    else
    { 
		divObj.innerHTML = html;
    }
}

function raiseNotice ( message )
{
    var divObj = document.getElementById("notice");
    divObj.className = "system-message";
    divObj.innerHTML = message;
}

function lowerNotice()
{
    var divObj = document.getElementById("notice");
    if (divObj)
    {
		divObj.className = "";
		divObj.innerHTML = "";
    }
}


function raiseError( message)
{
    var divObj = document.getElementById("error");
    divObj.className = "errornote";
    divObj.innerHTML = message;
}

function lowerError()
{
    var divObj = document.getElementById("error");
    if (divObj) 
    {
        divObj.className = "";
        divObj.innerHTML = "";
    }
}

/* 
 * I'm trying something new, returning all my values as 
 * key=value which I intend to parse with string.split
 * method and set in the appropriate divs... Here goes.
 */

function parseData(data) 
{
    var temp = new Array();
    var lines = new Array();

 	lines = data.split('\n');
	var message = lines[0];

	// trim 'empty' lines
	for (var i = 0; i < lines.length; i++)
		if (lines[i] == '')
			lines.splice(i,1);
	

	if (message == "ERROR_CAUSED_SHUTDOWN") 
		showMessage('error', 'ERROR_CAUSED_SHUTDOWN', lines[1]);

	if (message == "SETUP") {
		if (lines.length < 6) // abort for error.
		{
			return showMessage('error', 'CGI API VIOLATION', 
							   'SETUP gave invalid response:<br><pre>'+
							   data + '</pre>');
		}

		showMessage('info', 'SETUP', 'Gameid: ' + lines[1] + '<br>Sessionid: ' + lines[2]);

		// Update UI to show the game screen
		$('#accordion').accordion( "activate" , 1 );
		$('#gameid').val(lines[1]);
		$('#sessionid').val(lines[2]);

        // if this is an hc1 game, we get the computer's move from this response.
        if (lines.length == 8) {
            var coords = lines[5] + ' ' + lines[6];
            var computer_move = $('input[name="' + coords + '"]')[0];
            place_piece(computer_move, "BLACK");
        }

		// Start auto-refresh (refreshing every 3 seconds)
		window.auto_check = setInterval("check_for_move();", 3000); 

		window.joined = true;

        // Display some turn informations.
        if (lines[lines.length-1] == "MOVE")
            showTurn(true);
        else
            showTurn(false);
	}

	if (message == "JOIN") {
		
		if (lines.length != 3 && lines.length != 7) // abort for error
		{
			return showMessage('error', 'CGI API VIOLATION',
							   'JOIN gave invalid response:<br><pre>' +
							   data + '</pre>');
		}

		if (lines[2] == 'GAME_UNDERWAY')
			return showMessage('warning', 'GAME_UNDERWAY', 
							   'Server returned GAME_UNDERWAY error when joining ' +
							   lines[1]);

		showMessage('info', 'JOIN', 'Gameid: ' + lines[1] + '<br>Sessionid: ' + lines[2]);

        /* We now get hh1/hh2 information.
           JOIN\n<gameid>\n<sessionid>\nhh1\n9\n9\nMOVE	You've joined as player 2
           JOIN\n<gameid>\n<sessionid>\nhh2\n9\n9\nWAITING	You've joined as player 1
        */
        
        if (lines[3] == "hh1") // we're player2, as we've just joined a game started as hh1
            place_piece($("input[name='9 9']")[0], 'BLACK');

        else if (lines[3] == "hh2") // we're player1 having just joined a game started w/ hh2
            place_piece($("input[name='9 9']")[0], 'WHITE');

        // update gameid + sessionid text boxes.
		$('#gameid').val(lines[1]);
		$('#sessionid').val(lines[2]);

		// Start auto-refresh (refreshing every 3 seconds)
		window.auto_check = setInterval("check_for_move();", 3000); 

	}		

	if (message == "MOVE") {
		/* 
		   MOVE\n<gameid>\n<sessionid>
		   MOVE\n<gameid>\n<sessionid>\nWIN
		*/
		if (lines.length != 3 && lines.length != 4)
			return showMessage('error', 'CGI API VIOLATION',
							   'MOVE game invalid response:<br><pre>' + 
							   data + '</pre>');

		// update the UI with the new piece
		if (window.last_piece) {
			place_piece(window.last_piece, 'WHITE');
            chkCaptures(window.last_piece);

			window.last_piece = null;
		}
		else 
			return showMessage('error', 'Something weird happened!',
							   'This should normally never happen, but' + 
							   'we seemed to have lost your playing piece.');

		if (lines.length == 4 && lines[3] == "WIN") {
            clearInterval(window.auto_check); // stop refreshing.
			return showMessage('success', 'Victory',
							   'Victory is yours!');
        }
	}

	if (message == "CHECK") {
		/*
		  Waiting on them:
		  CHECK\n<gameid>\n<sessionid>\nWAITING

		  Their move was:
		  CHECK\n<gameid>\n<sessionid>\n<row>\n<col>

		  Their move was (and they won/lost)
		  CHECK\n<gameid>\n<sessionid>\n<row>\n<col>\n{WIN|LOSE}

		  Timeout (row + col don't matter).
		  CHECK\n<gameid>\n<sessionid>\n<row>\n<col>\nTIMEOUT
		*/

		if (lines.length < 3 || lines.length > 6) {
            alert(data);
			return showMessage('error', 'CGI API VIOLATION',
							   'CHECK game invalid response:<br><pre>' + 
							   data + '</pre>');
        }

		var gameid    = lines[1];
		var sessionid = lines[2];

		$('#sessionid').val(sessionid); // update the sessionid

		if (lines[3] == "WAITING") {
			showTurn(false);
			return; // nothing to do here.
		}
		
		var row = lines[3];
		var col = lines[4];

		var name = row + ' ' + col;
		// place a move at button NAME. 
		place_piece($('input[name="' + name + '"]')[0], 'BLACK');
		chkCaptures($('input[name="' + name + '"]')[0]);
		// Update the UI to show that it is our turn
		showTurn(true);

		// Game is over or invalid.
		if (lines.length == 6) {
            window.clearInterval(window.auto_check);
            if (lines[5] == "TIMEOUT")
                showMessage('error', 'GAME TIMEOUT', 
                            'The game has timed out. You took too long strategizing.');
            else if (lines[5] == "WIN")
                showMessage('success', 'Victory',
							   'Victory is yours!');
            else if (lines[5] == "LOSE")
                showMessage('warning', 'Loser detected',
                            'You have lost this game.');

		}

	}
	
}

function sendData(dataStr, url, method) { 
    var req = getXMLHttpReq();

    if (req == null)
		return 1;

    /* if we are using the GET method we need to append the data
       to the URL after a ?-mark. */

    if ( method == "GET" ) 
		url += '?' + dataStr.replace(/\n/g, '&');

    req.open(method, url, true);
    
    req.onreadystatechange = function()
    {
		if(req.readyState == 4)
		{
			if(req.status == 200)
			{
				parseData(req.responseText);
				updateLog("<br>Data sent/recieved from server!<br>--");
			}
			else
				updateLog("Error Status: " 
						  + req.status 
						  + "<br />Error Description: " 
						  + req.statusText);
		}
    }


    req.setRequestHeader("Content-Type", "text/plain");

    if (method == "POST")
		req.send(dataStr);
    else if (method == "GET")
		req.send(dataStr);

    return 0;
}

/* 
 * Shaun's "clear the box if it's at the default" function...
 * extra awesomeness, if it's blank reset the box to its default
 * "boxObj" is gotten from `this` in the onFocus="" method.
 * "setfocus" is 1 for onFocus and 0 for onBlur so we know if 
 * a textbox has focus or no (so we know if we're coming or going)
 */

function checkDefault(boxObj, setfocus)
{
    boxObj.hasFocus = setfocus;

    if (boxObj.hasFocus && boxObj.value ==  boxObj.defaultValue) 
        boxObj.value = "";
    else if (!boxObj.hasFocus && boxObj.value == "")
        boxObj.value = boxObj.defaultValue;
}

/*
 * A bit of a useless function, if t (object with value) has or doesn't
 * have the correct value, does something. 
 * The idea sort of fizzled out when I realized how difficult this is to
 * check with onKeyUp or onEvent stuffies.
 */

function isDigit( t )
{
    if (isNaN(parseInt(t.value)) && t.value != "") {
        alert("Integers only! Bad Human, Bad!");
        t.value = 0;
    }
}

function exchangeData( boxObj )
{
    var uploadstring = boxObj.value;
    sendData(uploadstring, 'http://gimli.morningside.edu/~meyersh/cgi-bin/S8.E1.1.cgi');
}

/*
 * Allows two submit buttons that can change a Forms submit method
 */

function modSubmit(formid, method) 
{
    formid = document.getElementById(formid);
    formid.method = method;
    //document.getElementById('inputFormSubmit').value = 'Submit ' + method;
    formid.submit();
}

function Submit(formid) 
{
    var frm = document.getElementById(formid);
    var method = frm.method;
    var url = frm.action;
    var inpt = frm.input.value;
    sendData(inpt, url, method);
}

// ** HANDLE SUBMIT CODES **
function create_game() { 
    var player_config_select = document.getElementById("player_configuration");
    var selected = player_config_select.selectedIndex;
    var player_config = player_config_select[ selected ].value; // one of hh1,hh2,hc1,hc2.

    var post_text = "SETUP\n" + player_config;

    sendData(post_text, cgi_url, "POST");


	// place the initial piece.
    var first_color = (player_config[2] == '1' ? 'WHITE' : 'BLACK');
	place_piece($('input[name="9 9"]')[0], first_color);

}

function join_game() {
	if (window.joined) {
		showMessage("info", "You are already joined to this game.", 
				   "Why would you want to join again?");
		return;
	}
	
	if ($('#gameid')[0].value == $('#gameid')[0].title)
		return showMessage('error', 'Invalid Gameid', 
					'You must set a valid gameid to join a game.')

	var post_text = "JOIN\n" + $('#gameid').val() + "\n";

	sendData(post_text, cgi_url, "POST");

}

function status_of_button(button) {
	if (button.src.match(/\/([0-9]).gif/))
		return "EMPTY";
	else if (button.src.match(/\/w([0-9]).gif/))
		return "WHITE";
	else if (button.src.match(/\/b([0-9]).gif/))
		return "BLACK";
}

function get_neighbor(button, direction) {
    var row, col;

    if (button == null)
        return null;

    row = parseInt(button.name.split(' ')[0]);
    col = parseInt(button.name.split(' ')[1]);

    var neighbor;

    if (direction < 0 || direction > 7)
        return null;

    switch (direction) {
    case dir.W:
        neighbor = new Array(row, col-1);
        break;
    case dir.NW:
        neighbor = new Array(row-1, col-1);
        break;
    case dir.N:
        neighbor = new Array(row-1, col);
        break;
    case dir.NE:
        neighbor = new Array(row-1, col+1);
        break;
    case dir.E:
        neighbor = new Array(row, col+1);
        break;
    case dir.SE:
        neighbor = new Array(row+1, col+1);
        break;
    case dir.S:
        neighbor = new Array(row+1, col);
        break;
    case dir.SW:
        neighbor = new Array(row+1, col-1);
    }
    
    for (var i = 0; i < 2; i++)
        if (neighbor[i] < 0 || neighbor[i] > board_size)
            return null;

    var name = neighbor[0] + ' ' + neighbor[1];

    return $('input[name="' + name + '"]')[0];
}

function place_piece(button, color) {
    
	if (color == 'WHITE')
		button.src = button.src.replace(/\/([0-9]).gif/, '\/w$1.gif');
	else if (color == 'BLACK')
		button.src = button.src.replace(/\/([0-9]).gif/, '\/b$1.gif');
    else if (color == 'EMPTY')
        button.src = button.src.replace(/\/[wb]([0-9]).gif/, '\/$1.gif');

}
function opposite_color(color) {
    if (color == 'WHITE')
        return 'BLACK';
    else if (color == 'BLACK')
        return 'WHITE';
    else 
        return null;
}

function add_capture_to_table(color) {
    var element;
    var black_piece = "images/b4.gif";
    var white_piece = "images/w4.gif";
    
    if (color == 'WHITE') {
        var html = "<img src='" + black_piece + "'/>";
        $("#white_captures").append(html + html + "<br/>");
    }
    else if (color == 'BLACK') {
        var html = "<img src='" + white_piece + "'/>";
        $("#black_captures").append(html + html + "<br/>");
    }
}

function chkCaptures(button) {
    /* Logistically, this function will act just like its C++ cousin.
       Given a button, it will check all cardinal directions and remove
       any new captures as detected. In doing so, it will also update
       the white/black captures score in the UI. */

    for (var d = 0; d < 9; d++) {
        var middlea = get_neighbor(button, d);
        var middleb = get_neighbor(middlea, d);

        var end = get_neighbor(middleb, d);

        // skip empty sets.
        if (!end || !middlea || !middleb)
            continue; 

        // Have we found a capture?
        if (status_of_button(end) == status_of_button(button)
            && (status_of_button(middlea) == status_of_button(middleb)) 
            && (status_of_button(middleb) == opposite_color(status_of_button(end))))
        {
            // Remove the middle pieces
            place_piece(middlea, 'EMPTY');
            place_piece(middleb, 'EMPTY');

            // add them to the score
            add_capture_to_table( status_of_button(button) );
        }


    }

}

function check_for_move() {
	// Prompt the server to tell us about new moves.
	/*
	  POST: 
	  CHECK\n<gameid>\n<sessionid>
	 */

	var post_text = "CHECK\n" + 
		$('#gameid').val() + '\n' +
		$('#sessionid').val() + '\n';

	sendData(post_text, cgi_url, 'POST');
}

function make_move(button) {
	var coords = button.name;

	/* 
	   src looks like this: 

	   http://gimli.morningside.edu/~meyersh/431/pente/images/4.gif
	   
	   we want to make it seem like this: 
	   
	   http://gimli.morningside.edu/~meyersh/431/pente/images/b4.gif
	*/

	if (status_of_button(button) != "EMPTY")
		return showMessage('warning', 'Move violation [Click to Dismiss]', 
					'You can hardly expect to move <i>there</i>! '+ 
					'Are you trying to cheat or what?');

	// Do a check to make sure we haven't moved in the interrim.

	check_for_move();

	// everything is OK so go ahead and send the ajax move.
    // if+when MOVE comes back from API we'll place the piece visually.
	var row = button.name.split(' ')[0];
	var col = button.name.split(' ')[1];
	var post_text = "MOVE\n" 
		+ $('#gameid').val() + '\n' 
		+ $('#sessionid').val() + '\n'
		+ row + '\n' 
		+ col + '\n';
	sendData(post_text, cgi_url, 'POST');
	window.last_piece = button;

}




//
// EOF
//
