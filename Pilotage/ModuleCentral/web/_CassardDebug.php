<?php
	include_once "_config.php";
	include_once "/lib/sc_envu.php";
?>

<!doctype html>

<html lang="fr">
	<head>
		<meta charset="UTF-8">
		<title>Diagnostic — Cassard</title>
		<meta name="viewport" content="width=device-width, initial-scale=1">
		
		<link rel='stylesheet prefetch' href='_normalize.min.css'>
		<link rel='stylesheet prefetch' href='_rangeslider.css'>
		<link rel="stylesheet" href="style.css">
		<link rel="stylesheet" href="app.css">
		<script src="_cassard-link.js"></script>
		<script>CassardLink.requireKey();</script>
		
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
					ws = CassardLink.open();
					document.getElementById("ws_state").textContent = "Connexion en cours…";
					ws.onopen = ws_onopen;
					ws.onclose = ws_onclose;
					ws.onmessage = ws_onmessage;
				}
				else
					ws.close();		
			}

			function ws_onopen()
			{
				document.getElementById("ws_state").textContent = "Connecté";
				document.getElementById("ws_state").classList.add("connected");
				document.getElementById("bt_connect").textContent = "Déconnecter";
			}

			function ws_onclose()
			{
				document.getElementById("ws_state").textContent = "Non connecté";
				document.getElementById("ws_state").classList.remove("connected");
				document.getElementById("bt_connect").textContent = "Connecter";
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
	<body class="control-page">
		<header class="app-header">
			<a class="brand" href="_CassardMenu.php"><span class="brand-mark">C</span>CASSARD <span class="eyebrow" style="color:#b9c9da">PILOTAGE</span></a>
			<div class="header-title">Console de diagnostic</div>
			<div class="header-actions">
				<span class="connection-status" id="ws_state">Non connecté</span>
				<button class="app-button" id="bt_connect" type="button" onclick="connect_onclick();">Connecter</button>
			</div>
		</header>
		<main class="page-shell">
			<div id="page-wrapper" class="debug-panel">
				<div class="eyebrow">Outils techniques</div>
				<h2>Échange de trames</h2>
				<p class="page-intro">Envoyez une commande brute au contrôleur et consultez l’historique envoyé.</p>

				<ul id="messages"></ul>

				<textarea id="message" placeholder="Entrer la trame ici..." required></textarea>
				<button class="app-button" type="button" id="bt_sendTrame" onclick="sendTrame_onclick();">Envoyer la trame</button>
				<button class="app-button" type="button" id="bt_effacer" onclick="clearSentTrame_onclick();">Effacer l’historique</button>
				<p id="debug-status" class="page-intro" role="status"></p>
			</div>

				<script>
					// Get references to elements on the page.
					var messageField = document.getElementById('message');
					var messagesList = document.getElementById('messages');
					function sendTrame_onclick() 
					{
						var message = messageField.value;
						
						if(!ws || (ws.readyState != 1))
						{			
							document.getElementById("debug-status").textContent = "Connectez-vous avant d’envoyer une trame.";
							return;
						}

						ws.send(message+"\r\n");
						var messageItem = document.createElement("li");
						var messageLabel = document.createElement("span");
						messageLabel.textContent = "Envoyé :";
						messageItem.appendChild(messageLabel);
						messageItem.appendChild(document.createTextNode(message));
						messagesList.appendChild(messageItem);
						document.getElementById("debug-status").textContent = "";

						messageField.value = '';
					}
					
					function clearSentTrame_onclick() 		
					{
						messagesList.innerHTML = "";
					};
				</script>
		</main>
	</body>
</html>
