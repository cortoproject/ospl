# ospl
Generic connector for OpenSplice 

## Building on Ubuntu systems
The repository contains a build.sh script that downloads & installs all dependencies and builds the project. The script will request  your password to install packages with apt-get. Right now, only Ubuntu installations are supported by this build script. Before invoking the buildscript, ensure that OpenSplice is available from the environment ($OSPL_HOME must be set). Then invoke (from the `ospl` repository root):
```
./build.sh
```
The build process will take a few minutes, depending on the internet connection. When the build has finished, you can use the `run.sh` script to run the tools. To run the osplmon tool, run (from the `ospl` repository root):
```
./run.sh osplmon
```

## Building on non-Ubuntu systems
If you are building on a non-ubuntu system, you'll have to build the project manually. Use the package manager to install the following packages (if not already installed):
 * libffi-dev
 * libxml2-dev
 * flex
 * bison
 * rake
 
Once these packages are installed, run the following commands to clone dependencies (you might want to run these commands in a new directory to not contaminate the repository directory):
```
git clone https://github.com/cortoproject/corto
git clone https://github.com/cortoproject/c-binding
git clone https://github.com/cortoproject/json
git clone https://github.com/cortoproject/xml
git clone https://github.com/cortoproject/corto-language
git clone https://github.com/cortoproject/x
```

To build the dependencies, run the following commands (from the same directory where the clone commands where invoked):
```
source corto/configure
rake -f corto/rakefile
corto build c-binding json
corto build xml corto-language x
```

Finally, build the ospl project itself, from the `ospl` repository root:
```
corto build .
```

## Run the examples
To run an example, first build the example, then run it with `corto run`:
```
corto build examples/osplread
corto run osplread "*.*"
```

Some examples may require additional dependencies. To be able to run all examples, also install the following packages:
```
git clone https://github.com/cortoproject/mqtt
git clone https://github.com/cortoproject/web
git clone https://github.com/cortoproject/admin
git clone https://github.com/cortoproject/influxdb
corto build mqtt web admin influxdb
```
