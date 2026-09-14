#!/bin/bash
set -euo pipefail

# Update the bundled manual checkout. Resolve the directory relative to this
# script, so that a failed cd can never leave the git commands running in the
# enroute repository itself.
cd "$(dirname "$0")/3rdParty/enrouteManual"
git fetch origin
git checkout html
git reset --hard origin/html
