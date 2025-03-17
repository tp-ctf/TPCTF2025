#!/bin/bash

set -euo pipefail

base="$(dirname "$(realpath "$0")")"

jarsigner "$1" signature -keystore "$base/keystore.jks" -storepass "$(cat "$base/password")"
