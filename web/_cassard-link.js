// Liaison authentifiée avec le module central.
//
// Le mot de passe de pilotage n'est jamais transmis : il sert à dériver une clé
// (PBKDF2-HMAC-SHA256) gardée dans sessionStorage. Chaque trame est signée :
// "<charge>|<sequence>|<mac>", mac = 16 premiers hex de
// HMAC-SHA256(cle, nonce + "|" + sequence + "|" + charge), le nonce étant
// fourni par l'Arduino en réponse à "AUTH". Voir firmware/ModuleCentral/CassardAuth.h.
// Le module central (Arduino UNO R4 WiFi) sert les pages sur le port 80 et le
// WebSocket sur le port 81.
//
// CassardLink.open() renvoie un objet qui se comporte comme un WebSocket
// (readyState, send, close, onopen, onclose, onerror, onmessage) ; onopen n'est
// appelé qu'une fois le mot de passe accepté par le navire.

var CassardLink = (function()
{
	var KEY_STORAGE = "cassard.pilotKey";
	var SALT = "CASSARD-D614-pilotage";
	var ITERATIONS = 10000;
	var WEBSOCKET_PORT = 81;
	var WEBSOCKET_PROTOCOL = "cassard";
	var K = [
		0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
		0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
		0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
		0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
		0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
		0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
		0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
		0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
	];

	function utf8(text)
	{
		if(typeof TextEncoder !== "undefined")
		{
			return Array.prototype.slice.call(new TextEncoder().encode(text));
		}
		var binary = unescape(encodeURIComponent(text));
		var bytes = [];
		for(var i = 0; i < binary.length; i++)
		{
			bytes.push(binary.charCodeAt(i));
		}
		return bytes;
	}

	function sha256(bytes)
	{
		var h = [0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19];
		var message = bytes.slice();
		var bitLength = bytes.length * 8;
		message.push(0x80);
		while(message.length % 64 !== 56)
		{
			message.push(0);
		}
		message.push(0, 0, 0, 0, (bitLength >>> 24) & 255, (bitLength >>> 16) & 255, (bitLength >>> 8) & 255, bitLength & 255);

		var w = new Array(64);
		for(var offset = 0; offset < message.length; offset += 64)
		{
			for(var i = 0; i < 64; i++)
			{
				if(i < 16)
				{
					w[i] = (message[offset + i * 4] << 24) | (message[offset + i * 4 + 1] << 16) | (message[offset + i * 4 + 2] << 8) | message[offset + i * 4 + 3];
				}
				else
				{
					var s0 = ((w[i - 15] >>> 7) | (w[i - 15] << 25)) ^ ((w[i - 15] >>> 18) | (w[i - 15] << 14)) ^ (w[i - 15] >>> 3);
					var s1 = ((w[i - 2] >>> 17) | (w[i - 2] << 15)) ^ ((w[i - 2] >>> 19) | (w[i - 2] << 13)) ^ (w[i - 2] >>> 10);
					w[i] = (w[i - 16] + s0 + w[i - 7] + s1) | 0;
				}
			}
			var a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
			for(var j = 0; j < 64; j++)
			{
				var t1 = (hh + (((e >>> 6) | (e << 26)) ^ ((e >>> 11) | (e << 21)) ^ ((e >>> 25) | (e << 7))) + ((e & f) ^ (~e & g)) + K[j] + w[j]) | 0;
				var t2 = ((((a >>> 2) | (a << 30)) ^ ((a >>> 13) | (a << 19)) ^ ((a >>> 22) | (a << 10))) + ((a & b) ^ (a & c) ^ (b & c))) | 0;
				hh = g; g = f; f = e; e = (d + t1) | 0;
				d = c; c = b; b = a; a = (t1 + t2) | 0;
			}
			h[0] = (h[0] + a) | 0; h[1] = (h[1] + b) | 0; h[2] = (h[2] + c) | 0; h[3] = (h[3] + d) | 0;
			h[4] = (h[4] + e) | 0; h[5] = (h[5] + f) | 0; h[6] = (h[6] + g) | 0; h[7] = (h[7] + hh) | 0;
		}

		var digest = [];
		for(var k = 0; k < 8; k++)
		{
			digest.push((h[k] >>> 24) & 255, (h[k] >>> 16) & 255, (h[k] >>> 8) & 255, h[k] & 255);
		}
		return digest;
	}

	function hmac(key, message)
	{
		if(key.length > 64)
		{
			key = sha256(key);
		}
		var inner = [];
		var outer = [];
		for(var i = 0; i < 64; i++)
		{
			var byte = i < key.length ? key[i] : 0;
			inner.push(byte ^ 0x36);
			outer.push(byte ^ 0x5c);
		}
		return sha256(outer.concat(sha256(inner.concat(message))));
	}

	// PBKDF2-HMAC-SHA256 limité à un bloc de 32 octets (suffisant pour la clé).
	function pbkdf2(password, salt, iterations)
	{
		var u = hmac(password, salt.concat([0, 0, 0, 1]));
		var result = u.slice();
		for(var i = 1; i < iterations; i++)
		{
			u = hmac(password, u);
			for(var j = 0; j < 32; j++)
			{
				result[j] ^= u[j];
			}
		}
		return result;
	}

	function toHex(bytes)
	{
		var hex = "";
		for(var i = 0; i < bytes.length; i++)
		{
			hex += (bytes[i] < 16 ? "0" : "") + bytes[i].toString(16);
		}
		return hex;
	}

	function fromHex(hex)
	{
		var bytes = [];
		for(var i = 0; i + 1 < hex.length; i += 2)
		{
			bytes.push(parseInt(hex.substr(i, 2), 16));
		}
		return bytes;
	}

	function readKey()
	{
		try
		{
			var hex = sessionStorage.getItem(KEY_STORAGE);
			return hex && /^[0-9a-f]{64}$/.test(hex) ? fromHex(hex) : null;
		}
		catch(error)
		{
			return null;
		}
	}

	function storePassword(password)
	{
		sessionStorage.setItem(KEY_STORAGE, toHex(pbkdf2(utf8(password), utf8(SALT), ITERATIONS)));
	}

	function forget()
	{
		try
		{
			sessionStorage.removeItem(KEY_STORAGE);
		}
		catch(error)
		{
		}
	}

	// ---------- Connexion unique au bateau ----------
	//
	// La page coquille pilotage.html ouvre la connexion dès que le mot de passe est
	// saisi et la garde pendant toute la session ; les écrans (menu, propulsion,
	// tourelle…) s'y affichent dans des cadres qui restent chargés et partagent
	// cette connexion via CassardLink.open().

	var SCREEN_PATTERN = /^_Cassard\w+\.html$/;
	var STATUS_LABELS = {
		connexion: "Connexion…",
		connecte: "Connecté",
		reconnexion: "Reconnexion…",
		refuse: "Mot de passe refusé",
		reprise: "Session reprise par un autre poste",
		ferme: "Non connecté"
	};

	function shell()
	{
		try
		{
			return window.parent !== window && window.parent.CassardShell ? window.parent.CassardShell : null;
		}
		catch(error)
		{
			return null;
		}
	}

	function currentScreen()
	{
		var file = location.pathname.split("/").pop();
		return SCREEN_PATTERN.test(file) ? file : null;
	}

	// Écran ouvert seul (hors coquille) : on passe par la coquille, ou par la
	// connexion si aucun mot de passe n'a été saisi.
	function requireKey()
	{
		if(!readKey())
		{
			location.replace("index.html");
		}
		else if(currentScreen() && !shell())
		{
			location.replace("pilotage.html#" + currentScreen());
		}
	}

	// Appelée une seule fois, par la coquille. Reconnexion automatique toutes les
	// 2 s si la liaison tombe ; arrêt définitif si le mot de passe est refusé ou si
	// un autre poste prend la main (pas de reprise en boucle entre deux téléphones).
	function connect(options)
	{
		var key = readKey();
		var boat = { readyState: 0, status: "connexion" };
		var views = [];
		var socket = null;
		var nonce = null;
		var sequence = 0;
		var stopped = false;
		var everOpened = false;

		function setStatus(status)
		{
			boat.status = status;
			views.forEach(function(view) { view._status(status); });
			if(options && options.onstatus)
			{
				options.onstatus(status);
			}
		}

		function sendSigned(payload)
		{
			sequence++;
			socket.send(signFrame(key, nonce, sequence, payload) + "\r\n");
		}

		function stop(status)
		{
			stopped = true;
			if(socket)
			{
				socket.close();
			}
			setStatus(status);
		}

		function openSocket()
		{
			boat.readyState = 0;
			setStatus(everOpened ? "reconnexion" : "connexion");
			socket = new WebSocket("ws://" + location.hostname + ":" + WEBSOCKET_PORT + "/", WEBSOCKET_PROTOCOL);
			var thisSocket = socket;
			socket.onopen = function()
			{
				thisSocket.send("AUTH\r\n");
			};
			socket.onmessage = function(event)
			{
				var lines = String(event.data).split(/\r?\n/);
				for(var i = 0; i < lines.length; i++)
				{
					var line = lines[i].trim();
					if(line.indexOf("NONCE,") === 0)
					{
						nonce = line.substr(6);
						sequence = 0;
						sendSigned("PING");
					}
					else if(line === "AUTH,OK")
					{
						if(boat.readyState !== 1)
						{
							boat.readyState = 1;
							everOpened = true;
							setStatus("connecte");
							views.forEach(function(view) { view._open(); });
						}
					}
					else if(line === "AUTH,REPRISE")
					{
						stop("reprise");
						return;
					}
					else if(line === "AUTH,ERR")
					{
						if(boat.readyState !== 1)
						{
							stop("refuse");
							return;
						}
					}
					else if(line !== "")
					{
						views.forEach(function(view) { view._message(line); });
					}
				}
			};
			socket.onclose = function()
			{
				if(thisSocket !== socket)
				{
					return;
				}
				var wasOpen = boat.readyState === 1;
				boat.readyState = 3;
				if(wasOpen)
				{
					views.forEach(function(view) { view._close(); });
				}
				if(stopped)
				{
					// Les écrans viennent d'écrire « Non connecté » : on rétablit la raison
					// de l'arrêt (mot de passe refusé, session reprise, verrouillage).
					setStatus(boat.status);
				}
				else
				{
					setStatus("reconnexion");
					setTimeout(function() { if(!stopped) openSocket(); }, 2000);
				}
			};
		}

		boat.send = function(text)
		{
			if(boat.readyState === 1)
			{
				sendSigned(String(text).trim());
			}
		};
		boat.close = function()
		{
			stop("ferme");
		};
		boat.reconnect = function()
		{
			stopped = false;
			if(socket)
			{
				socket.onclose = null;
				socket.close();
			}
			openSocket();
		};
		// Vue pour un écran : même interface qu'un WebSocket (readyState, send,
		// onopen, onclose, onmessage) ; elle met aussi à jour le témoin #ws_state.
		boat.attach = function(screenDocument)
		{
			var view = { onopen: null, onclose: null, onerror: null, onmessage: null };
			Object.defineProperty(view, "readyState", { get: function() { return boat.readyState; } });
			view.send = boat.send;
			view.close = function() {};
			view._open = function() { if(view.onopen) view.onopen(); };
			view._close = function() { if(view.onclose) view.onclose(); };
			view._message = function(line) { if(view.onmessage) view.onmessage({ data: line }); };
			view._status = function(status)
			{
				var label = screenDocument && screenDocument.getElementById("ws_state");
				if(label)
				{
					label.textContent = STATUS_LABELS[status] || status;
					label.classList.toggle("connected", status === "connecte");
				}
			};
			views.push(view);
			// L'écran installe ses gestionnaires juste après : on le prévient ensuite.
			setTimeout(function()
			{
				view._status(boat.status);
				if(boat.readyState === 1)
				{
					view._open();
				}
			}, 0);
			return view;
		};

		openSocket();
		return boat;
	}

	// Dans un écran : vue sur la connexion unique de la coquille.
	function open()
	{
		var host = shell();
		if(host)
		{
			return host.attach(document);
		}
		requireKey();
		return { readyState: 0, send: function() {}, close: function() {} };
	}

	// Déconnecter : coupe la connexion, oublie la clé et revient à la page de connexion.
	function lock()
	{
		forget();
		var host = shell();
		if(host)
		{
			host.lock();
		}
		else
		{
			location.replace("index.html");
		}
		return false;
	}

	function signFrame(key, nonce, sequence, payload)
	{
		var mac = toHex(hmac(key, utf8(nonce + "|" + sequence + "|" + payload))).substr(0, 16);
		return payload + "|" + sequence + "|" + mac;
	}

	// Dans la coquille, les liens entre écrans changent d'écran sans recharger.
	if(typeof document !== "undefined")
	{
		document.addEventListener("click", function(event)
		{
			var host = shell();
			var anchor = event.target.closest ? event.target.closest("a[href]") : null;
			if(!host || !anchor)
			{
				return;
			}
			var href = anchor.getAttribute("href");
			if(SCREEN_PATTERN.test(href))
			{
				event.preventDefault();
				host.show(href);
			}
		}, true);
	}

	return {
		open: open,
		connect: connect,
		lock: lock,
		storePassword: storePassword,
		forget: forget,
		requireKey: requireKey,
		hasKey: function() { return readKey() !== null; },
		// Exposés pour les tests de concordance avec CassardAuth.h.
		_sha256: sha256,
		_hmac: hmac,
		_pbkdf2: pbkdf2,
		_signFrame: signFrame,
		_utf8: utf8,
		_toHex: toHex
	};
})();

if(typeof module !== "undefined")
{
	module.exports = CassardLink;
}
