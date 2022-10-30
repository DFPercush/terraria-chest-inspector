# Terraria Chest Inspector

Command-line terraria chest viewer. Born of the inability of TEdit to run on linux. To be clear, this is not a replacement for TEdit. You can not modify tile data or world flags like which bosses have been cleared. This is simply a read-only tool to dump a bunch of info about the chests in the world and the items they contain. However, if you want to modify the items, this will at least give you an address to open in a hex editor.

## How to use

Put the world file as an argument, or "Open with" on the world file using this program. The output can be combined with grep, sort, and other CLI tools to filter or condense the information as desired. Recommended to use a unix-style environment like MinGW



## Example output:
```
$ terraria-chest-inspector My_World.wld
0x00ba5526 tile( 4197,   465)  gps(   -6,   260) Chest # 386: --------------Mining
0x00ba5526 tile( 4197,   465)  gps(   -6,   260) Chest # 386: --------------Mining 0x00ba5535 {   4,   180,  0}    4 x Topaz
0x00ba5526 tile( 4197,   465)  gps(   -6,   260) Chest # 386: --------------Mining 0x00ba553c { 204,   702,  0}  204 x Platinum Ore
0x00ba5629 tile( 4201,   465)  gps(    2,   260) Chest # 388: --------------Plants
0x00ba5629 tile( 4201,   465)  gps(    2,   260) Chest # 388: --------------Plants 0x00ba5638 { 100,    27,  0}  100 x Acorn
0x00ba5629 tile( 4201,   465)  gps(    2,   260) Chest # 388: --------------Plants 0x00ba563f { 836,  1725,  0}  836 x Pumpkin
0x00ba5629 tile( 4201,   465)  gps(    2,   260) Chest # 388: --------------Plants 0x00ba5646 {   1,  4282,  0}    1 x Apricot
0x00ba5629 tile( 4201,   465)  gps(    2,   260) Chest # 388: --------------Plants 0x00ba564d {  20,     5,  0}   20 x Mushroom
...
```

Where in the world did I put that yoyo set?

```
$ terraria-chest-inspector The_Lasting_Wood_of_Orchids.wld | grep -i "counterweight"
0x00ba4e37 tile( 4201,   459)  gps(    2,   272) Chest # 373: -------------Weapons 0x00ba4e5e {   1,  3314,  0}    1 x Yellow Counterweight
```

Without spoiling the exact location, does your world even have a certain item? Let's say, a Sandstorm in a Bottle:

```
$ terraria-chest-inspector The_Lasting_Wood_of_Orchids.wld | grep -i "sandstorm" | wc -l
0
```

Nope :(  Oh well. Does it have a red balloon?

```
$ terraria-chest-inspector The_Lasting_Wood_of_Orchids.wld | grep -i "balloon" | wc -l
1
```

Well that's good news, at least.


I'm at my wit's end searching for Hermes Boots, and I don't care any more. Where are they?

```
$ terraria-chest-inspector The_Lasting_Wood_of_Orchids.wld | grep -i "hermes"
0x00b9b549 tile( 4069,  1128)  gps( -262, -1066) Chest #  42: -------------------- 0x00b9b552 {   1,    54, 76}    1 x Quick Hermes Boots
...
```


## Explanation

First, a line will be printed for each chest, whether it contains any items or not. These lines lack item fields.
```
CHEST ADDR      TILE POSITION      GPS POSITION        INDEX           CUSTOM NAME
0x00ba5526 tile( 4197,   465)  gps(   -6,   260) Chest # 386: --------------Mining
```

Then, each item in the chest will be listed. The full chest information is also included for the purpose of making it easier to script and combine with other tools.
The name of the item is on the right.

```
CHEST ADDR      TILE POSITION      GPS POSITION        INDEX           CUSTOM NAME  ITEM ADDR   DATA STRUCTURE   QTY   DISPLAY NAME
0x00ba5526 tile( 4197,   465)  gps(   -6,   260) Chest # 386: --------------Mining 0x00ba5535 {   4,   180,  0}    4 x Topaz
                                                                               DATA SIZE --->   2 B,   4 B, 1 B
```

The `ADDR` fields are given in bytes from the beginning of the save file, not a memory location while the game is loaded.
You can use a hex editor to modify these fields by going to that offset in the file, if you wish. Quantity is a 2-byte integer, item_id is a 4-byte integer, and prefix is a single byte. These ids can be found in the source code or on the Terraria wiki.


### Replace, do not add!

Never try to add more items to a chest, only replace existing items. This is because an empty slot only contains a quantity, not the other fields. So chests are a variable-length structure. You will throw the whole save file off if you try to add things to an empty slot. If you want to give yourself stuff, throw down a chest and put a bunch of junk in it.

