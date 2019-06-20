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
				<td bgcolor=#FC8316 align="center">
					<h1 style="text-align:center">Instruments</h1>
				</td>
				<td bgcolor=#FAAC1A align="center">
					<button id="bt_connect" type="button" onclick="connect_onclick();">Connection</button>
				</td>
				<td bgcolor=#F8D61D aligh="center">				
					<div>
						<h1><font id="ws_state" size=3	color="gray">Non connecté</font></h1>
					</div>
					<!-- span id="debug"></span-->
				</td>		
			</tr>
		</table>
		<table bgcolor=#FEFFD3 width=100% height=100%>
			<tr>
				<!--div id="contenu"--> 
				<!--div class = "ligne1"-->		
				<td align="center">
					<!-- ****************************************************************************************************************************** -->
					<!-- ********************************      VITESSE ROTATION MOTEUR GAUCHE      **************************************************** -->
					<!-- ****************************************************************************************************************************** -->
					<canvas 
						id="Vitesse_Moteur_Gauche" 
						data-title="Moteur" 
						data-units="tr/min" 
						width=250
						height=250
						data-major-ticks="0 1000 2000 3000 4000 5000 6000" 
						data-type="canv-gauge" 
						data-min-value="0" 
						data-max-value="6000" 
						data-highlights="0 600 #4D89F2, 601 1000 #25B8D9, 1001 3000 #0BB950, 3001 4000 #cc5, 4001 6000 #f33" 
						data-onready="setInterval( function() { Gauge.Collection.get('Vitesse_Moteur_Gauche').setValue(RotationMoteur);}, 200);">
					</canvas>
				</td>
				<td align="center">
					<!-- ****************************************************************************************************************************** -->
					<!-- ********************************      VITESSE ROTATION ARBRE GAUCHE       **************************************************** -->
					<!-- ****************************************************************************************************************************** -->
					<canvas 
						id="Vitesse_Arbre_Gauche" 
						data-title="Arbre" 
						data-units="tr/min" width="250" height="250" 
						data-major-ticks="0 500 1000 1500 2000 2500" 
						data-type="canv-gauge" 
						data-min-value="0" 
						data-max-value="2500" 
						data-highlights="0 100 #4D89F2, 101 300 #25B8D9, 301 500 #0BB950, 501 1000 #cc5, 1001 2500 #f33" 
						data-onready="setInterval( function() { Gauge.Collection.get('Vitesse_Arbre_Gauche').setValue(RotationArbre);}, 200);">
					</canvas>
				</td>
				<td align="center">
					<!-- ****************************************************************************************************************************** -->
					<!-- ********************************                 PUISSANCE                **************************************************** -->
					<!-- ****************************************************************************************************************************** -->
					<canvas 	
						id="Puissance" 
						data-title="Puissance" 
						data-units="%" width="300" height="300" 
						data-major-ticks="0 10 20 30 40 50 60 70 80 90 100" 
						data-type="canv-gauge" 
						data-min-value="0" 
						data-max-value="100" 
						data-highlights="0 10 #4D89F2, 11 30 #25B8D9, 30 50 #0BB950, 51 80 #cc5, 81 100 #f33" 
						data-onready="setInterval( function() { Gauge.Collection.get('Puissance').setValue(Puissance);}, 200);">
					</canvas>
				</td align="center">
				<td align="center">
					<!-- ****************************************************************************************************************************** -->
					<!-- ********************************      VITESSE ROTATION ARBRE DROIT        **************************************************** -->
					<!-- ****************************************************************************************************************************** -->
					<canvas 
						id="Vitesse_Arbre_Droit" 
						data-title="Arbre" 
						data-units="tr/min" width="250" height="250" 
						data-major-ticks="0 500 1000 1500 2000 2500" 
						data-type="canv-gauge" 
						data-min-value="0" 
						data-max-value="2500" 
						data-highlights="0 100 #4D89F2, 101 300 #25B8D9, 301 500 #0BB950, 501 1000 #cc5, 1001 2500 #f33" 
						data-onready="setInterval( function() { Gauge.Collection.get('Vitesse_Arbre_Droit').setValue(RotationArbre);}, 200);">
					</canvas>
				</td>
				<td align="center">
					<!-- ****************************************************************************************************************************** -->
					<!-- ********************************      VITESSE ROTATION MOTEUR DROIT       **************************************************** -->
					<!-- ****************************************************************************************************************************** -->
					<canvas 
						id="Vitesse_Moteur_Droit" 
						data-title="Moteur" 
						data-units="tr/min" width="250" height="250" 
						data-major-ticks="0 1000 2000 3000 4000 5000 6000" 
						data-type="canv-gauge" 
						data-min-value="0" 
						data-max-value="4000" 
						data-highlights="0 600 #4D89F2, 601 1000 #25B8D9, 1001 3000 #0BB950, 3001 4000 #cc5, 4001 6000 #f33" 
						data-onready="setInterval( function() { Gauge.Collection.get('Vitesse_Moteur_Droit').setValue(RotationMoteur);}, 200);">
					</canvas>
				</td>
			</tr>
		</table>
	</body>
</html>
