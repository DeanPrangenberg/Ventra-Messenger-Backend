#!/bin/bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

chmod +x "$SCRIPT_DIR/nodeSetup/cleanK3s.sh"
chmod +x "$SCRIPT_DIR/startDeploy.sh"

"$SCRIPT_DIR/nodeSetup/cleanK3s.sh"
"$SCRIPT_DIR/startDeploy.sh"