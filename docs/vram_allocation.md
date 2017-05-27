# VRAM Allocation Strategies 

For ease of reference, passes start at number 1 and the debugger displays them as such. In code, the first pass is 0.

## First attempt

We never fully documented this strategy, but this involved cycling the LCD Capture, Texture, and Background display between three banks. It was difficult to keep track of and error prone, and we eventually realized that it was cleaner to restrict the rear-plane capture to two banks and flip between them.

## Second attempt

Banks A and B: Flip Flop between LCDC and Texture Memory, for use as the rear plane / previous pass data.
Bank C: Holds normal game textures
Bank D: Holds the final image, flips between LCDC and BG, to dislpay final capture during next frame's rendering process.

During the final pass, we instruct the NDS's 3D engine to output directly to the display, while simultaneously capturing that output for BG display in future passes.

### Odd Pass

|                | Bank A | Bank B | Bank C | Bank D |
| -------------- | ------ | ------ | ------ | ------ |
| Texture Slot 0 |        | X      |        |        |
| Texture Slot 1 |        |        |        |        |
| Texture Slot 2 |        |        | X      |        |
| LCDC           | X      |        |        |        |
| Background     |        |        |        | X      |

### Even Pass

|                | Bank A | Bank B | Bank C | Bank D |
| -------------- | ------ | ------ | ------ | ------ |
| Texture Slot 0 | X      |        |        |        |
| Texture Slot 1 |        |        |        |        |
| Texture Slot 2 |        |        | X      |        |
| LCDC           |        | X      |        |        |
| Background     |        |        |        | X      |


### Final Pass Odd

|                | Bank A | Bank B | Bank C | Bank D |
| -------------- | ------ | ------ | ------ | ------ |
| Texture Slot 0 |        | X      |        |        |
| Texture Slot 1 |        |        |        |        |
| Texture Slot 2 |        |        | X      |        |
| LCDC           |        |        |        | X      |
| Background     |        |        |        |        |

### Final Pass Even

|                | Bank A | Bank B | Bank C | Bank D |
| -------------- | ------ | ------ | ------ | ------ |
| Texture Slot 0 | X      |        |        |        |
| Texture Slot 1 |        |        |        |        |
| Texture Slot 2 |        |        | X      |        |
| LCDC           |        |        |        | X      |
| Background     |        |        |        |        |

