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

(defn unimplemented-conversion [values]
  (throw NotImplementedError))

(defn bad-conversion [values]
  (throw ValueError))

(defn convert-to-2bpp [indexes]
  (let
    [[truncated-indexes (list (map (fn [i] (& i 3)) indexes))]
      [indexes-in-bytes (->> truncated-indexes (bunch 4) list)]
      [individual-bytes (map
        (fn [byte] (let [[(, a b c d) byte]] (| a (<< b 2) (<< c 4) (<< d 6))))
        indexes-in-bytes)]
      [packed-bytes (map pack-byte individual-bytes)]]
    (.join (bytes) packed-bytes)))

(defn convert-to-4bpp [indexes]
  (let
    [[truncated-indexes (list (map (fn [i] (& i 15)) indexes))]
      [indexes-in-bytes (->> truncated-indexes (bunch 2) list)]
      [individual-bytes (map
        (fn [byte] (let [[(, a b) byte]] (| a (<< b 4))))
        indexes-in-bytes)]
      [packed-bytes (map pack-byte individual-bytes)]]
    (.join (bytes) packed-bytes)))

(defn convert-to-8bpp [indexes]
  (let [[packed-bytes (map pack-byte indexes)]]
    (.join (bytes) packed-bytes)))

(def converters
  {"2bpp" convert-to-2bpp
  "4bpp" convert-to-4bpp
  "8bpp" convert-to-8bpp
  "16bpp" unimplemented-conversion
  "a3i5" convert-to-8bpp
  "a5i3" convert-to-8bpp
  "4x4c" unimplemented-conversion})

(defn extract-n-bit-palette [bit-depth color-triplets]
  (let 
    [[color-count (** 2 bit-depth)]
      [colors (list (take color-count color-triplets))]
      [packed-colors (map (fn [c] (let [[(, r g b) c]] (| r (<< g 5) (<< b 10)))) colors)]
      [packed-shorts (map pack-short packed-colors)]]
    (.join (bytes) packed-shorts)))

(defn extract-2bpp-palette [palette]
  (extract-n-bit-palette 2 palette))

(defn extract-4bpp-palette [palette]
  (extract-n-bit-palette 4 palette))

(defn extract-8bpp-palette [palette]
  (extract-n-bit-palette 8 palette))

(defn gather-subcomponents [palette-subcomponents]
  (->> palette-subcomponents (bunch 3) list))

(defn correct-subcomponent-bit-depth [component]
  (>> component 3))

(def palette-converters
  {"2bpp" extract-2bpp-palette
  "4bpp" extract-4bpp-palette
  "8bpp" extract-8bpp-palette
  "16bpp" bad-conversion
  "a3i5" extract-8bpp-palette
  "a5i3" extract-8bpp-palette
  "4x4c" unimplemented-conversion})

(defn pack-byte [b]
  (.pack struct "<B" b))

(defn pack-short [s]
  (.pack struct "<H" s))

(defn bunch [n slicable]
  (apply zip (list (map (fn [i] (slice slicable i None n)) (range 0 n))) {}))

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