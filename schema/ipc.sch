record dataShort
    field value short
end

record dataInt
    field value int
end

record dataLong
    field value long
end

record dataChar
    field value char
end

record dataSymbol
    field value array char xz
end

record dataIntList
    field flags byte
    field valueL int
    field value array int xv valueL
end

record dataLongList
    field flags byte
    field valueL int
    field value array long xv valueL
end

record dataCharList
    field flags byte
    field valueL int
    field value array char xv valueL
end

record dataSymbolList
    field flags byte
    field valueL int
    field value array record dataSymbol xv valueL
end

record dataGenList
    field flags byte
    field valueL int
    field value array record data xv valueL
end

record dataDict
    field key record data
    field value record data
end

record dataTable
    field u0 byte
    field value record data
end

record data
    field type byte
    field value case type
        0 dataGenList
        6 dataIntList
        7 dataLongList
        10 dataCharList
        11 dataSymbolList
        98 dataTable
        99 dataDict
        245 dataSymbol
        246 dataChar
        249 dataLong
        250 dataInt
        251 dataShort
    end
end

record ipc
    field u0 array byte x 4
    field dataL int
    field data parsedArray xv dataL record data
end
