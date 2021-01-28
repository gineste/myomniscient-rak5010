# firmware-c

Template for firmware projects which contain mainly C language.

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
