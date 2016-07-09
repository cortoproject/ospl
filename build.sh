echo
echo "### Installing packages"
sudo apt-get --assume-yes install libxml2-dev libffi-dev rake flex bison

echo
echo "### Cloning dependencies..."
rm -rf deps
mkdir -p deps
cd deps
git clone https://github.com/cortoproject/corto
git clone https://github.com/cortoproject/c-binding
git clone https://github.com/cortoproject/xml
git clone https://github.com/cortoproject/corto-language
git clone https://github.com/cortoproject/json
git clone https://github.com/cortoproject/web
git clone https://github.com/cortoproject/admin
git clone https://github.com/cortoproject/ipso
git clone https://github.com/cortoproject/mqtt

echo
echo "### Building dependencies"
source corto/configure
rake -f corto/rakefile
corto build c-binding xml corto-language json web admin mqtt
cd ..

echo
echo "### Building ospl"
corto build . deps/ipso examples

echo
echo "### Done!"
echo
echo "To run an example, do:"
echo "./run.sh <ddsclient|ddsread|mqttbridge|mqttclient|osplmon|shapes|webbridge>"
echo
echo "Have fun!"
echo
