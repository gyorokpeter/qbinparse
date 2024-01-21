record unknown
    field data array byte repeat
end

record udp
    field srcPort be ushort
    field dstPort be ushort
    field length be short
    field checksum short
    field data array byte repeat
end

record ipv4
    field version byte
    field services byte
    field totalLength be short
    field id short
    field flags short
    field ttl byte
    field protocol byte
    field headerChecksum short
    field srcAddr array byte x 4
    field dstAddr array byte x 4
    field payload case protocol
        17 udp
        default unknown
    end
end

record frame
    field dstAddr array byte x 6
    field srcAddr array byte x 6
    field type short
    field ip case type
        8 ipv4
        default unknown
    end
end

record packet
    field timeStampSec int
    field timeStampSubSec int
    field capLength int
    field origLength int
    field data parsedArray xv capLength record frame
end

record pcap
    field magic int
    field majorVersion short
    field minorVersion short
    field reserved1 int
    field reserved2 int
    field snapLen int
    field fcs short
    field linkType short

    field cap array record packet repeat
end
