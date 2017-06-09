require 'rake/clean'

# Clobber generated header files

CLOBBER.include("include/Connector.h")
CLOBBER.include("include/Connector_Connection.h")
CLOBBER.include("include/DCPSTopic.h")
CLOBBER.include(".corto/dep.rb")
