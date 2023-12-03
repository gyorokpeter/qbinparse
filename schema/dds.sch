record ddsPixelFormat
    field headerSize int
    field flags int
    field fourCC int
    field rgbBitCount int
    field rBitMask int
    field gBitMask int
    field bBitMask int
    field aBitMask int
end

record ddsHeader
    field headerSize int
    field flags int
    field height int
    field width int
    field pitchOrLinearSize int
    field depth int
    field mipMapCount int
    field reserved array int x 11
    field pixelFormat record ddsPixelFormat
    field caps int
    field caps2 int
    field caps3 int
    field caps4 int
    field reserved2 int
end

record dds
    field magic array char x 4
    field header record ddsHeader
    field content array byte repeat
end
