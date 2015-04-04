#!/bin/bash
#
# Script to make an installable package of the plugin.
#
# Uses xcodebuild, pkgbuild and productbuild.
#

# Create a clean install directory...
if test -d build/Package; then
	sudo chmod -R u+w build/Package
	sudo rm -rf build/Package
fi
mkdir -p build/Package/Root
mkdir -p build/Package/Resources

# Install into this directory...
xcodebuild -workspace "$PWD/QLWindowsApps.xcworkspace" \
           -scheme QLWindowsApps \
           -configuration Release \
           install \
           DSTROOT="$PWD/build/Package/Root"

# Extract the version number from the project...
ver=$(/usr/libexec/PlistBuddy -c "Print:CFBundleShortVersionString" "MicrosoftBinaries/Info.plist")

# Make the package with pkgbuild and the product distribution with productbuild...
echo pkgbuild...
pkgbuild --identifier       com.danielecattaneo.qlgenerator.qlwindowsapps   \
         --version          "$ver"   \
         --root             build/Package/Root   \
         "./PluginPackage.pkg"   
productbuild --distribution     ./PackageResources/Distribution.xml   \
             --package-path     ./ \
             "./QLWindowsApps-$ver.pkg"
rm ./PluginPackage.pkg
