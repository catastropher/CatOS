OS2Tools v1.3
Brandon Wilson

>What is it?
This is a suite of tools to sign your own OS and build an 8XU file out of it given a collection of one or more binary files containing the pages you want to make up your 8XU.

>How do I use them?
Run them without parameters, "/?", "/h", or "-h" (I forget).

>What all is in here?
Build8XU.exe - allows you to build a signed 8XU out of a collection of one or more binary files. You specify the file and offset into the file for each page you want to include in the 8XU. If you do not specify a key file to use for signing, the data is signed with the community-created 0005 key (to work in combination with Free83P) and will validate on a Free83P-installed 83+ series graphing calculator.

GenerateSignature.exe - signs a binary file and gives you the OS signature as a 0200h field as it would be in an 8XU OS upgrade file. If you do not specify a key file, it will default to the 0005 key.

Hex2ROM.exe - converts ZDS-style Intel Hex to a ROM image. This in combination with Build8XU can take ZDS output of an OS and sign it into an 8XU OS upgrade file for sending to a real calculator.

ExtractFrom8XU.exe - splits an 8XU OS upgrade file into individual pages. Run it to see more information.

>Can I get an example of how to use all this stuff?
Check OS2 development at the SVN repository at http://brandonw.net/svn/calcstuff/OS2/.

>How can I contact you?
brandonlw@gmail.com, brandonw.net, etc.

>Version History
v1.3 - Added ExtractFrom8XU.
v1.2 - Updated Build8XU to support signing 73 OSes as well as 83+/84+ series ones.
v1.1 - Updated Build8XU and GenerateSignature to use external key files if specified.
       Also added logging and stuff to Build8XU.
v1.0 - Initial release.
