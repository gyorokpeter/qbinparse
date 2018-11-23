record pngImageHeader
    field width be int
    field height be int
    field bitDepth byte
    field colorType byte
    field compressionMethod byte
    field filterMethod byte
    field interlaceMethod byte
end

record pngText
    field ky array char tpb 0
    field separator char
    field val array char repeat
end

record pngGamma
    field gamma be int
end

record pngChromacities
    field whitePointX be int
    field whitePointY be int
    field redX be int
    field redY be int
    field greenX be int
    field greenY be int
    field blueX be int
    field blueY be int
end

record pngPixelDimensions
    field pixelPerUnitX be int
    field pixelPerUnitY be int
    field unit byte
end

record pngImageEnd
end

record pngUnknownTag
    field data array byte repeat
end

record tag
    field length be int
    field chunkType array char x 4
    field chunkData parsedArray xv length case chunkType
        "IHDR" pngImageHeader
        "tEXt" pngText
        "gAMA" pngGamma
        "cHRM" pngChromacities
        "pHYs" pngPixelDimensions
        "IEND" pngImageEnd
        default pngUnknownTag
    end
    field crc array byte x 4
end

record png
    field magic array char x 8
    field tags array record tag repeat
end