HOW TO BUILD A FIRMWARE FOR THE BROADCOM BCM963XX V1000H DSL MODEM/ROUTER

Warning!! These instructions do not build a fully functional modem/router. You will need to add a way to access the device into the code, before compiling it.
(i.e. SSH, Telnet, or some type of GUI)

These Instructions Explain:
 
 * How to install the BCM963xx GPL source code onto the Linux workstation 
 * How to build the image file for the Actiontec V1000H 
 * How to upgrade the device using the new firmware image file. 

NOTE: The following procedures are known to work with the following versions of Ubuntu 12.04 and Fedora 19.

The Actiontec V1000H Source Code can be obtained from the Actiontec support website at http://opensource.actiontec.com.


IMPORTANT: When entering the file name at the command prompt be sure to replace the question marks "??.???.??" with the release number "31.121L.11".


1. INSTALL THE BCM963XX ??.???.?? CONSUMER RELEASE V1000H.

$ su
                    (open a console window and login as "su")

 mkdir /tmp/consumer
                    (create a temp directory to install the source to)

$ cp bcm963xx_??.???.??_consumer_release_V1000H.tar.gz /tmp/consumer  
                    (copy source code to temp directory)

$ cd /tmp/consumer
                    (make /tmp/consumer the working directory)

$ tar xvfz bcm963xx_??.???.??_consumer_release_V1000H.tar.gz
                    (decompress and extract the files so they can be installed)

$ ./consumer_install
                    (install the source code and software tools)



2. BUILD THE DESIRED IMAGE.

$ cd /opt/bcm963xx_router
                    (make /opt/bcm963xx_router the working directory)

$ make PROFILE=96368BGW
                    (running this "make" command will build the firmware image)


YOU CAN UPLOAD THE IMAGE TO THE ROUTER BY ONE OF THESE METHODS


(They are in order of difficulty, with the simplest first.)



STANDARD WEB INTERFACE UPGRADE METHOD

1. Use a Web browser to access the V1000H at it's LAN IP Address

2. Navigate to the "Firmware Upgrade" page

3. Use the Browse button to locate the newly created image file
       (It will be in the /opt/bcm963xx_router/images directory)

4. Click the button to upgrade the Firmware

5. The device will reboot using the new firmware image 

6. To access the new firmware interface, use the (new) IP address, 192.168.1.254
(You may need to renew the IP address of the workstation, to reach 192.168.1.254)



UPLOAD THE IMAGE TO THE ROUTER USING THE BOOTLOADER METHOD

1. Enter Bootload mode, press enter at the console prompt...
   ( *** Press any key to stop auto run (1 seconds) *** )

2. Give the PC a static IP address: 192.168.1.100 & Subnet-Mask: 255.255.255.0

3. Use a Web browser to access the interface at the IP address, 192.168.1.1

4. In the Software File Name:box, select the (new) firmware image 
       (Look for it in the "images" directory at: /opt/bcm963xx_router/images )

5. Click the Update Software button.

TFTP METHOD

1.Enable LAN side telnet support using the router's current web interface.

2.Telnet to the router

3.Login.        (default username and password are both, admin)

4.tftp get serverip:serverfile localfile 
                (serverfile is the name of the file that resides on the server) 
                (localfile is the name of the file that resides on the router)


PLEASE NOTE: The machine-readable software source code provided here is for 
Programmers who may wish to alter or customize the code, and typically will not 
be of use to the vast majority of consumers. The software source code provided 
here is subject to Version 2 of the GPL or other GPL or LGPL licenses which 
are all included in the download and are available below and also at 
"http://www.gnu.org/licenses/" and as such, the software source code is 
distributed WITHOUT ANY WARRANTY and subject to the copyrights of the authors. 
Compiling errors due to lack of public libraries/tools in the customer's 
specific platform/OS are the responsibility of the user of this GPL source code.
