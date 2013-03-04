MicrosoftBinaries
=================

**MicrosoftBinaries** is a QuickLook Plugin for Microsoft Windows DLL and EXE files, intended as a 
companion to Wine on Mac OS X. It features:

- Programs icons in the Finder for EXEs. High resolution PNG icons are fully supported (thanks to the 
  builtin support in Mac OS X); in the Finder, Windows EXEs look just like native apps, without the 
  usual QuickLook-imposed frame.
- Windows 3.1 16 bit EXEs (a.k.a. New Executables), 32 bit EXEs (most common kind, a.k.a. Portable 
  Executables) and 64 bit EXEs (PE-Plus) are all supported. It flags 16 bit EXEs in the preview.
- In the preview window, the version information data embedded in the EXE is shown. Supports 
  multilingual version resources.
- It's fast.
- The resource reader is based on a modified version of icoutils by Oskar Liljeblad, expanded,
  debugged, and statically linked into the plugin.

Drawbacks/Known Bugs:

- Since there's no way and no API available for removing the frame for QL thumbnails, to work around
  this and "remove" the frame anyway, MicrosoftBinaries simply sets the file's custom icon (the one you
  can change in the Get Info window in the Finder) to the application's icon. Thus, filesystems that do
  not support Finder metadata writing (like native NTFS) are stuck in QL-framed land, and I can't do 
  anything about it.
- Like Oskar says in the icoutils webpage, "There is no relocation support for 16-bit (NE) binaries. If 
  you find a massive deposit of such binaries, let me know." I couldn't find any relocated 16 bit binary 
  myself, maybe because I didn't search hard enough, maybe that kind of binary is just uncommon.
- The plugin, like icoutils, still chokes a bit on 16 bit resources, even when it extracts them fine.
  But I guess it's not a high-priority issue.
- It will take a looong time generating previews on big files located across a slow link because
  the existing icoutils code expects the whole binary to be in memory at once, which is a bit 
  overzealous. I plan to have a permanent fix for this in the future; for now, the plugin will refuse to
  QuickLook anything too big that's located on a network share.
