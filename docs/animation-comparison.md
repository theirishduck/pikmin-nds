# Hardware Run of 100 Pikmin w/ various Animation Systems

Recently, we rewrote the animation system to support vertex+normal animations,
in addition to the standard pre-computed bone transform style. We expected this
to be a performance improvement, and it turns out that it was instead very
enlightening.

We performed 5 tests in both the DeSmuME emulator, and on a real Nintendo DS
unit. For each test, I spawned Olimar in the center, ran to the right in a
straight line to the Red Onion, and immediately withdrew 100 Pikmin. As soon as
those pikmin finished emerging and settling down into the squad, I read off the
numbers from the Timing Page measuring the total Cycle Cost of each of the 4
passes needed to render the scene. I display these value verbatim, as well as
their sum at the end of each row.

The 5 animation modes used were as follows:
- Old-style Bone Animation (An old build of pikmin-nds, from before the new systems were implemented)
- New-style Bone Animation
- Vertex + Normal Animation
- Vertex Only Animation
- No Animation (Pikmin render in T-Pose, no bone-matrices embedded in their DSGX chunk)

The draw times are as follows:

| DeSmuME  | Pass 1 | Pass 2 | Pass 3 | Pass 4 | TOTAL |
| -------- | -----: | -----: | -----: | -----: | ----: |
| Old-Bone | 176k   | 198k   | 198k   | 25k    | 597k  |
| New-Bone | 197k   | 225k   | 224k   | 25k    | 671k  |
| Vtx+Norm | 195k   | 220k   | 220k   | 27k    | 662k  |
| Vtx Only | 144k   | 159k   | 161k   | 23k    | 487k  |
| No-Anims | 85k    | 91k    | 91k    | 17k    | 284k  |

| Hardware | Pass 1 | Pass 2 | Pass 3 | Pass 4 | TOTAL |
| -------- | -----: | -----: | -----: | -----: | ----: |
| Old-Bone | 168k   | 190k   | 190k   | 24k    | 572k  |
| New-Bone | 151k   | 171k   | 172k   | 21k    | 515k  |
| Vtx+Norm | 184k   | 207k   | 210k   | 24k    | 625k  |
| Vtx Only | 119k   | 130k   | 130k   | 20k    | 399k  |
| No-Anims | 73k    | 80k    | 79k    | 14k    | 246k  |

These results are really surprising for a number of reasons. Despite what I
thought was increased complexity, the new animation system handles bones better
than the old one did by a small margin, but only on real hardware.

The bummer is that the Vertex+Normals system that perfectly replicates the bone
animation system both consumes more memory, and is simply too CPU heavy during
application to be a viable replacement for the Bone system, even for the Pikmin.

A viable alternative that reduces overall animation quality would be to use
vertices *only* with the existing systems, which is faster than the bone system
by a small margin. This eliminates updating the Normal data during animation
though, which means the Pikmin will have incorrect / more static lighting. I
don't like this idea much.

The very *fastest* option by a landslide would be to toss all the dynamic
animation systems out for the Pikmin and simply pre-render a new DSGX chunk for
them for each frame. This would achieve speeds very close to No-Anims while
being very accurate animation. This is just ridiculously fast, but carries a
huge catch: it is very, VERY memory intensive. If we go that route, we'll
need to seriously optimize the mesh for size where possible.
