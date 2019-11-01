# dwm

This is my own fork of the awesome [Dynamic Window Manager](https://dwm.suckless.org/) (version 6.1) from [suckless](https://suckless.org).

I have implemented the following functionalities:
* IPC hooks
* `dwm-msg` for sending IPC messages
* Hidable windows in tiling mode  
* Custom windows hooks (configurable in the `rules`)
* Spawn floating windows in the center
* Toggle window full screen mode

I would love to provide those changes as patches, but the coding style requirements are crazy and I do not have so much time :smiley:

I have applied the following patches:
* dwm-6.1-systray
* dwm-centeredmaster-6.1
* dwm-cyclelayouts-20180524-6.2
* dwm-gaplessgrid-6.1
* dwm-6.1-pertag-tab-v2b
* dwm-noborder-6.1.diff

my `config.h` requires: `nerd-fonts-complete`
