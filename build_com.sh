# Download the latest version of redbean
# wget -O augame.com.temp https://cdn.dump.garden/redbean-3.0.0.com
cp augame.com.temp augame.com
# original source: https://redbean.dev/redbean-3.0.0.com

# Run the compile script
# chmod +x compile.sh
# ./compile.sh

# We don't want all the files in the web folder, so move them out temporarily
mv web web.temp
mkdir web
zip -0 augame.com web
rm -rf web
mv web.temp web

# Zip only the necessary files
zip -r augame.com \
   .init.lua \
   404.html \
   index.html \
   web/index.html \
   web/econfigs/* \
   web/tests \
   web/scripts/* \
   web/resources/page/* \
   README.md \
   LICENSE

# Zip these with no compression (-0) to avoid issues with fetching from the client
zip -0 augame.com web/build/main.wasm web/build/main.js

cp web/resources/page/Diogenex10.gif redbean.png
zip -0 augame.com redbean.png
rm redbean.png

# Run the augame redbean server
chmod +x augame.com
./augame.com