#!/usr/bin/env bash
set -eo pipefail
cd -- "$(dirname -- "$0")/.."
if [[ -z "${DISPLAY:-}" ]] || ! command -v xauth >/dev/null; then
  printf 'Affichage indisponible : vérifier DISPLAY et installer xauth.\n' >&2
  exit 1
fi
mkdir -p .runtime
touch .runtime/xauth
chmod 600 .runtime/xauth
xauth -i nlist "$DISPLAY" | sed 's/^..../ffff/' | xauth -f .runtime/xauth nmerge -
if [[ ! -s .runtime/xauth ]]; then
  printf 'Aucune autorisation X11 trouvée pour cet affichage.\n' >&2
  exit 1
fi
