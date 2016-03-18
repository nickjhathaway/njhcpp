#!/usr/bin/env python

import shutil, os, argparse, sys, stat
sys.path.append("scripts/pyUtils")
sys.path.append("scripts/setUpScripts")
from utils import Utils
from genFuncs import genHelper
def main():
    name = "bibcpp"
    libs = "boost:1_60_0,pstreams:RELEASE_0_8_1,jsoncpp:1.6.5,cppitertools:v0.1"
    args = genHelper.parseNjhConfigureArgs()
    if Utils.isMac():
        cmd = genHelper.mkConfigCmd(name, libs, sys.argv, "-lpthread,-lz")
    else:
        cmd = genHelper.mkConfigCmd(name, libs, sys.argv, "-lpthread,-lz,-lrt")
    Utils.run(cmd)
    
main()


