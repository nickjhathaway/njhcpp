#!/usr/bin/env python3

import shutil, os, argparse, sys, stat
import os, subprocess, sys

projectDir = os.path.dirname(os.path.abspath(__file__))
setupToolsDir = os.path.join(projectDir, "cppSetupTools")
if not os.path.exists(os.path.join(setupToolsDir, "setup.py")):
    if not os.path.exists(os.path.join(projectDir, ".git")):
        sys.exit("cppSetupTools is missing; it is a git submodule, so get this project with: git clone --recursive <url>")
    subprocess.run(["git", "submodule", "update", "--init", "cppSetupTools"], cwd=projectDir, check=True)
sys.path.append(os.path.join(setupToolsDir, "scripts", "pyUtils"))
sys.path.append(os.path.join(setupToolsDir, "scripts", "setUpScripts"))
from utils import Utils
from genFuncs import genHelper
def main():
    name = "njhcpp"
    #libs = "boost_filesystem:1_68_0,pstreams:RELEASE_1_0_1,jsoncpp:1.8.3,cppitertools:v0.2.1,zlib:1.2.11"
    libs = "boost_filesystem:1_78_0,pstreams:RELEASE_1_0_1,jsoncpp:1.9.7,cppitertools:v2.0,zlib:1.3.1,nlohmann_json:v3.12.0,catch2:v3.7.1"
    args = genHelper.parseNjhConfigureArgs()
    if Utils.isMac():
        cmd = genHelper.mkConfigCmd(name, libs, sys.argv, "-lpthread,-lz")
    else:
        cmd = genHelper.mkConfigCmd(name, libs, sys.argv, "-lpthread,-lz,-lrt")
    Utils.run(cmd)
    
main()


