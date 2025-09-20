# Download the latest version of redbean
wget -O augame.com "http://cdn.dump.garden/redbean-3.0.0.com"
# original source: https://redbean.dev/redbean-3.0.0.com

# Run the compile script
chmod +x compile.sh
./compile.sh

# Zip only the necessary files
zip -r augame.com \
   .init.lua \
   index.html \
   web/index.html \
   web/econfigs \
   web/tests \
   web/scripts \
   web/resources/page \
   README.md \
   LICENSE

# Zip these with no compression (-0) to avoid issues with fetching from the client
zip -0 augame.com web/build/main.wasm web/build/main.js

# Run the augame redbean server
chmod +x augame.com
./augame.com