<!doctype html>

<html lang="fr">
	<head>
		<meta charset="UTF-8">
		<meta name="viewport" content="width=device-width, initial-scale=1">
		<title>Centre de pilotage — Cassard</title>
		<link rel="stylesheet" href="app.css">
		<script src="_cassard-link.js"></script>
		<script>CassardLink.requireKey();</script>
	</head>
	<body>
		<header class="app-header">
			<a class="brand" href="_CassardMenu.php"><span class="brand-mark">C</span>CASSARD <span class="eyebrow" style="color:#b9c9da">PILOTAGE</span></a>
			<a class="header-menu" href="setup_info.php">Configuration réseau</a>
			<a class="header-menu" href="index.html" onclick="CassardLink.forget();">Verrouiller</a>
		</header>
		<main class="page-shell">
			<div class="eyebrow">Centre de commande</div>
			<h1 class="page-heading">Tableau de bord</h1>
			<p class="page-intro">Accédez rapidement aux systèmes du Cassard et à leurs outils de supervision.</p>

			<?if((int)ini_get("init_bcfg")){?>
			<div class="notice"><strong>Mode configuration actif.</strong> La carte Wi-Fi fonctionne en mode « SETUP » ; les services web ne sont pas disponibles.</div>
			<?}else{?>
			<section class="module-grid" aria-label="Modules de pilotage">
				<a class="module-card" href="_CassardPropulsion.php">
					<span class="module-index">01 / NAVIGATION</span>
					<h2>Propulsion &amp; direction</h2>
					<p>Pilotez la puissance moteur, la marche arrière et l’orientation.</p>
					<span class="module-link">Ouvrir le module&nbsp; →</span>
				</a>
				<a class="module-card" href="_CassardTourelle.php">
					<span class="module-index">02 / TOURELLE</span>
					<h2>Tourelle 100 mm</h2>
					<p>Réglez l’azimut et l’élévation de la tourelle.</p>
					<span class="module-link">Ouvrir le module&nbsp; →</span>
				</a>
				<a class="module-card" href="_CassardInstruments.php">
					<span class="module-index">03 / SUPERVISION</span>
					<h2>Instruments</h2>
					<p>Consultez les indicateurs de vitesse et de puissance.</p>
					<span class="module-link">Consulter les instruments&nbsp; →</span>
				</a>
				<a class="module-card" href="_CassardSonsEtLumieres.php">
					<span class="module-index">04 / ÉQUIPEMENTS</span>
					<h2>Sons &amp; lumières</h2>
					<p>Accédez au module des équipements sonores et lumineux.</p>
					<span class="module-link">Ouvrir le module&nbsp; →</span>
				</a>
				<a class="module-card secondary" href="_CassardDebug.php">
					<span class="module-index">OUTILS TECHNIQUES</span>
					<h2>Console de diagnostic</h2>
					<p>Envoyez et consultez des trames de protocole.</p>
					<span class="module-link">Accéder à la console&nbsp; →</span>
				</a>
			</section>
			<?}?>
			<footer class="page-footer">CASSARD · Interface locale de pilotage</footer>
		</main>
	</body>
</html>
