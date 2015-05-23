#!/usr/bin/env python

import shutil, os, argparse, sys, stat
from setUpScripts.utils import Utils
from setUpScripts.genFuncs import genHelper

def main():
    name = "bibcpp"
    libs = "cppitertools,zi_lib,boost,armadillo,jsoncpp,pstreams"
    args = genHelper.parseNjhConfigureArgs()
    cmd = genHelper.mkConfigCmd(name, libs, sys.argv)
    Utils.run(cmd)
    
main()


