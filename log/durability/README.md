# OpenSplice durability logfile parser
The ospl/log/durability application uses the `corto/x` framework to interpret and
extract useful information from durability logfiles.

The expressions for the durability logfile are located in `durability.cx`.
This list is not exhaustive, and additional rules may be added in the future.

It is currently required to run this tool in an OpenSplice environment (the
`release.com` script needs to be sourced). Future versions will remove this
dependency.

## Build the project
To build the project, run the following commands:

```
sudo apt-get install rake libffi-dev libxml2-dev flex bison
git clone https://github.com/cortoproject/corto
git clone https://github.com/cortoproject/c-binding
git clone https://github.com/cortoproject/json
git clone https://github.com/cortoproject/xml
git clone https://github.com/cortoproject/corto-language
git clone https://github.com/cortoproject/x
git clone https://github.com/cortoproject/ospl
cd corto
source configure
rake
cd ..
corto build c-binding json
corto build xml corto-language x ospl
```

## Run the project
```
corto run ospl/log/durability durability.log
```

## Visitors
The `corto/x` framework supports using different visitors so that a logfile can
be analyzed in many different ways. Currently the durability application only
has one visitor that is under development. Future versions will allow a user to
specify which visitor needs to be run.
