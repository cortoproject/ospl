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
git clone https://github.com/cortoproject/json
git clone https://github.com/cortoproject/xml
git clone https://github.com/cortoproject/corto-language
git clone https://github.com/cortoproject/idl

echo
echo "### Building dependencies"
source corto/configure
rake -f corto/rakefile
corto build c-binding json
corto build xml corto-language idl
cd ..

echo
echo "### Building ospl and examples"
corto build . examples

echo
echo "### Done!"
echo
echo "To run an example, do:"
echo "corto run <ospl/shapes|ospl/read>"
echo
echo "Note: if you did not source this script, you'll have to set the corto environment first, with:"
echo "source deps/corto/configure"
echo
echo "Have fun!"
echo
