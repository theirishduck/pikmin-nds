#ifndef PROJECT_SETTINGS_H_
#define PROJECT_SETTINGS_H_

//Used to increase the overlap between multipass slices.
//Might be useful to reduce visible seams. Higher values make the 3DS
//engine draw more extra polygons at the rear of a pass, and will likely
//cause artifacts with alpha-blended polygons.
#ifndef CLIPPING_FUDGE_FACTOR
#define CLIPPING_FUDGE_FACTOR 0
#endif

//As polygon counts are estimated (can't use the hardware to directly count)
//we need to be able to fudge on the max per pass to achieve a good balance
//between performance (average closer to 2048 polygons every pass) and
//sanity (not accidentally omitting polygons because we guess badly)
#ifndef MAX_POLYGONS_PER_PASS
#define MAX_POLYGONS_PER_PASS 1800
#endif



#endif