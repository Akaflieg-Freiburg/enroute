#!/bin/python3

from subprocess import Popen, PIPE
import json
import os


def qtattributionsscanner(path):
    process = Popen([scanner, "--output-format", "json", path], stdout=PIPE)
    (output, err) = process.communicate()
    process.wait()
    return json.loads(output)

def get_name(employee):
    return employee['Name'].upper()


# Acquire data
data = []

qtbaseDir = os.environ.get('Qt6_DIR_BASE')

# The scanner lives in the desktop kit, whose directory name depends on the platform
scanner = next(qtbaseDir+"/"+kit+"/libexec/qtattributionsscanner"
               for kit in ("gcc_64", "macos")
               if os.path.exists(qtbaseDir+"/"+kit+"/libexec/qtattributionsscanner"))

# Include data from all the Qt modules that we use. Keep this list in sync
# with the find_package(Qt6 ...) calls in CMakeLists.txt.
qtModules = ["qt5compat", "qtbase", "qtconnectivity", "qtdeclarative",
             "qthttpserver", "qtimageformats", "qtlocation", "qtpositioning",
             "qtsensors", "qtserialport", "qtspeech", "qtsvg", "qttranslations",
             "qtwebview"]
for module in qtModules:
    for entry in qtattributionsscanner(qtbaseDir+"/Src/"+module):
        # Only credit what ends up in the shipped libraries. The scanner also
        # reports components of Qt's examples, tests and build tools.
        parts = entry.get("QtParts", [])
        if parts and "libs" not in parts:
            continue
        data.append(entry)

# Include data from modules in 3rdParty
for root,directors,files in os.walk("3rdParty"):
    for file in files:
        if file == "qt_attribution.json":
            continue
        if file.endswith("_attribution.json"):
            with open(root+"/"+file) as json_file:
                x = json.load(json_file)
                if isinstance(x, list):
                    data += x
                else:
                    data.append(x)

# Sort data
data.sort(key=get_name)

# Generate output
rstString = ""
htmlString = ""
for entry in data:
    if entry["Homepage"] != "":
        rstString += "- `{} <{}>`_. {}.\n".format(entry["Name"], entry["Homepage"], entry["License"])
        htmlString += "<li><a href='{}'>{}</a>. {}.</li>\n".format(entry["Homepage"], entry["Name"], entry["License"])
    else:
        rstString += "- {}. {}.\n".format(entry["Name"], entry["License"])
        htmlString += "<li>{}. {}.</li>\n".format(entry["Name"], entry["License"])
        

with open("generatedSources/licenses_overview.rst", "w") as rstFile:
    rstFile.write(rstString)
    rstFile.close()

with open("generatedSources/licenses_overview.html", "w") as htmlFile:
    htmlFile.write(htmlString)
    htmlFile.close()
