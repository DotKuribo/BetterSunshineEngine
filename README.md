# BetterSunshineEngine
Better Sunshine Engine is a core module powered by Kuribo, providing an extensive interface for composable, high-level modifications of Super Mario Sunshine. Better Sunshine Engine also provides a user-oriented layer of built-in patches for the game, such as 60-FPS, 16:9 and 21:9 aspect ratios, and more. These are made accessible through a custom settings menu designed to work constructively with the compositional system provided.

## Downloads
Start by downloading [Better Sunshine Engine](https://github.com/DotKuribo/BetterSunshineEngine/releases/latest) and extracting the `BetterSunshineEngine.zip` file anywhere you like. Additionally, you should ensure you have a legally extracted **NTSC-U** ISO of Super Mario Sunshine available. If you don't, consider downloading [pyisotools](https://github.com/JoshuaMKW/pyisotools/releases/latest), which supports extracting and rebuilding ISOs.
If you intend on modifying or developing your own levels, also download [BinEditor](https://github.com/AugsEU/Bin-editor-improvements/releases/latest).

## Game Setup
Copy the `~/Kuribo!` folder from the extracted zip to the `~/files/` directory of your extracted ISO. Also copy the `~/main.dol` and `~/boot.bin` from the extracted zip to the `~/sys/` directory of your extracted ISO.
- *`~/files/`*

![Files View](https://github.com/DotKuribo/BetterSunshineEngine/assets/60854312/19807c00-ad9b-4d5d-be37-ff35dfe27739)
  
- *`~/sys/`*

![Sys View](https://github.com/DotKuribo/BetterSunshineEngine/assets/60854312/55a779ea-acfa-45e2-9f7e-6f5fb01059e3)

This is everything you need to do to simply run Better Sunshine Engine with no further modifications. You are free to rebuild the ISO at this point and play the enhanced game!

## Editor Setup
Copy the `~/BinEditor/Parameters` and `~/BinEditor/Templates` directories from the extracted zip to the installation directory of your BinEditor. This is enough to get BinEditor to recognize the new Better Sunshine Engine objects next time you open the tool.
The following objects are included in Better Sunshine Engine:
- GenericRailObj - This object serves as a template for specialization. You provide it a filename and it loads relevant assets for the object accordingly (.bmd, .col, .btk, etc.). It also can be attached to a rail, and has predefined behavior for primitive animation patterns and sounds.
- ParticleBox - This object acts as a volumetric bounding box for a particle to spawn within. You can specify the particle to spawn, as well as its scale and spawn rate.
- SoundBox - This object acts as a volumetric bounding box for sounds to emit within. You can specify a sound to emit, as well as its volume, pitch, and spawn rate.
- SimpleFog - This object manipulates the darkness effect found in Delfino Plaza to emulate non-material fog within the scene. You can specify its behavior, start and end planes, near and far distances, and the color of the fog itself.

## Module Setup
To develop your own module in order to modify the code of the game, clone the existing [module template repository](https://github.com/DotKuribo/BetterSunshineModule) and follow the instructions provided to start developing your own module!

## Common Issues
- "Module "TheModuleName" is trying to register under the name "AliasModuleName", which is already taken!" - The stated module has attempted to register to Better Sunshine Engine under an alias name that already exists in the registration. To fix this, pick a name that is unique to your module.
- "Failed to load settings for module "TheModuleName"! (VERSION MISMATCH)" - The stated module is attempting to load its settings from the memory card, but the save file present is an older (or newer!) version than support by the module. You can delete the save file of the module if Better Sunshine Engine is failing to boot.
- "Application attempted to fetch context handler X but it wasn't found!" - One of the modules present has attempted to reroute Super Mario Sunshine to a game context that doesn't exist. To fix this, either remove the bad module, or repair the context registration if you are capable to by modifying the source code of the module.
- "Tried to open shine select screen for an area that has no pane ID!" - A stage or module has attempted to open a shine select screen for a world that doesn't have one. To fix this, change the stage warp to an area ID that does exist, or if it's module-side, repair the scene registration by assigning a valid pane ID in the scene information.
- "Missing stage name for area ID X (Y)" - A registered stage is pointing to an entry of `~/files/common.szs/2d/scenename.bmg` that doesn't exist. To fix this, either change the entry the registered stage information is pointing to, or add the entries to the BMG file.
- "A music stream attempted to play, but music streaming is disabled! Set byte 8 of boot.bin to 0x01 to enable music streaming." - This should never happen under a correct installation of Better Sunshine Engine. However, to fix it you should open `~/sys/boot.bin` in a hex editor and set the 8th byte to 0x01.

## License
Licensed under the [GPL-3.0](LICENSE) license.
