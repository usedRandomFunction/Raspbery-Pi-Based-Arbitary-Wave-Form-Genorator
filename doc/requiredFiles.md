# This folder states what is expected to be on the SD by the kernal

- user/apps/mainmenu.img As the name implys, the main menu of the system (Not inculded compile it from /user/mainmenu)
- user/apps/mainmenu.cfg Config file for the exicutable (stored in user/mainmenu)
- config/system.cfg Used to store settings to do with the OS (Defult provieded in disk/config)
- config/outputs.cfg Used to store settings common to all DACs (Example provieded in disk/config)
- config/channel`N`.cfg Used to store the hardware capiblitys of the `N`the channel, [1, 4] inclusive<br> (Not included, as these files represent details about each physical DAC card / channel)