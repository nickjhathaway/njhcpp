#!/usr/bin/env python

import shutil, os, argparse, sys, stat
sys.path.append("scripts/pyUtils")
sys.path.append("scripts/setUpScripts")
from utils import Utils
from genFuncs import genHelper
def main():
    name = "bibcpp"
    libs = "boost_filesystem:1_60_0,pstreams:RELEASE_1_0_1,jsoncpp:1.7.7,cppitertools:v0.2.1,zlib:1.2.11"
    args = genHelper.parseNjhConfigureArgs()
    if Utils.isMac():
        cmd = genHelper.mkConfigCmd(name, libs, sys.argv, "-lpthread,-lz")
    else:
        cmd = genHelper.mkConfigCmd(name, libs, sys.argv, "-lpthread,-lz,-lrt")
    Utils.run(cmd)
    
main()


