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

echo
echo "### Building dependencies"
source corto/configure
rake -f corto/rakefile
corto build c-binding xml corto-language json web admin
cd ..

echo
echo "### Building ospl"
corto build . idl health examples/Shapes examples/osplmon examples/webadmin

echo
echo "### Done!"
echo
echo "To use, run one of the following commands from this directory:"
echo "./run.sh osplmon"
echo "./run.sh webadmin"
echo "./run.sh demo"
echo
echo "Have fun!"
echo
