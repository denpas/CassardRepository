<?php
	include_once "_config.php";
	include_once "/lib/sc_envu.php";
?>

<!doctype html>

<html lang="fr">
	<head>
		<meta charset="UTF-8">
		<title>Propulsion &amp; direction — Cassard</title>
		<meta name="viewport" content="width=device-width, initial-scale=1">
		<link rel="stylesheet" href="app.css">
	</head>
	<body class="control-page helm-page">
		<header class="app-header">
			<a class="brand" href="_CassardMenu.php"><span class="brand-mark">C</span>CASSARD <span class="eyebrow" style="color:#b9c9da">PILOTAGE</span></a>
			<div class="header-title">Propulsion &amp; direction</div>
			<div class="header-actions">
				<span class="connection-status" id="ws_state" aria-live="polite">Non connecté</span>
				<button class="app-button" id="bt_connect" type="button">Connecter</button>
			</div>
		</header>

		<main class="helm-layout">
			<section class="helm-center" aria-label="Commande principale">
				<div class="helm-instructions">
					<div>
						<div class="eyebrow">Commande combinée</div>
						<h1>Maintenez le joystick dans la direction souhaitée</h1>
					</div>
					<span class="helm-live-value" id="joystick-state" aria-live="polite">Neutre</span>
				</div>

				<div class="joystick-frame">
					<div class="joystick-axis-label joystick-forward">AVANT</div>
					<div class="joystick-axis-label joystick-backward">ARRIÈRE</div>
					<canvas
						id="joystick"
						class="joystick-canvas"
						width="680"
						height="400"
						tabindex="0"
						role="application"
						aria-label="Joystick en perspective. Déplacez-le à la souris, au toucher ou avec les flèches du clavier. Le relâcher arrête le moteur et recentre la direction."
					></canvas>
					<div class="joystick-hint">Glisser pour piloter · relâcher pour arrêter et recentrer</div>
				</div>

				<section class="helm-control helm-throttle" aria-labelledby="throttle-title">
					<div class="helm-control-heading">
						<div>
							<h2 id="throttle-title">Puissance moteur</h2>
							<p>Marche arrière <span>−100 %</span> · avant <span>+100 %</span></p>
						</div>
						<output class="helm-value" id="power-value" for="PuissanceSlider">0 % · arrêt</output>
					</div>
					<div class="helm-slider-wrap">
						<span class="helm-end-label">−100</span>
						<input id="PuissanceSlider" class="helm-range helm-range-horizontal" type="range" min="-100" max="100" value="0" aria-label="Puissance moteur, arrière à avant">
						<span class="helm-end-label">+100</span>
					</div>
					<div class="helm-range-center" aria-hidden="true"><span>ARRIÈRE</span><span>NEUTRE</span><span>AVANT</span></div>
				</section>

				<section class="helm-proportion" aria-label="Réactivité de la propulsion">
					<label for="ProportionSlider">Réactivité moteur</label>
					<span class="helm-proportion-value" id="proportion-value">Niveau 0</span>
					<input id="ProportionSlider" type="range" min="0" max="5" value="0" aria-label="Réactivité du moteur">
				</section>
			</section>

			<aside class="helm-side">
				<section class="helm-control helm-steering" aria-labelledby="steering-title">
					<div class="helm-control-heading">
						<div>
							<div class="eyebrow">Gouvernail</div>
							<h2 id="steering-title">Direction</h2>
						</div>
						<output class="helm-value" id="direction-value" for="DirectionSlider">0° · axe central</output>
					</div>
					<div class="helm-steering-scale helm-steering-positive" aria-hidden="true"><span>TRIBORD</span><span>+90°</span></div>
					<input id="DirectionSlider" class="helm-range helm-range-vertical" type="range" min="-90" max="90" value="0" aria-label="Direction du gouvernail, bâbord à tribord">
					<div class="helm-steering-scale helm-steering-negative" aria-hidden="true"><span>−90°</span><span>BÂBORD</span></div>
					<p class="helm-steering-help">Axe horizontal du joystick</p>
				</section>

				<section class="helm-mode">
					<span class="helm-mode-dot" aria-hidden="true"></span>
					<div>
						<strong>Mode de pilotage</strong>
						<span>Joystick + curseurs synchronisés</span>
					</div>
				</section>
			</aside>
		</main>

		<script>
			var ws = null;
			var power = 0;
			var direction = 0;
			var joystickPointer = null;
			var joystickView = { x: 0, y: 0 };
			var commandTimer = null;
			var canvas = document.getElementById("joystick");
			var canvasContext = canvas.getContext("2d");

			function sendCommand(command, value)
			{
				if(ws && ws.readyState === 1)
				{
					var frame = "2," + command;
					if(value !== null)
					{
						frame += "," + value;
					}
					ws.send(frame + ",255\r\n");
				}
			}

			function connect_onclick()
			{
				if(ws === null)
				{
					var host = "<?echo _SERVER("HTTP_HOST")?>";
					ws = new WebSocket("ws://" + host + "/_Cassard", "text.phpoc");
					document.getElementById("ws_state").textContent = "Connexion en cours…";
					ws.onopen = function()
					{
						document.getElementById("ws_state").textContent = "Connecté";
						document.getElementById("ws_state").classList.add("connected");
						document.getElementById("bt_connect").textContent = "Déconnecter";
					};
					ws.onclose = function()
					{
						document.getElementById("ws_state").textContent = "Non connecté";
						document.getElementById("ws_state").classList.remove("connected");
						document.getElementById("bt_connect").textContent = "Connecter";
						ws = null;
					};
					ws.onerror = function()
					{
						document.getElementById("ws_state").textContent = "Erreur de connexion";
						document.getElementById("ws_state").classList.remove("connected");
					};
				}
				else
				{
					ws.close();
				}
			}

			function renderJoystick(x, y)
			{
				var context = canvasContext;
				var bounds = canvas.getBoundingClientRect();
				var pixelRatio = window.devicePixelRatio || 1;
				var pixelWidth = Math.max(1, Math.round(bounds.width * pixelRatio));
				var pixelHeight = Math.max(1, Math.round(bounds.height * pixelRatio));
				if(canvas.width !== pixelWidth || canvas.height !== pixelHeight)
				{
					canvas.width = pixelWidth;
					canvas.height = pixelHeight;
				}
				var scaleX = canvas.width / 680;
				var scaleY = canvas.height / 400;
				var tiltX = x * 105;
				var tiltY = -y * 76;
				context.setTransform(scaleX, 0, 0, scaleY, 0, 0);
				context.clearRect(0, 0, 680, 400);

				context.save();
				context.translate(340, 225);
				context.scale(1, 0.36);
				var baseGradient = context.createLinearGradient(0, -120, 0, 120);
				baseGradient.addColorStop(0, "#ffffff");
				baseGradient.addColorStop(0.55, "#dce7f0");
				baseGradient.addColorStop(1, "#aebfce");
				context.fillStyle = baseGradient;
				context.beginPath();
				context.arc(0, 0, 210, 0, Math.PI * 2);
				context.fill();
				context.strokeStyle = "#9eafbf";
				context.lineWidth = 4;
				context.stroke();
				context.restore();

				context.save();
				context.translate(340, 225);
				context.scale(1, 0.36);
				context.strokeStyle = "rgba(36,116,166,0.16)";
				context.lineWidth = 2;
				for(var ring = 1; ring <= 3; ring++)
				{
					context.beginPath();
					context.arc(0, 0, ring * 48, 0, Math.PI * 2);
					context.stroke();
				}
				context.beginPath();
				context.moveTo(-155, 0);
				context.lineTo(155, 0);
				context.moveTo(0, -145);
				context.lineTo(0, 145);
				context.stroke();
				context.restore();

				context.fillStyle = "rgba(16,35,63,0.18)";
				context.beginPath();
				context.ellipse(340 + tiltX, 235 + tiltY, 62, 23, 0, 0, Math.PI * 2);
				context.fill();

				var pivotX = 340 + tiltX;
				var pivotY = 210 + tiltY;
				context.lineCap = "round";
				context.strokeStyle = "#8398aa";
				context.lineWidth = 29;
				context.beginPath();
				context.moveTo(340, 220);
				context.lineTo(pivotX, pivotY);
				context.stroke();
				context.strokeStyle = "#f9fbfd";
				context.lineWidth = 19;
				context.beginPath();
				context.moveTo(340, 220);
				context.lineTo(pivotX, pivotY);
				context.stroke();

				var knobX = pivotX;
				var knobY = pivotY - 78;
				var knobGradient = context.createRadialGradient(knobX - 20, knobY - 19, 3, knobX, knobY, 58);
				knobGradient.addColorStop(0, "#ffffff");
				knobGradient.addColorStop(0.28, "#80b3d1");
				knobGradient.addColorStop(1, "#2474a6");
				context.fillStyle = knobGradient;
				context.beginPath();
				context.arc(knobX, knobY, 52, 0, Math.PI * 2);
				context.fill();
				context.strokeStyle = "#1b5c87";
				context.lineWidth = 3;
				context.stroke();
				context.fillStyle = "rgba(255,255,255,0.72)";
				context.beginPath();
				context.ellipse(knobX - 14, knobY - 19, 14, 8, -0.45, 0, Math.PI * 2);
				context.fill();

				context.setTransform(1, 0, 0, 1, 0, 0);
			}

			function updateReadouts()
			{
				var powerLabel = power > 0 ? "avant" : (power < 0 ? "arrière" : "arrêt");
				document.getElementById("power-value").textContent = power + " % · " + powerLabel;
				document.getElementById("direction-value").textContent = direction === 0
					? "0° · axe central"
					: Math.abs(direction) + "° · " + (direction < 0 ? "bâbord" : "tribord");
				document.getElementById("joystick-state").textContent = power === 0 && direction === 0
					? "Neutre"
					: powerLabel + " · " + Math.abs(direction) + "°";
				document.getElementById("PuissanceSlider").value = power;
				document.getElementById("DirectionSlider").value = direction;
				joystickView.x = direction / 90;
				joystickView.y = power / 100;
				renderJoystick(joystickView.x, joystickView.y);
			}

			function publishControls()
			{
				if(power > 0)
				{
					sendCommand(12, power);
				}
				else if(power < 0)
				{
					sendCommand(13, Math.abs(power));
				}
				else
				{
					sendCommand(14, null);
				}

				if(direction < 0)
				{
					sendCommand(16, Math.abs(direction));
				}
				else if(direction > 0)
				{
					sendCommand(15, direction);
				}
				else
				{
					sendCommand(17, null);
				}
			}

			function scheduleControls()
			{
				updateReadouts();
				if(commandTimer !== null)
				{
					clearTimeout(commandTimer);
				}
				commandTimer = setTimeout(function()
				{
					commandTimer = null;
					publishControls();
				}, 80);
			}

			function setJoystickFromPointer(event)
			{
				var bounds = canvas.getBoundingClientRect();
				var x = ((event.clientX - bounds.left) / bounds.width - 0.5) * 2;
				var y = (0.5 - (event.clientY - bounds.top) / bounds.height) * 2;
				var distance = Math.sqrt(x * x + y * y);
				if(distance > 1)
				{
					x /= distance;
					y /= distance;
				}
				direction = Math.round(x * 90);
				power = Math.round(y * 100);
				joystickView = { x: x, y: y };
				renderJoystick(x, y);
				scheduleControls();
			}

			function returnJoystickToNeutral()
			{
				var startX = joystickView.x;
				var startY = joystickView.y;
				var startedAt = Date.now();
				power = 0;
				direction = 0;
				updateReadouts();
				if(commandTimer !== null)
				{
					clearTimeout(commandTimer);
					commandTimer = null;
				}
				publishControls();

				function animateReturn()
				{
					var progress = Math.min(1, (Date.now() - startedAt) / 180);
					var eased = 1 - Math.pow(1 - progress, 3);
					joystickView.x = startX * (1 - eased);
					joystickView.y = startY * (1 - eased);
					renderJoystick(joystickView.x, joystickView.y);
					if(progress < 1)
					{
						requestAnimationFrame(animateReturn);
					}
				}
				requestAnimationFrame(animateReturn);
			}

			canvas.addEventListener("pointerdown", function(event)
			{
				joystickPointer = event.pointerId;
				canvas.setPointerCapture(event.pointerId);
				setJoystickFromPointer(event);
				event.preventDefault();
			});
			canvas.addEventListener("pointermove", function(event)
			{
				if(event.pointerId === joystickPointer)
				{
					setJoystickFromPointer(event);
					event.preventDefault();
				}
			});
			canvas.addEventListener("pointerup", function(event)
			{
				if(event.pointerId === joystickPointer)
				{
					joystickPointer = null;
					returnJoystickToNeutral();
				}
			});
			canvas.addEventListener("pointercancel", function(event)
			{
				if(event.pointerId === joystickPointer)
				{
					joystickPointer = null;
					returnJoystickToNeutral();
				}
			});
			canvas.addEventListener("keydown", function(event)
			{
				var increments = { ArrowUp: [0, 5], ArrowDown: [0, -5], ArrowLeft: [-5, 0], ArrowRight: [5, 0] };
				if(increments[event.key])
				{
					power = Math.max(-100, Math.min(100, power + increments[event.key][1]));
					direction = Math.max(-90, Math.min(90, direction + increments[event.key][0]));
					scheduleControls();
					event.preventDefault();
				}
				else if(event.key === " ")
				{
					returnJoystickToNeutral();
					event.preventDefault();
				}
			});

			document.getElementById("PuissanceSlider").addEventListener("input", function()
			{
				power = Number(this.value);
				scheduleControls();
			});
			document.getElementById("DirectionSlider").addEventListener("input", function()
			{
				direction = Number(this.value);
				scheduleControls();
			});
			document.getElementById("ProportionSlider").addEventListener("input", function()
			{
				var level = Number(this.value);
				document.getElementById("proportion-value").textContent = "Niveau " + level;
				sendCommand(18, level);
			});
			document.getElementById("bt_connect").addEventListener("click", connect_onclick);
			window.addEventListener("resize", function()
			{
				renderJoystick(joystickView.x, joystickView.y);
			});

			renderJoystick(0, 0);
		</script>
	</body>
</html>
