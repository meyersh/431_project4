
var cgi_url = "http://gimli.morningside.edu/~meyersh/431/pente/project3.cgi";

function draw_board() {
    var board_size = 19;
    var board_elem = document.getElementById('board');
    var board = ""

    board = '<table class="board" cellpadding=0 cellspacing=0 border=0>\n';

    for (var y = 0; y < board_size; y++) {

	board += "<tr>\n";

	for (var x = 0; x < board_size; x++) {
	    var img = "";
	    var name = y + ' ' + x;

	    
	    if (y == 0 && x==0)
		img = "0.gif";
	    else if (x == (board_size-1)/2 && y == (board_size-1)/2)
		img = "9.gif";
	    else if (y == 0 && x < board_size-1)
		img = "1.gif";
	    else if (y == 0 && x < board_size)
		img = "2.gif";
	    else if (y < board_size-1 && x == 0)
		img = "3.gif";
	    else if (y < board_size-1 && x < board_size-1)
		img = "4.gif";
	    else if (y < board_size-1 && x < board_size)
		img = "5.gif";
	    else if (y == board_size-1 && x == 0)
		img = "6.gif";
	    else if (y == board_size-1 && x < board_size-1)
		img = "7.gif";
	    else if (y == board_size-1 && x == board_size-1)
		img = "8.gif";



	    board += '<td><input type="image" border="0" name="'
		+ name  
		+ '" src="images/' + img +'" height=30 width=30 onClick="make_move(this);"></td>\n';
	    
	} 
	board += "</tr>\n";
    } // 

    board += "</table>";
    
    board_elem.innerHTML = board;

}

function reset_state() {
    clearInterval(window.auto_check);

    draw_board();
    window.turn = false;
    window.last_piece=null;

    $("#gameid").val("gameid");
    $("#sessionid").val("sessionid");


    hideAllMessages();

}
