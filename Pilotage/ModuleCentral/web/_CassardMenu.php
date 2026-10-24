<?php
	session_start();
	$identifiant = $_POST['identifiant'];
	$_SESSION['identifiant'] = $identifiant;
?>

<html>
	<head>
		<title>PHPoC Shield for Arduino</title>
		<meta name="viewport" content="width=device-width, initial-scale=0.7">
		<style>
			body { text-align:center; }
			a { font-size: 20pt; }
			a:link { text-decoration: none;}
			a:hover { text-decoration: underline;}
		</style>
	</head>
	<body bgcolor=#FEFFD3>
		<table width=100% height=100 border=1>
			<tr>
				<td bgcolor=#FC8316>
					<h1 style="text-align:center">Menu</h1>
				</td>
			</tr>
		</table>
		<br><br>
		<a href="setup_info.php">Setup</a>

		<?if((int)ini_get("init_bcfg")){?>

		<br><br><br>
		<font color="red">
			La carte WIFI fonctionne en mode "SETUP"<br>
			Les services WEB ne sont pas disponibles.
		</font>

		<?}else{?>

		<br><br><br>
		<a href="_CassardPropulsion.php">CASSARD Propulsion</a>

		<br><br><br>
		<a href="_CassardTourelle.php">CASSARD Tourelle 100mm</a>

		<br><br><br>
		<a href="_CassardSonsEtLumieres.php">CASSARD Sons et Lumières</a>

		<br><br><br>
		<a href="_CassardInstruments.php">CASSARD Instruments</a>
				
		<br><br><br>
		<a href="_CassardDebug.php">CASSARD Debug</a>
		<?}?>
	</body>
</html>

