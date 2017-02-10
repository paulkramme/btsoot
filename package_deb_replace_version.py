import sys

fullversion = sys.argv[1]
path = f"btsoot_{fullversion}/DEBIAN/control"
version = fullversion[1:]
version = version[1]
control_content = f"""Package: btsoot
Version: {version}
Section: base
Priority: optional
Architecture: i386
Depends: build-essential
Maintainer: Paul Kramme <pjkramme@gmail.com>
Description: BTSOOT
 Folder redundancy offsite-backup utility.
"""
print("DEB PACKAGE VERSION REPLACER")
# yes, i wrote a tool for this...
with open(path, "a") as f:
	f.write(control_content)
print("Done.")
