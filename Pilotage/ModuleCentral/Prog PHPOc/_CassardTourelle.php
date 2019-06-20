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
					<h1 style="text-align:center">Tourelle 100mm</h1>
				</td>
				<td bgcolor=#FAAC1A align="center">
					<button id="bt_connect" type="button" onclick="connect_onclick();">Connection</button>
				</td>
				<td bgcolor=#F8D61D align="center">				
					<div>
						<h1><font id="ws_state" size=3	color="gray">Non connecté</font></h1>
					</div>
				</td>		
			</tr>
		</table>
		<table width=100% height=400 border=1>
			<tr>
				<td width=50% height=400 border=1>
					<!--******************************************************************************************************************************* -->
					<!-- *******************************************           CURSEUR ELEVATION              ***************************************** -->
					<!-- ****************************************************************************************************************************** -->
					
					<center><input type="range" id="ElevationSlider" data-orientation="vertical" min="0" max="30"></center>
					
					<script>
						var $elementElevation = $('input[id="ElevationSlider"]');
						
						function updateElevation(val) 
						{
							Elevation = Math.abs(val);
							if(ws && (ws.readyState == 1))
							{			
								ws.send("1,6,"+Elevation+",255"+"\r\n");
							}
						}

						$elementElevation
							.rangeslider({
								polyfill: false,
								onInit: function() 
								{
								//updateElevation(this.value);
								}
						  })
						  .on('input', function() 
						  {
							updateElevation(this.value);
						  });
					</script>
					
				</td>
				<td width=50% height=400 border=1>
					<!--******************************************************************************************************************************* -->
					<!-- *******************************************           CURSEUR AZIMUT              ***************************************** -->
					<!-- ****************************************************************************************************************************** -->
					<center><input type="range" id="AzimutSlider" data-orientation="horizontal" min="0" max="120"></center>
					
					<script>
						var $elementAzimut = $('input[id="AzimutSlider"]');
			
						function updateAzimut(val) 
						{
							Azimut = Math.abs(val);
						  	if(ws && (ws.readyState == 1))
							{				
								ws.send("1,5,"+Azimut+",255"+"\r\n");
							}
						}

						$elementAzimut
							.rangeslider({
								polyfill: false,
								onInit: function() 
								{
								//updateAzimut(this.value);
								}
							})
							.on('input', function() {
								updateAzimut(this.value);
							});
					</script>		
				</td>
			</tr>
		</table>
	</body>
</html>
