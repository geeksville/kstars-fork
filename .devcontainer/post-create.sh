#!/usr/bin/env bash
set -e

export USER=`whoami`

# the devcontainer mount of vscode/.local/share implicity makes the owner root (which is bad)
echo "Fixing permissions"
# Some containers might not have a .local directory at all, don't fail in that case
mkdir ~/.local | true
sudo chown -R $USER ~/.local

echo "Build stellarsolver libs"
cd /tmp
git clone --branch 2.6 https://github.com/rlancaste/stellarsolver.git
cd stellarsolver/linux-scripts/
./installStellarSolverLibraryQt5.sh 

echo "Run CMake"
cd /workspaces/kstars
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=RelWithDebInfo

echo "Environment created, you can now run: make -j16; sudo make install inside your devcontainer"