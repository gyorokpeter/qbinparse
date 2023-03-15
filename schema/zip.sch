record file
    field ver short
    field flags short
    field compression short
    field mtime short
    field mdate short
    field crc int
    field cmprSize int
    field origSize int
    field nameL short
    field extraL short
    field name array char xv nameL
    field extra array byte xv extraL
    field data array byte xv cmprSize
end

record centralDir
    field ver short
    field needVer short
    field flags short
    field compression short
    field mtime short
    field mdate short
    field crc int
    field cmprSize int
    field origSize int
    field nameL short
    field extraL short
    field commentL short
    field diskNo short
    field internalAttr short
    field externalAttr int
    field localHdrOff int
    field name array char xv nameL
    field extra array byte xv extraL
    field comment array char xv commentL
end

record endOfArchive
    field diskNo short
    field diskWithCD short
    field diskEntries short
    field totalEntris short
    field cdSize int
    field cdOffset int
    field commentL short
    field comment array char xv commentL
end

record zipc
    field magic int
    field cont case magic
        67324752 file
        33639248 centralDir
        101010256 endOfArchive
    end
end

record zip
    field cont array record zipc repeat
end
