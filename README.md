mdgcm
===

The purpose of this tool is to make it easier to extract and build GCM discs with command line tools. Although GCM discs technically should always be a fixed size, this program as it stands will leave as little free space as possible. This means re-built discs can be anywhere from 200MB to the full 1.4GB.

### Commands

To extract all the files from the disc into a directory:

    extract disc.gcm output/directory/path

To build a disc you must pass in a directory that has had the contents of the disc extracted to it previously. If it detects missing files or improper structure it will not build anything.
    
    build previously/extracted/directory output.gcm
    
Files will simply list the contents of the disc to the console.

    files disc.gcm

#### Aliases

For convinience there are single letter aliases for all the commands.

|Command|Alias|
|-------|----:|
|extract|e|
|build|b|
|files|f|
