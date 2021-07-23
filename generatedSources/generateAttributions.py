#!/bin/python3

from subprocess import Popen, PIPE
import json
import os


def qtattributionsscanner(path):
    process = Popen(["qtattributionsscanner-qt5", "--output-format", "json", path], stdout=PIPE)
    (output, err) = process.communicate()
    process.wait()
    return json.loads(output)

def get_name(employee):
    return employee['Name'].upper()


# Acquire data
data = []

# Include data from all the Qt modules that we use
data += qtattributionsscanner("../qt5/qtandroidextras")
data += qtattributionsscanner("../qt5/qtbase")
data += qtattributionsscanner("../qt5/qtdeclarative")
data += qtattributionsscanner("../qt5/qtlocation")
data += qtattributionsscanner("../qt5/qtquickcontrols2")
data += qtattributionsscanner("../qt5/qtsvg")
data += qtattributionsscanner("../qt5/qttranslations")
data += qtattributionsscanner("../qt5/qtwebview")
data += qtattributionsscanner("../qt5/qtx11extras")

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
        

with open("3rdParty/enrouteText/manual/04-appendix/licenses_overview.rst", "w") as rstFile:
    rstFile.write(rstString)
    rstFile.close()

with open("generatedSources/licenses_overview.html", "w") as htmlFile:
    htmlFile.write(htmlString)
    htmlFile.close()
