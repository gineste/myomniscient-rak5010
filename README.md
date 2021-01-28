# firmware-c

Template for firmware projects which contain mainly C language.

### Architecture

- **docs**
  - fw
    - .zip, code examples, flowchart, documentation, environmental information, ...
  - hw
    - schematic, datasheets, ...
  - meas
    - power consumption, battery profile, ...
  - project
    - specifications, planning, goals, ... 
- **src**
  - updateVersion.sh (script updating commit and build infos in Version.h)
  - Version.h
  - project / source files
- **output**
  - hex
    - all version .hex files
  - change-log.txt (release note)
  - GenerateHex.bat (script generating hex file with version number)
- **.gitignore**
- **README.md** (should contain a description of the project.)

### Configure IDE for the scripts

We need to configure the IDE to run the scripts automatically.

In **pre-build step**, for any build configuration, enter this command:	sh ../updateVersion.sh

In **post-build step**, for release build configuration, enter this command:	"../../output/GenerateHex.bat" ${ProjName}