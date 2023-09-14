# PNG-Decoder
This decoder was built from the documentation on PNGs found at http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html.

This decoder does not accept PNGs that implement the interlace method.

## Zlib
This decoder utilizes zlib-1.3. The zlib file directory should be placed within the headers directory. Zlib is available for download at https://www.zlib.net/. For more information on the decompression process, see https://www.zlib.net/manual.html.

## Example Usage
You can interact with the raw pixel data of a PNG by running your PNG through the following steps:
```
// Open PNG with std::filesystem::path.
PNG_Decoder decoder(pngPath);
if (!decoder.IsOpen()) {
  throw std::runtime_error("Failed to open the provided PNG file.");
}

// Get compressed data from joined IDAT chunks.
char * compressedData = nullptr;
unsigned long compressedDataSize = decoder.AllocateCompressedData(compressedData);
if (compressedDataSize == 0) {
  throw std::runtime_error("Failed to allocate compressed data.");
}

// Get decompressed image data using zlib.
char * decompressedData = nullptr;
unsigned long decompressedDataSize = PNG_Decoder::AllocateDecompressedData(compressedData, compressedDataSize, decompressedData);
if (decompressedDataSize == 0) {
  throw std::runtime_error("Failed to allocate decompressed data.");
}

// Get unfiltered image data.
char * unfilteredData = nullptr;
unsigned long unfilteredDataSize = PNG_Decoder::AllocateUnfilteredData(decompressedData, unfilteredData,
  decoder.GetWidth(), decoder.GetHeight(), decoder.GetBitDepth(), decoder.GetColorType());
if (unfilteredDataSize == 0) {
  throw std::runtime_error("Failed to allocate unfiltered data.");
}

std::free(compressedData);
std::free(decompressedData);
std::free(unfilteredData);
```

## Color Type and Bit Depth
All PNGs have a color type and a bit depth.

There are 5 color types with corresponding bit depths:
```
Color    Allowed    Interpretation
Type    Bit Depths

0       1,2,4,8,16  Each pixel is a grayscale sample.

2       8,16        Each pixel is an R,G,B triple.

3       1,2,4,8     Each pixel is a palette index;
                    a PLTE chunk must appear.

4       8,16        Each pixel is a grayscale sample,
                    followed by an alpha sample.

6       8,16        Each pixel is an R,G,B triple,
                    followed by an alpha sample.
```

A color type of 6 and bit depth of 8 means that each pixel consists of 4 channels (rgba), each 8 bits (1 byte) in length for a total of 4 bytes per pixel.

You can see the color type and bit depth of a PNG in the IHDR chunk. Once you open a PNG with the decoder, you can use public methods to access all IHDR information:
```
Width:              4 bytes
Height:             4 bytes
Bit depth:          1 byte
Color type:         1 byte
Compression method: 1 byte
Filter method:      1 byte
Interlace method:   1 byte
```

## Color Type 3 (Palette)
If a PNG is color type 3, the raw data returned in the unfiltered data will not contain actual pixel values. Rather, each bit depth of data will be an index into a palette table. You can access the palette by looking through the PNG chunks available in the decoder and finding a chunk type equal to that of `ChunkType::PLTE`. Save the pointer to that chunk's data to have access to the palette table. The palette entries consist of three-byte pixel values (rgb). The first entry is index 0, the second entry is index 1, and so on.