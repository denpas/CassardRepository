<?php
	include_once "_config.php";
	include_once "/lib/sc_envu.php";
?>

<!doctype html>

<html lang="fr">
	<head>
		<meta charset="UTF-8">
		<title>Tourelle 100 mm — Cassard</title>
		<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
		<meta name="theme-color" content="#101215">
		<meta name="apple-mobile-web-app-capable" content="yes">
		<meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
		<link rel="stylesheet" href="app.css">
		<script src="_cassard-link.js"></script>
		<script src="_cassard-trim.js"></script>
		<script>CassardLink.requireKey();</script>
	</head>
	<body class="control-page helm-page turret-page">
		<header class="app-header">
			<a class="brand" href="_CassardMenu.php"><span class="brand-mark">C</span>CASSARD</a>
			<div class="header-title">Tourelle 100 mm</div>
			<span class="connection-status" id="ws_state" aria-live="polite">Non connecté</span>
		</header>

		<main class="tx turret" aria-label="Commande de la tourelle 100 mm">
			<div class="tx-body turret-body">
				<span class="tx-screw tx-screw-tl" aria-hidden="true"></span>
				<span class="tx-screw tx-screw-tr" aria-hidden="true"></span>
				<span class="tx-screw tx-screw-bl" aria-hidden="true"></span>
				<span class="tx-screw tx-screw-br" aria-hidden="true"></span>

				<div class="turret-strip">
					<span class="tx-brand-badge">CASSARD</span>
					<span class="tx-print turret-model">T 100</span>
					<span class="tx-led" id="tx-led" aria-hidden="true"></span>
					<span class="tx-power-label">POWER</span>
					<button class="tx-power-switch" id="bt_connect" type="button" role="switch" aria-checked="false" aria-label="Liaison avec le navire"></button>
				</div>

				<section class="turret-view turret-view-top" aria-label="Vue de dessus, azimut">
					<div class="turret-view-heading">
						<span class="tx-print">Vue de dessus · azimut ±60°</span>
						<output class="turret-readout" id="azimuth-value" aria-live="polite">0°</output>
					</div>
					<div class="turret-screen">
						<svg id="top-view" class="turret-drawing" viewBox="-1180 -1340 2360 2190" preserveAspectRatio="xMidYMid meet" role="img" aria-label="Tourelle vue de dessus : faites-la pivoter au doigt">
							<g class="turret-dial">
								<line class="turret-ship-axis" x1="0" y1="-1300" x2="0" y2="820"></line>
								<path class="turret-dial-arc" id="azimuth-arc"></path>
								<g id="azimuth-ticks"></g>
								<text class="turret-dial-label" x="0" y="-1280" text-anchor="middle">AVANT</text>
								<text class="turret-dial-label" x="-1065" y="-400" text-anchor="middle">BÂB</text>
								<text class="turret-dial-label" x="1065" y="-400" text-anchor="middle">TRI</text>
								<path class="turret-dial-marker" id="azimuth-marker" d="M0,-1200 L-34,-1262 L34,-1262 Z"></path>
							</g>
							<g id="turret-rotation" class="turret-handle">
								<g transform="rotate(-90) translate(-640 -756)">
									<!-- Contour rendu symétrique par rapport à l'axe du canon (y = 756) : le plan PNG d'origine ne l'était pas. -->
									<polygon class="part" points="312,168 664,160 1144,456 1144,1056 664,1352 312,1344 120,1120 120,392"></polygon>
									<circle class="detail" cx="616" cy="216" r="20"></circle>
									<circle class="detail" cx="600" cy="1296" r="16"></circle>
									<polygon class="detail" points="216,456 648,456 648,640 424,640 424,888 680,888 680,1096 216,1096"></polygon>
									<polygon class="dashed" points="240,480 624,480 624,616 400,616 400,912 656,912 656,1072 240,1072"></polygon>
									<line class="thin" x1="280" y1="1016" x2="400" y2="856"></line>
									<line class="thin" x1="648" y1="456" x2="1144" y2="456"></line>
									<line class="thin" x1="680" y1="1056" x2="1144" y2="1056"></line>
									<rect class="vent" x="696" y="480" width="28" height="152" rx="3"></rect>
									<rect class="vent" x="764" y="480" width="28" height="152" rx="3"></rect>
									<rect class="vent" x="832" y="480" width="28" height="152" rx="3"></rect>
									<rect class="detail" x="920" y="488" width="192" height="128" rx="8"></rect>
									<rect class="vent" x="696" y="880" width="28" height="136" rx="3"></rect>
									<rect class="vent" x="756" y="880" width="28" height="136" rx="3"></rect>
									<rect class="vent" x="816" y="880" width="28" height="136" rx="3"></rect>
									<rect class="vent" x="876" y="880" width="28" height="136" rx="3"></rect>
									<rect class="vent" x="936" y="880" width="28" height="136" rx="3"></rect>
									<rect class="vent" x="996" y="880" width="28" height="136" rx="3"></rect>
									<rect class="vent" x="1056" y="880" width="28" height="136" rx="3"></rect>
									<rect class="part" x="1144" y="480" width="24" height="80" rx="4"></rect>
									<rect class="part" x="1144" y="952" width="24" height="80" rx="4"></rect>
									<rect class="vent" x="456" y="676" width="712" height="160"></rect>
									<path class="part" d="M488,700 C520,688 760,690 920,692 L1272,732 L1272,780 L920,820 C760,822 520,824 488,812 Z"></path>
									<rect class="part" x="1272" y="732" width="520" height="48" rx="6"></rect>
									<rect class="part" x="1792" y="724" width="48" height="64" rx="10"></rect>
									<line class="axis" x1="100" y1="756" x2="1900" y2="756"></line>
								</g>
							</g>
							<circle class="turret-pivot" cx="0" cy="0" r="14"></circle>
						</svg>
					</div>
				</section>

				<div class="trim trim-horizontal turret-trim-azimuth" id="azimuth-trim">
					<button class="trim-step trim-step-left" type="button" data-step="-5" aria-label="Pivoter à bâbord">BÂB</button>
					<div class="trim-track" tabindex="0" role="slider" aria-orientation="horizontal" aria-label="Azimut de la tourelle, bâbord à tribord">
						<div class="trim-slot"></div>
						<div class="trim-rail"><i class="trim-fill"></i><span class="trim-lever"></span></div>
					</div>
					<button class="trim-step trim-step-right" type="button" data-step="5" aria-label="Pivoter à tribord">TRI</button>
				</div>

				<section class="turret-view turret-view-side" aria-label="Vue de profil, élévation">
					<div class="turret-view-heading">
						<span class="tx-print">Vue de profil · élévation 0–80°</span>
						<output class="turret-readout" id="elevation-value" aria-live="polite">0°</output>
					</div>
					<div class="turret-side-row">
						<div class="turret-screen">
							<svg id="side-view" class="turret-drawing" viewBox="40 -660 1940 2140" preserveAspectRatio="xMidYMid meet" role="img" aria-label="Tourelle de profil : inclinez le canon au doigt">
								<g class="turret-dial">
									<path class="turret-dial-arc" id="elevation-arc"></path>
									<g id="elevation-ticks"></g>
									<path class="turret-dial-marker" id="elevation-marker"></path>
								</g>
								<path class="part" d="M120,392 Q120,320 192,320 L700,320 L1160,780 L1160,1108 L120,1108 Z"></path>
								<rect class="part" x="100" y="480" width="20" height="120" rx="4"></rect>
								<line class="seam" x1="120" y1="640" x2="920" y2="640"></line>
								<line class="seam" x1="120" y1="960" x2="1160" y2="960"></line>
								<circle class="detail" cx="608" cy="648" r="24"></circle>
								<circle class="rivet" cx="732" cy="336" r="10"></circle>
								<circle class="rivet" cx="776" cy="380" r="10"></circle>
								<circle class="rivet" cx="952" cy="556" r="10"></circle>
								<circle class="rivet" cx="996" cy="600" r="10"></circle>
								<circle class="rivet" cx="1040" cy="644" r="10"></circle>
								<circle class="rivet" cx="1084" cy="688" r="10"></circle>
								<circle class="rivet" cx="1128" cy="732" r="10"></circle>
								<rect class="vent" x="328" y="1108" width="680" height="32"></rect>
								<polygon class="part" points="320,1140 1008,1140 1056,1400 272,1400"></polygon>
								<rect class="vent" x="392" y="1160" width="36" height="232" rx="4"></rect>
								<rect class="vent" x="536" y="1160" width="36" height="232" rx="4"></rect>
								<rect class="vent" x="680" y="1160" width="36" height="232" rx="4"></rect>
								<rect class="vent" x="824" y="1160" width="36" height="232" rx="4"></rect>
								<rect class="vent" x="952" y="1160" width="36" height="232" rx="4"></rect>
								<rect class="part" x="232" y="1400" width="864" height="48" rx="6"></rect>
								<line class="ground" x1="40" y1="1448" x2="1940" y2="1448"></line>
								<g id="gun" class="turret-handle" transform="translate(777 527)">
									<polygon class="part" points="0,-51 471,-27 471,27 0,51"></polygon>
									<rect class="part" x="468" y="-22" width="556" height="44" rx="8"></rect>
									<rect class="part" x="1020" y="-30" width="48" height="60" rx="10"></rect>
								</g>
								<circle class="turret-pivot" cx="777" cy="527" r="12"></circle>
							</svg>
						</div>

						<div class="trim trim-vertical turret-trim-elevation" id="elevation-trim">
							<button class="trim-step trim-step-up" type="button" data-step="5" aria-label="Lever le canon">HAUT</button>
							<div class="trim-track" tabindex="0" role="slider" aria-orientation="vertical" aria-label="Élévation du canon, horizontale à 80°">
								<div class="trim-slot"></div>
								<div class="trim-rail"><i class="trim-fill"></i><span class="trim-lever"></span></div>
							</div>
							<button class="trim-step trim-step-down" type="button" data-step="-5" aria-label="Baisser le canon">BAS</button>
						</div>
					</div>
				</section>
			</div>
		</main>

		<script>
			var ws = null;
			var rotation = 0;
			var elevation = 0;
			var commandTimer = null;
			var AZIMUTH_LIMIT = 60;
			var ELEVATION_MAX = 80;
			var AZIMUTH_CENTER = 90;
			var SIDE_PIVOT = { x: 777, y: 527 };
			var AZIMUTH_DIAL_RADIUS = 1230;
			var ELEVATION_DIAL_RADIUS = 1150;
			var SVG_NS = "http://www.w3.org/2000/svg";

			function setLinkState(label, connected)
			{
				document.getElementById("ws_state").textContent = label;
				document.getElementById("ws_state").classList.toggle("connected", connected);
				document.getElementById("bt_connect").setAttribute("aria-checked", ws !== null ? "true" : "false");
				document.getElementById("tx-led").classList.toggle("on", connected);
			}

			function sendTurretPosition()
			{
				if(ws && ws.readyState === 1)
				{
					ws.send("1,4," + (AZIMUTH_CENTER + rotation) + "," + elevation + ",255\r\n");
				}
			}

			function connect_onclick()
			{
				if(ws === null)
				{
					ws = CassardLink.open();
					setLinkState("Connexion…", false);
					ws.onopen = function()
					{
						setLinkState("Connecté", true);
						sendTurretPosition();
					};
					ws.onclose = function()
					{
						ws = null;
						setLinkState("Non connecté", false);
					};
					ws.onerror = function()
					{
						setLinkState("Erreur de connexion", false);
					};
				}
				else
				{
					ws.close();
				}
			}

			// ---------- Cadrans gradués ----------

			function polar(cx, cy, radius, degrees)
			{
				var angle = degrees * Math.PI / 180;
				return { x: cx + Math.cos(angle) * radius, y: cy + Math.sin(angle) * radius };
			}

			function drawDial(arcId, ticksId, cx, cy, radius, fromDegrees, toDegrees, labelFor)
			{
				var start = polar(cx, cy, radius, fromDegrees);
				var end = polar(cx, cy, radius, toDegrees);
				document.getElementById(arcId).setAttribute("d",
					"M" + start.x + "," + start.y + " A" + radius + "," + radius + " 0 0 1 " + end.x + "," + end.y);
				var ticks = document.getElementById(ticksId);
				for(var degrees = fromDegrees; degrees <= toDegrees + 0.01; degrees += 5)
				{
					var label = labelFor(degrees);
					var major = label !== null;
					var medium = Math.round(degrees - fromDegrees) % 10 === 0;
					var inner = polar(cx, cy, radius - (major ? 70 : (medium ? 48 : 28)), degrees);
					var outer = polar(cx, cy, radius, degrees);
					var tick = document.createElementNS(SVG_NS, "line");
					tick.setAttribute("x1", inner.x);
					tick.setAttribute("y1", inner.y);
					tick.setAttribute("x2", outer.x);
					tick.setAttribute("y2", outer.y);
					tick.setAttribute("class", major ? "turret-tick major" : "turret-tick");
					ticks.appendChild(tick);
					if(major)
					{
						var position = polar(cx, cy, radius - 130, degrees);
						var text = document.createElementNS(SVG_NS, "text");
						text.setAttribute("x", position.x);
						text.setAttribute("y", position.y + 20);
						text.setAttribute("text-anchor", "middle");
						text.setAttribute("class", "turret-tick-label");
						text.textContent = label;
						ticks.appendChild(text);
					}
				}
			}

			// Azimut : 0° vers l'avant (haut de l'écran), angles SVG -150° à -30°.
			drawDial("azimuth-arc", "azimuth-ticks", 0, 0, AZIMUTH_DIAL_RADIUS, -90 - AZIMUTH_LIMIT, -90 + AZIMUTH_LIMIT, function(degrees)
			{
				var value = Math.round(degrees + 90);
				return value % 30 === 0 ? Math.abs(value) + "°" : null;
			});
			// Élévation : 0° horizontal, 80° presque vertical ; angles SVG -80° à 0°.
			drawDial("elevation-arc", "elevation-ticks", SIDE_PIVOT.x, SIDE_PIVOT.y, ELEVATION_DIAL_RADIUS, -ELEVATION_MAX, 0, function(degrees)
			{
				var value = Math.round(-degrees);
				return value % 20 === 0 ? value + "°" : null;
			});

			// ---------- Affichage et commandes ----------

			var azimuthTrim = createTrim(document.getElementById("azimuth-trim"), -AZIMUTH_LIMIT, AZIMUTH_LIMIT, 30, function(value)
			{
				rotation = value;
				scheduleTurret();
			});
			var elevationTrim = createTrim(document.getElementById("elevation-trim"), 0, ELEVATION_MAX, 20, function(value)
			{
				elevation = value;
				scheduleTurret();
			});

			function render()
			{
				document.getElementById("turret-rotation").setAttribute("transform", "rotate(" + rotation + ")");
				document.getElementById("azimuth-marker").setAttribute("transform", "rotate(" + rotation + ")");
				document.getElementById("gun").setAttribute("transform", "translate(" + SIDE_PIVOT.x + " " + SIDE_PIVOT.y + ") rotate(" + (-elevation) + ")");
				var tip = polar(SIDE_PIVOT.x, SIDE_PIVOT.y, ELEVATION_DIAL_RADIUS + 8, -elevation);
				var left = polar(SIDE_PIVOT.x, SIDE_PIVOT.y, ELEVATION_DIAL_RADIUS + 70, -elevation - 1.8);
				var right = polar(SIDE_PIVOT.x, SIDE_PIVOT.y, ELEVATION_DIAL_RADIUS + 70, -elevation + 1.8);
				document.getElementById("elevation-marker").setAttribute("d",
					"M" + tip.x + "," + tip.y + " L" + left.x + "," + left.y + " L" + right.x + "," + right.y + " Z");
				document.getElementById("azimuth-value").textContent = rotation === 0
					? "0° · axe"
					: Math.abs(rotation) + "° · " + (rotation < 0 ? "bâbord" : "tribord");
				document.getElementById("elevation-value").textContent = elevation + "°";
				azimuthTrim.set(rotation);
				elevationTrim.set(elevation);
			}

			function scheduleTurret()
			{
				render();
				if(commandTimer !== null)
				{
					clearTimeout(commandTimer);
				}
				commandTimer = setTimeout(function()
				{
					commandTimer = null;
					sendTurretPosition();
				}, 80);
			}

			// ---------- Rotation à la main ----------

			// Fait tourner une pièce autour de son pivot en suivant le doigt : la
			// valeur varie de l'angle balayé depuis la prise (direction : +1 sens
			// horaire à l'écran, -1 sens trigonométrique).
			function makeRotatable(svg, handle, pivot, direction, min, max, getValue, setValue)
			{
				var dragPointer = null;
				var startAngle = 0;
				var startValue = 0;

				function pointerAngle(event)
				{
					var point = svg.createSVGPoint();
					point.x = event.clientX;
					point.y = event.clientY;
					var local = point.matrixTransform(svg.getScreenCTM().inverse());
					return Math.atan2(local.y - pivot.y, local.x - pivot.x) * 180 / Math.PI;
				}

				handle.addEventListener("pointerdown", function(event)
				{
					dragPointer = event.pointerId;
					handle.setPointerCapture(event.pointerId);
					handle.classList.add("dragging");
					startAngle = pointerAngle(event);
					startValue = getValue();
					event.preventDefault();
				});
				handle.addEventListener("pointermove", function(event)
				{
					if(event.pointerId !== dragPointer)
					{
						return;
					}
					var delta = pointerAngle(event) - startAngle;
					delta = ((delta + 540) % 360) - 180;
					setValue(Math.max(min, Math.min(max, Math.round(startValue + direction * delta))));
				});
				function release(event)
				{
					if(event.pointerId === dragPointer)
					{
						dragPointer = null;
						handle.classList.remove("dragging");
					}
				}
				handle.addEventListener("pointerup", release);
				handle.addEventListener("pointercancel", release);
			}

			makeRotatable(document.getElementById("top-view"), document.getElementById("turret-rotation"), { x: 0, y: 0 }, 1,
				-AZIMUTH_LIMIT, AZIMUTH_LIMIT,
				function() { return rotation; },
				function(value) { if(value !== rotation) { rotation = value; scheduleTurret(); } });
			makeRotatable(document.getElementById("side-view"), document.getElementById("gun"), SIDE_PIVOT, -1,
				0, ELEVATION_MAX,
				function() { return elevation; },
				function(value) { if(value !== elevation) { elevation = value; scheduleTurret(); } });

			document.getElementById("bt_connect").addEventListener("click", connect_onclick);
			render();
		</script>
	</body>
</html>
