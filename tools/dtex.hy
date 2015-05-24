#!/usr/bin/env hy
(import
  [docopt [docopt]]
  [PIL [Image]]
  struct
  [sys [stderr]])

(def --doc-- "
dtex, a texture converter for Nintendo DS homebrew

Usage:
    dtex.hy <input_filename> to (2bpp | 4bpp | 8bpp | 16bpp | a3i5 | a5i3 |
      4x4c ) at <output_filename>
    dtex.hy <input_filename> to (2bpp | 4bpp | 8bpp | 16bpp | a3i5 | a5i3 |
      4x4c ) palette at <output_filename>
    dtex.hy --help
    dtex.hy --version

Options:
    --help     Print this message and exit
    --version  Show version number and exit
    --format <format>  Output texture format; one of 2bpp, 4bpp, 8bpp, 16bpp,
                       a3i5, a5i3, compressed
")

(def --version-- "0.1.0")

(defn pack-byte [b]
  (.pack struct "<B" b))

(defn pack-short [s]
  (.pack struct "<H" s))

(defn bunch [n slicable]
  (apply zip (list (map (fn [i] (slice slicable i None n)) (range 0 n))) {}))

(defn gather-subcomponents [palette-subcomponents]
  (->> palette-subcomponents (bunch 3) list))

(defn correct-subcomponent-bit-depth [component]
  (>> component 3))

(defn stack-shift-bits [bits-per-element elements]
  (reduce
    (fn [a b] (| a b))
    (list-comp (<< e (* bits-per-element i)) [(, i e) (enumerate elements)])))

(defn unimplemented-conversion [values]
  (throw NotImplementedError))

(defn bad-conversion [values]
  (throw ValueError))

(defn n-bit-image-converter [bit-depth]
  (fn [indexes] (convert-to-n-bit-image bit-depth indexes)))
(def converters
  {"2bpp" (n-bit-image-converter 2)
  "4bpp" (n-bit-image-converter 4)
  "8bpp" (n-bit-image-converter 8)
  "16bpp" unimplemented-conversion
  "a3i5" (n-bit-image-converter 8)
  "a5i3" (n-bit-image-converter 8)
  "4x4c" unimplemented-conversion})

(defn n-bit-palette-extractor [bit-depth]
  (fn [palette] (extract-n-bit-palette bit-depth palette)))
(def palette-converters
  {"2bpp" (n-bit-palette-extractor 2)
  "4bpp" (n-bit-palette-extractor 4)
  "8bpp" (n-bit-palette-extractor 8)
  "16bpp" bad-conversion
  "a3i5" (n-bit-palette-extractor 8)
  "a5i3" (n-bit-palette-extractor 8)
  "4x4c" unimplemented-conversion})

(defn convert-to-n-bit-image [bit-depth indexes]
  (let
    [[index-mask (dec (<< 1 bit-depth))]
      [pixels-per-byte (// 8 bit-depth)]
      [packed-bytes
        (->>
          indexes
          (map (fn [i] (& i index-mask)))
          list
          (bunch pixels-per-byte)
          list
          (map (fn [byte] (stack-shift-bits bit-depth byte)))
          (map pack-byte))]]
    (.join (bytes) packed-bytes)))

(defn extract-n-bit-palette [bit-depth color-triplets]
  (let 
    [[color-count (** 2 bit-depth)]
      [colors (list (take color-count color-triplets))]
      [packed-colors (map (fn [c] (stack-shift-bits 5 c)) colors)]
      [packed-shorts (map pack-short packed-colors)]]
    (.join (bytes) packed-shorts)))

(defmain [&rest args]
  (try
    (let
      [[args (apply docopt [--doc--] {"version" --version--})]
        [input-filename (get args "<input_filename>")]
        [output-filename (get args "<output_filename>")]
        [format-names (.split "2bpp 4bpp 8bpp 16bpp a3i5 a5i3 4x4c")]
        [format (get (list (filter (fn [f] (get args f)) format-names)) 0)]
        [convert-palette? (get args "palette")]
        [input-image (.open Image input-filename)]
        [image-converter (get converters format)]
        [palette-converter (get palette-converters format)]]
      (with [[output-file (open output-filename "wb")]]
        (.write output-file
          (if convert-palette?
            (->>
              input-image
              .getpalette
              (map correct-subcomponent-bit-depth)
              list
              gather-subcomponents
              palette-converter)
            (-> input-image .getdata image-converter)))))
    (catch [e NotImplementedError]
      (.write stderr "Requested conversion not yet implemented.\n")
      -1)
    (catch [e ValueError]
      (.write stderr "Requested conversion doesn't make sense.\n")
      -1)))