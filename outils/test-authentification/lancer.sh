#!/bin/sh
# Vérifie que CassardAuth.h (Arduino), _cassard-link.js (navigateur) et le
# générateur Python calculent les mêmes clés et signatures.
# Prérequis : clang++ (ou g++), node, python3.
set -e
ICI=$(cd "$(dirname "$0")" && pwd)
RACINE=$(cd "$ICI/../.." && pwd)
LIEN="$RACINE/web/_cassard-link.js"
SORTIE=$(mktemp -d)
trap 'rm -rf "$SORTIE"' EXIT

${CXX:-c++} -std=c++11 -Wall -Wextra -I"$RACINE/firmware/ModuleCentral" "$ICI/test_auth.cpp" -o "$SORTIE/test_auth"
PY=$(python3 -I "$ICI/signer.py")
TRAME_PY=$(echo "$PY" | sed -n 1p)
CLE_PY=$(echo "$PY" | sed -n 2p)
TRAME_JS=$(node "$ICI/signer.js" "$LIEN" frame)
PING_JS=$(node "$ICI/signer.js" "$LIEN" ping)
CLE_JS=$(node "$ICI/signer.js" "$LIEN" key)

[ "$TRAME_PY" = "$TRAME_JS" ] && echo "OK   signature JS == Python" || { echo "FAIL signature"; exit 1; }
[ "$CLE_PY" = "$CLE_JS" ] && echo "OK   PBKDF2 JS == Python" || { echo "FAIL PBKDF2"; exit 1; }
"$SORTIE/test_auth" "$TRAME_PY" "$PING_JS"
