#!/bin/bash
#
# Script to make an installable package of the plugin.
#
# Uses xcodebuild, pkgbuild and productbuild.
#


recreate_dir() {
  if test -d "$1"; then
    sudo chmod -R u+w "$1"
    sudo rm -rf "$1"
  fi
  mkdir -p "$1"/Root
}

# Create a clean install directory...
recreate_dir build/Package-qlwindowsapps
recreate_dir build/Package-WindowsAppsImporter

# Install into this directory...
xcodebuild -workspace "$PWD/QLWindowsApps.xcworkspace" \
           -scheme QLWindowsApps \
           -configuration Release \
           install \
           DSTROOT="$PWD/build/Package-qlwindowsapps/Root"
xcodebuild -workspace "$PWD/QLWindowsApps.xcworkspace" \
           -scheme WindowsAppsImporter \
           -configuration Release \
           install \
           DSTROOT="$PWD/build/Package-WindowsAppsImporter/Root"

# Extract the version number from the project...
ver=$(git describe | sed 's/release_//')
if [[ ! ( $? -eq 0 ) ]]; then
  ver=$(/usr/libexec/PlistBuddy -c "Print:CFBundleShortVersionString" "MicrosoftBinaries/Info.plist")
fi

# Make the package with pkgbuild and the product distribution with productbuild...
echo pkgbuild...
pkgbuild --identifier       com.danielecattaneo.qlgenerator.qlwindowsapps   \
         --version          "$ver"   \
         --root             build/Package-qlwindowsapps/Root   \
         --scripts          ./MicrosoftBinaries/PackageResources/Scripts \
         "./QLWindowsApps.pkg"   
pkgbuild --identifier       com.danielecattaneo.WindowsAppsImporter   \
         --version          "$ver"   \
         --root             build/Package-WindowsAppsImporter/Root   \
         --scripts          ./WindowsAppsImporter/PackageResources/Scripts \
         "./WindowsAppsImporter.pkg"
productbuild --distribution     ./PackageResources/Distribution.xml   \
             --resources        ./PackageResources/Resources \
             --package-path     ./ \
             "./QLWindowsApps-$ver.pkg"
rm ./QLWindowsApps.pkg
rm ./WindowsAppsImporter.pkg
