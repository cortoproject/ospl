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
git clone https://github.com/cortoproject/xml
git clone https://github.com/cortoproject/corto-language
git clone https://github.com/cortoproject/json
git clone https://github.com/cortoproject/web
git clone https://github.com/cortoproject/admin
git clone https://github.com/cortoproject/ipso
git clone https://github.com/cortoproject/mqtt
```

To build the dependencies, run the following commands (from the same directory where the clone commands where invoked):
```
source corto/configure
rake -f corto/rakefile
corto build c-binding xml corto-language json web admin mqtt
```

Finally, build the ospl project itself, from the `ospl` repository root:
```
corto build . <path to ipso repo> examples
```
Note that the `ipso` repository needs to be built after the `ospl` project, as it currently depends on the `ospl/idl` package.

## Run the tools
To run osplmon, type (from the `ospl` repository root):
```
./run osplmon
```
To run the webbridge, type (from the `ospl` repository root):
```
./run webbridge
```
Then navigate to `http://localhost:9090/admin/` to browse through the admin data. Double-click on a row to navigate its contents (the discovery database is located under `db`).

To use the REST interface, use the `http://localhost:9090/api` endpoint. Here are a few simple REST queries to try out:
```
// Dump all identifiers of the entire discovery database
http://localhost:9090/api/db?select=//*

// Select objects in the "db" scope, display identifiers and metadata
http://172.28.128.3:9090/api/db?select=*&meta=true

// Select objects in the "db" scope, display identifiers and value
http://172.28.128.3:9090/api/db?select=*&value=true
```
