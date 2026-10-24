<?php
	include_once "_config.php";
	include_once "/lib/sc_envu.php";
?>

<!doctype html>

<html lang="fr">
	<head>
		<meta charset="UTF-8">
		<title>Tourelle 100 mm — Cassard</title>
		<meta name="viewport" content="width=device-width, initial-scale=1">
		
		<link rel='stylesheet prefetch' href='_normalize.min.css'>
		<link rel='stylesheet prefetch' href='_rangeslider.css'>
		<link rel="stylesheet" href="style.css">
		<link rel="stylesheet" href="app.css">
		
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
			<div class="header-title">Tourelle 100 mm</div>
			<div class="header-actions">
				<span class="connection-status" id="ws_state">Non connecté</span>
				<button class="app-button" id="bt_connect" type="button" onclick="connect_onclick();">Connecter</button>
			</div>
		</header>
		<main class="control-main">
			<section class="control-card vertical-control">
				<h2>Élévation</h2>
				<p>Inclinaison · de 0° à 60°</p>
				<span class="control-value" id="elevation-value">0°</span>
				<input type="range" id="ElevationSlider" data-orientation="vertical" min="0" max="60" value="0">
				<script>
					var $elementElevation = $('input[id="ElevationSlider"]');
					var elevation = 0;
					var azimuth = 0;

					function sendTurretPosition()
					{
						if(ws && (ws.readyState == 1))
						{
							ws.send("1,4,"+azimuth+","+elevation+",255\r\n");
						}
					}

					function updateElevation(val)
					{
						elevation = Number(val);
						document.getElementById("elevation-value").textContent = elevation + "°";
						sendTurretPosition();
					}

					$elementElevation
						.rangeslider({
							polyfill: false,
							onInit: function()
							{
							}
					  })
					  .on('input', function()
					  {
						updateElevation(this.value);
					  });
				</script>
			</section>
			<section class="control-card">
				<h2>Azimut</h2>
				<p>Orientation · de 0° à 180°</p>
				<span class="control-value" id="azimuth-value">0°</span>
				<input type="range" id="AzimutSlider" data-orientation="horizontal" min="0" max="180" value="0">
				<script>
					var $elementAzimut = $('input[id="AzimutSlider"]');

					function updateAzimut(val)
					{
						azimuth = Number(val);
						document.getElementById("azimuth-value").textContent = azimuth + "°";
						sendTurretPosition();
					}

					$elementAzimut
						.rangeslider({
							polyfill: false,
							onInit: function()
							{
							}
						})
						.on('input', function() {
							updateAzimut(this.value);
						});
				</script>
			</section>
		</main>
	</body>
</html>
