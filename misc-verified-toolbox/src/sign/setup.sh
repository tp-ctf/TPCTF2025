#!/bin/bash

set -euo pipefail

cd "$(dirname "$(realpath "$0")")"

password="$(tr -dc A-Za-z0-9 </dev/urandom | head -c 32 || true)"
echo "$password" > password

rm -f keystore.jks

keytool -genkeypair \
    -alias signature \
    -dname 'CN=Unknown, OU=Unknown, O=TPCTF, L=Unknown, ST=Unknown, C=CN' \
    -keyalg DSA \
    -keysize 3072 \
    -validity 1000 \
    -keystore keystore.jks \
    -storepass "$password" \

keytool -exportcert \
    -alias signature \
    -file ../app/src/main/resources/signature.crt \
    -keystore keystore.jks \
    -storepass "$password" \
