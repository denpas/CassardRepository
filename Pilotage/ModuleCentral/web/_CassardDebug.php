<?php
	include_once "_config.php";
	include_once "/lib/sc_envu.php";
?>

<!doctype html>

<html>
	<head>
		<title>FAA Cassard</title>
		<meta name="viewport" content="width=device-width, initial-scale=0.7, maximum-scale=0.7">
		
		<link rel="stylesheet" href="_cassard.css">
		<link rel='stylesheet prefetch' href='_normalize.min.css'>
		<link rel='stylesheet prefetch' href='_rangeslider.css'>
		<link rel="stylesheet" href="style.css">
		
		<script src='_gauge.min.js'></script>
		<script src='_jquery.min.js'></script>
		<script src='_rangeslider.min.js'></script>

		<script>
			var ws;

			function ForwardEngine(value)
			{
				if(ws && (ws.readyState == 1))
				{			
					ws.send("2,12,"+value+",255"+"\r\n");
				}
			}
			
			function ReverseEngine(value)
			{
				if(ws && (ws.readyState == 1))
				{		
					ws.send("2,13,"+value+",255"+"\r\n");
				}
			}
			
			function StopEngine()
			{
				if(ws && (ws.readyState == 1))
				{		
					ws.send("2,14,255"+"\r\n");
				}
			}

			function SetProportionalMode(value)
			{
				if(ws && (ws.readyState == 1))
				{					
					ws.send("2,18,"+value+",255"+"\r\n");
				}
			}

			function TurnLeft(value)
			{
				if(ws && (ws.readyState == 1))
				{				
					ws.send("2,16,"+value+",255"+"\r\n");
				}
			}

			function TurnRight(value)
			{
				if(ws && (ws.readyState == 1))
				{					
					ws.send("2,15,"+value+",255"+"\r\n");
				}
			}
			
			function connect_onclick()
			{
				if(ws == null)
				{
					var ws_host_addr = "<?echo _SERVER("HTTP_HOST")?>";
					var debug = document.getElementById("debug");
					if((navigator.platform.indexOf("Win") != -1) && (ws_host_addr.charAt(0) == "["))
					{
						// network resource identifier to UNC path name conversion
						ws_host_addr = ws_host_addr.replace(/[\[\]]/g, '');
						ws_host_addr = ws_host_addr.replace(/:/g, "-");
						ws_host_addr += ".ipv6-literal.net";
					}
					//debug.innerHTML = "<br>" + navigator.platform + " " + ws_host_addr;
					ws = new WebSocket("ws://" + ws_host_addr + "/_Cassard", "text.phpoc");
					document.getElementById("ws_state").innerHTML = "Connexion en cours ...";
					ws.onopen = ws_onopen;
					ws.onclose = ws_onclose;
					ws.onmessage = ws_onmessage;
				}
				else
					ws.close();		
			}

			function ws_onopen()
			{
				document.getElementById("ws_state").innerHTML = "<font color='blue'>Connecté</font>";
				document.getElementById("bt_connect").innerHTML = "Déconnecter";
			}

			function ws_onclose()
			{
				document.getElementById("ws_state").innerHTML = "<font color='gray'>Non connecté</font>";
				document.getElementById("bt_connect").innerHTML = "Connecter";
				ws.onopen = null;
				ws.onclose = null;
				ws.onmessage = null;
				ws = null;
			}

			function ws_onmessage(e_msg)
			{
				e_msg = e_msg || window.event; // MessageEvent
				alert("msg : " + e_msg.data);
			}
		</script>
	</head>
	<body>
		<table width=100% height=100 border=1>
			<tr>
				<td bgcolor=#FD5813 align="center">
					<a href="index.php" style="text-align:center">Menu</a>
				</td>
				<td bgcolor=#FC8316>
					<h1 style="text-align:center">Debug</h1>
				</td>
				<td bgcolor=#FAAC1A align="center">
					<button id="bt_connect" type="button" onclick="connect_onclick();">Connection</button>
				</td>
				<td bgcolor=#F8D61D align="center">				
					<div>
						<h1><font id="ws_state" size=3	color="gray">Non connecté</font></h1>
					</div>
					<!-- span id="debug"></span-->
				</td>		
			</tr>
		</table>
		<table bgcolor=#FEFFD3 width=100% height=100%>
			<tr>
				<div id="page-wrapper">
					<h1>Protocole debug</h1>

					<ul id="messages"></ul>

						<textarea id="message" placeholder="Entrer la trame ici..." required></textarea>
						<button type="button" id="bt_sendTrame" onclick="sendTrame_onclick();">Envoyer la trame</button>
						<button type="button" id="bt_effacer" onclick="clearSentTrame_onclick();">Effacer la liste des trames envoyées</button>
				</div>

				<script>
					// Get references to elements on the page.
					var messageField = document.getElementById('message');
					var messagesList = document.getElementById('messages');
					var socketStatus = document.getElementById('status');
					var clearBtn = document.getElementById('bt_effacer');
					var sendTrameBtn = document.getElementById('bt_sendTrame');

					function sendTrame_onclick() 
					{
						// Retrieve the message from the textarea.
						var message = messageField.value;
						
						if(ws && (ws.readyState == 1))
						{			
							ws.send(message+"\r\n");
						}

						// Add the message to the messages list.
						messagesList.innerHTML += '<li class="sent"><span>Envoyé:</span>' + message + '</li>';

						// Clear out the message field.
						messageField.value = '';
					}
					
					function clearSentTrame_onclick() 		
					{
						messagesList.innerHTML = "";
					};
				</script>
			</tr>
		</table>
	</body>
</html>
