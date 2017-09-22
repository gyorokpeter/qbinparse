\c 100000 100000
{
    path:"/"sv -1_"/"vs ssr[;"\\";"/"]first -3#value .z.s;
    system"l ",path,"/qbinparse.q";
    }[];

simpleRecord:.binp.compileSchema"
    record point
        field xpos real
        field ypos real
    end
    ";
.binp.parse[simpleRecord;0x0000f04200000243;`point] //`xpos`ypos!(120e;130e)

simpleArray:.binp.compileSchema"
    record a
        field ints array int x 4
    end
    ";
.binp.parse[simpleArray;0x01000000020000000300000004000000;`a]  //enlist[`ints]!enlist 1 2 3 4i

varLengthArray:.binp.compileSchema"
    record a
        field intsL short
        field ints array int xv intsL
    end
    ";
.binp.parse[varLengthArray;0x0300010000000200000003000000;`a]  //`intsL`ints!(3h;1 2 3i)

varLengthRecArray:.binp.compileSchema"
    record point
        field xpos int
        field ypos int
    end

    record a
        field pointsL short
        field points array record point xv pointsL
    end
    ";
.binp.parse[varLengthRecArray;0x020001000000020000000300000004000000;`a]  //`pointsL`points!(2h;(`xpos`ypos!(1i;2i);`xpos`ypos!(3i;4i)))

byteGuardAtomicArray:.binp.compileSchema"
    record a
        field items array short tpb 3
        field guard byte
        field other byte
    end
    ";
.binp.parse[byteGuardAtomicArray;0x0102050607080a0b0304;`a] //`items`guard`other!(513 1541 2055 2826h;0x03;0x04)

shortGuardAtomicArray:.binp.compileSchema"
    record a
        field items array short tps 3
        field guard short
        field other byte
    end
    ";
.binp.parse[shortGuardAtomicArray;0x010203040506070803000a;`a] //`items`guard`other!(513 1027 1541 2055h;3h;0x0a)

byteGuardRecordArray:.binp.compileSchema"
    record point
        field xpos int
        field ypos int
    end

    record a
        field items array record point tpb 255
        field guard byte
        field other byte
    end
    ";
.binp.parse[byteGuardRecordArray;0x01000000020000000300000004000000ff04;`a] //`items`guard`other!(513 1541 2055 2826h;0x03;0x04)

recordWithCase:.binp.compileSchema"
    record caseA
        field contInt int
    end

    record caseB
        field contShort short
    end

    record tagged
        field tag byte
        field data case tag
            0 caseA
            1 caseB
        end
    end

    record main
        field tags array record tagged tpb 2
        field guard byte
    end
    ";
.binp.parse[recordWithCase;0x0001000000010200000300000001040002;`main]
//`tags`guard!(`tag`data!(0x00;enlist[`contInt]!enlist 1i);
//             `tag`data!(0x01;enlist[`contShort]!enlist 2h);
//             `tag`data!(0x00;enlist[`contInt]!enlist 3i);
//             `tag`data!(0x01;enlist[`contShort]!enlist 4h))
//             ;0x02)

recordWithCase2:.binp.compileSchema"
    record caseA
        field contInt int
    end

    record caseB
        field contShort short
    end

    record tagged
        field tag long
        field data case tag
            0 caseA
            1 caseB
        end
    end

    record main
        field tags array record tagged tpb 2
        field guard byte
    end
    ";
.binp.parse[recordWithCase2;0x000000000000000001000000010000000000000002000000000000000000030000000100000000000000040002;`main]
//`tags`guard!(`tag`data!(0j;enlist[`contInt]!enlist 1i);
//             `tag`data!(1j;enlist[`contShort]!enlist 2h);
//             `tag`data!(0j;enlist[`contInt]!enlist 3i);
//             `tag`data!(1j;enlist[`contShort]!enlist 4h))
//             ;0x02)

recordWithCaseDefault:.binp.compileSchema"
    record caseA
        field contInt int
    end

    record caseB
        field contShort short
    end

    record tagged
        field tag byte
        field data case tag
            0 caseA
            default caseB
        end
    end

    record main
        field tags array record tagged tpb 2
        field guard byte
    end
    ";
.binp.parse[recordWithCaseDefault;0x000100000038020000030000004f040002;`main]
//`tags`guard!(`tag`data!(0x00;enlist[`contInt]!enlist 1i);
//             `tag`data!(0x38;enlist[`contShort]!enlist 2h);
//             `tag`data!(0x00;enlist[`contInt]!enlist 3i);
//             `tag`data!(0x4f;enlist[`contShort]!enlist 4h))
//             ;0x02)
