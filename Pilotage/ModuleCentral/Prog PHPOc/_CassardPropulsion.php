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
					<h1 style="text-align:center">Propulsion & Direction</h1>
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

		<table bgcolor=#FEFFD3 width=100% height=100>
			<tr width=200>
				<th>
					<!--******************************************************************************************************************************* -->
					<!-- *******************************************           CURSEUR PROPORTION             ***************************************** -->
					<!-- ****************************************************************************************************************************** -->
					<center><input type="range" id="ProportionSlider" data-orientation="horizontal" min="0" max="5"></center>
					<script>
						var $elementProportion = $('input[id="ProportionSlider"]');

						function updateProportion(val) {
							Proportion = Math.abs(val);
						  	SetProportionalMode(Proportion);
						}

						$elementProportion
							.rangeslider({
								polyfill: false,
								onInit: function() {
									updateProportion(this.value);
								}
							})
							.on('input', function() {
								updateProportion(this.value);
							});
					</script>					
				</th>	
			</tr>
		</table>
		<table bgcolor=#FEFFD3 width=100% height=100% border=1>
			<tr>
				<th width=50%>
					<!--******************************************************************************************************************************* -->
					<!-- *******************************************           CURSEUR PUISSANCE              ***************************************** -->
					<!-- ****************************************************************************************************************************** -->
					<center><input type="range" id="PuissanceSlider" data-orientation="vertical" min="-100" max="100"></center>
					<script>
						var $elementPuissance = $('input[id="PuissanceSlider"]');
						
						function updatePuissance(val) {
							Puissance = Math.abs(val);
							RotationMoteur = Puissance * 40;
							RotationArbre = Puissance * 15;
							if (val > 0)
							{
								ForwardEngine(Puissance);
							}
							if (val < 0)
							{
								ReverseEngine(Puissance);
							} 
							if (val = 0)
							{
								StopEngine();
							} 
						}

						$elementPuissance
						  .rangeslider({
							polyfill: false,
							onInit: function() {
							  updatePuissance(this.value);
							}
						  })
						  .on('input', function() {
							updatePuissance(this.value);
						  });
					</script>
				</th>
				<th width=50%>
					<!--******************************************************************************************************************************* -->
					<!-- *******************************************           CURSEUR DIRECTION              ***************************************** -->
					<!-- ****************************************************************************************************************************** -->
					<center><input type="range" id="DirectionSlider" data-orientation="horizontal" min="-90" max="90"></center>
					<script>
						var $elementDirection = $('input[id="DirectionSlider"]');

						function updateDirection(val) {
							Direction = Math.abs(val);
						  	if (val < 0)
							{
								TurnLeft(Direction);
							} 
							if (val > 0)
							{
								TurnRight(Direction);
							}
						}

						$elementDirection
							.rangeslider({
								polyfill: false,
								onInit: function() {
								updateDirection(this.value);
							}
							})
							.on('input', function() {
								updateDirection(this.value);
							});
					</script>					
				</th>
			</tr>
		</table>
	</body>
</html>
