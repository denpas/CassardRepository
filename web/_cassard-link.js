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

	function requireKey()
	{
		if(!readKey())
		{
			location.replace("index.html");
		}
	}

	function signFrame(key, nonce, sequence, payload)
	{
		var mac = toHex(hmac(key, utf8(nonce + "|" + sequence + "|" + payload))).substr(0, 16);
		return payload + "|" + sequence + "|" + mac;
	}

	function open()
	{
		var key = readKey();
		var link = { readyState: 0, error: "", onopen: null, onclose: null, onerror: null, onmessage: null };
		var socket = new WebSocket("ws://" + location.hostname + ":" + WEBSOCKET_PORT + "/", WEBSOCKET_PROTOCOL);
		var nonce = null;
		var sequence = 0;

		function sendSigned(payload)
		{
			sequence++;
			socket.send(signFrame(key, nonce, sequence, payload) + "\r\n");
		}

		function fail(message)
		{
			link.error = message;
			socket.close();
		}

		socket.onopen = function()
		{
			if(!key)
			{
				fail("Mot de passe requis");
				return;
			}
			socket.send("AUTH\r\n");
		};
		socket.onmessage = function(event)
		{
			var lines = String(event.data).split(/\r?\n/);
			for(var i = 0; i < lines.length; i++)
			{
				var line = lines[i].trim();
				if(line.indexOf("NONCE,") === 0)
				{
					if(link.readyState === 1)
					{
						fail("Session reprise par un autre poste");
						return;
					}
					nonce = line.substr(6);
					sequence = 0;
					sendSigned("PING");
				}
				else if(line === "AUTH,OK")
				{
					if(link.readyState !== 1)
					{
						link.readyState = 1;
						if(link.onopen)
						{
							link.onopen();
						}
					}
				}
				else if(line === "AUTH,REPRISE")
				{
					fail("Session reprise par un autre poste");
					return;
				}
				else if(line === "AUTH,ERR")
				{
					if(link.readyState !== 1)
					{
						fail("Mot de passe refusé");
						return;
					}
				}
				else if(line !== "" && link.onmessage)
				{
					link.onmessage({ data: line });
				}
			}
		};
		socket.onerror = function(event)
		{
			if(link.onerror)
			{
				link.onerror(event);
			}
		};
		socket.onclose = function()
		{
			link.readyState = 3;
			if(link.onclose)
			{
				link.onclose();
			}
			var status = document.getElementById("ws_state");
			if(link.error && status)
			{
				status.textContent = link.error;
			}
		};

		link.send = function(text)
		{
			if(link.readyState === 1)
			{
				sendSigned(String(text).trim());
			}
		};
		link.close = function()
		{
			socket.close();
		};
		return link;
	}

	return {
		open: open,
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
