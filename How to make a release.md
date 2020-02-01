# How to make a release

This is a reminder for myself, so I do not forget what needs to be done when making a release.

## 1. Version number

Update the version number in the file **CMakeLists.txt**, found in the top-level directory. The version number is found in the section "Project data".

## 2. What's new

Update the file **src/text/whatsnew.html.in**. The content of this file will be shown to the user after an update.

## 3. Make release

Make a new release on GitHub.

## 4. Compile and upload

Compile and upload APK/AAB to Google.

## 5. Provide Qt source code to users.

We are required by the GPL to make to source code of Qt available to our users. This can be done as follows.

```bash
cd <build directory>
make DistributeQtSource
```


