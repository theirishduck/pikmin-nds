# Pikmin NDS :seedling:

Pikmin NDS (also called Pikmin DS) is a tech demo that runs on the Nintendo DS that pushes the hardware to its limits. It is inspired by the Pikmin series by Nintendo, which premiered on the Nintendo GameCube.

There is an [imgur album](http://imgur.com/a/YMi6K) here detailing progress on the demo.

## Goals

### Multipass Renderer

The Nintendo DS's hardware is capable of rendering 2048 polygons in a single frame. Due to the nature of Pikmin games, there are often over 100 entities on screen, with 100 of them being the pikmin themselves. If each pikmin is roughly 30 polygons, that would necessitate 3000 polygons, meaning that a full squad could not be rendered on screen at once.

The solution is to take advantage of the DS's display capture unit, which can copy the output of the 3D engine (even if it's not being displayed on screen), and subsequently render it as a texture in the following pass. By carefully partitioning the scene, the compositing of the resulting layers is straightforward. After rendering all passes, the final rendered frame can be captured and displayed as a background in the 2D hardware engine using the same display capture system. The fully rendered background will then be drawn over the top of the passes for the next frame.

The net effect of this technique is to artificially increase the polygon count (by 2048 per pass) at the expense of framerate. Considering the small size of a DS handheld, roughly 15-20FPS should be the theoretical limit of this technique. Any less breaks the illusion of motion. We're shooting for 3 passes maximum, at 20FPS, for a total polygon limit of 6144.

There are three main strategies that have been considered to composite individual passes together: front to back, top to bottom, and side to side. Currently the engine supports back to front partitioning, sorting objects based on their Z-coordinate and size, and correctly handles large objects that need to be redrawn across partition boundaries.

### Physics engine

The main challenge in writing the physics engine is that there are so many entities to process. The NDS's processors aren't very fast (they clock in at 66MHz and 33MHz), and there are hardware issues that can slow them down even further - namely, a poor hardware cache and non-sequential (i.e. most) memory access.

For level collision, we prerender the scene geometry into a height map. While this limits us in terms of overhangs and bridges (which will need special consideration) it is a reasonably fast technique, and handles most typical level geometry quite well.

Entity collision is processed entirely as axis-aligned cylinders. We use a modified k-nearest-neighbor graph to group entities together by their location. For the swarm, we additionally employ a small hack; members mostly ignore collision with each other, unless they share a cell on the height map. They are never added to the kNN graph, leaving them free to consider more important objects in the level.

### AI for all non-player entities

Most AI will be handled as interactions between the physics engine's sensors and a state machine to dictate responses. Advanced AI isn't necessary, but again, the entity count forces us to run a large number of state machines.

Fortunately, AI for every entity does not need to be run every frame - instead, we can get away with updating many of the state machines on a rolling basis, so long as they all get updated in a reasonable time frame.

## Building and running

Pikmin NDS is built using devkitARM portion of [devkitPro](http://devkitpro.org/), which runs on Windows, Linux, and Mac. It has been tested on r41 and r42, but should build on r43 as well. After installing devkitARM, `cd` into the project directory and run `make`. This will produce an NDS ROM titled `pikmin-nds.nds`.

An alternative method for compiling, more suitable for development, uses [docker](http://docs.docker.com/mac/started). **Warning**, this will download quite a few packages to set up the developmental envirnment (only the first time). To compile this way, execute `build.sh` as the root user. For example, under debian based systems you would run the following:
```
sudo apt-get install docker.io
sudo sh build.sh
```

This ROM can be run in [no$gba](http://problemkaputt.de/gba.htm) or in [DeSmuME](http://desmume.org/), or run on real hardware using flash carts. If you don't intend to be developing the software and want to run it in an emulator, the gaming version of no$gba is recommended; the debug version of no$gba is prone to rapid slowdowns, and DeSmuME is less accurate in its emulation.

## Usage notes

### Run speed

Emulators and real hardware differ greatly in their hardware timings - this can cause significant issues, particularly if a pass takes longer than a frame to render. In particular, no$'s graphics hardware related timings are too fast.

### Emulation bugs

Console output to the secondary screen in DeSmuME does not work. This broke when changing the backing bank for the console from VRAM bank C to bank H, and we're not sure why. It still works in no$ and on real hardware.
