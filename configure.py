#!/usr/bin/env python3

import shutil, os, argparse, sys, stat
sys.path.append("scripts/pyUtils")
sys.path.append("scripts/setUpScripts")
from utils import Utils
from genFuncs import genHelper
def main():
    name = "njhcpp"
    #libs = "boost_filesystem:1_68_0,pstreams:RELEASE_1_0_1,jsoncpp:1.8.3,cppitertools:v0.2.1,zlib:1.2.11"
    libs = "boost:1_70_0,pstreams:RELEASE_1_0_1,jsoncpp:1.8.3,cppitertools:v0.2.1,zlib:1.2.11"
    args = genHelper.parseNjhConfigureArgs()
    if Utils.isMac():
        cmd = genHelper.mkConfigCmd(name, libs, sys.argv, "-lpthread,-lz")
    else:
        cmd = genHelper.mkConfigCmd(name, libs, sys.argv, "-lpthread,-lz,-lrt")
    Utils.run(cmd)
    
main()


