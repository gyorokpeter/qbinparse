\c 100000 100000

{
    path:"/"sv -1_"/"vs ssr[;"\\";"/"]first -3#value .z.s;
    system"l ",path,"/../qbinparse.q";
    }[];

simpleRecord:.binp.compileSchema"
    record point
        field xpos real
        field ypos real
    end
    ";
if[not .binp.parse[simpleRecord;0x0000f04200000243;`point] ~`xpos`ypos!(120e;130e); '"failed"];

mixedRecord:.binp.compileSchema"
    record point
        field xpos real
        field ypos short
    end
    ";
if[not .binp.parse[mixedRecord;0x0000f042f0d8;`point] ~`xpos`ypos!(120e;-10000h); '"failed"];

simpleArray:.binp.compileSchema"
    record a
        field ints array int x 4
    end
    ";
if[not .binp.parse[simpleArray;0x01000000020000000300000004000000;`a]~enlist[`ints]!enlist 1 2 3 4i; '"failed"];

twoSimpleArrays:.binp.compileSchema"
    record a
        field ints array int x 4
        field shorts array short x 5
    end
    ";
if[not .binp.parse[twoSimpleArrays;0x0100000002000000030000000400000005000600070008000900;`a]~`ints`shorts!(1 2 3 4i;5 6 7 8 9h); '"failed"];

varLengthArray:.binp.compileSchema"
    record a
        field intsL short
        field ints array int xv intsL
    end
    ";
if[not .binp.parse[varLengthArray;0x0300010000000200000003000000;`a]~`intsL`ints!(3h;1 2 3i); '"failed"];

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
if[not .binp.parse[varLengthRecArray;0x020001000000020000000300000004000000;`a]~`pointsL`points!(2h;(`xpos`ypos!(1i;2i);`xpos`ypos!(3i;4i))); '"failed"];

byteGuardAtomicArray:.binp.compileSchema"
    record a
        field items array short tpb 3
        field guard byte
        field other byte
    end
    ";
if[not .binp.parse[byteGuardAtomicArray;0x0102050607080a0b0304;`a]~`items`guard`other!(513 1541 2055 2826h;0x03;0x04); '"failed"];

shortGuardAtomicArray:.binp.compileSchema"
    record a
        field items array short tps 3
        field guard short
        field other byte
    end
    ";
if[not .binp.parse[shortGuardAtomicArray;0x010203040506070803000a;`a]~`items`guard`other!(513 1027 1541 2055h;3h;0x0a); '"failed"];

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
if[not .binp.parse[byteGuardRecordArray;0x01000000020000000300000004000000ff04;`a]~`items`guard`other!((`xpos`ypos!1 2i;`xpos`ypos!3 4i);0xff;0x04); '"failed"];

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
if[not .binp.parse[recordWithCase;0x0001000000010200000300000001040002;`main]~
    `tags`guard!((`tag`data!(0x00;enlist[`contInt]!enlist 1i);
                 `tag`data!(0x01;enlist[`contShort]!enlist 2h);
                 `tag`data!(0x00;enlist[`contInt]!enlist 3i);
                 `tag`data!(0x01;enlist[`contShort]!enlist 4h));
                 0x02); '"failed"];

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
if[not .binp.parse[recordWithCase2;0x000000000000000001000000010000000000000002000000000000000000030000000100000000000000040002;`main]~
    `tags`guard!((`tag`data!(0j;enlist[`contInt]!enlist 1i);
                 `tag`data!(1j;enlist[`contShort]!enlist 2h);
                 `tag`data!(0j;enlist[`contInt]!enlist 3i);
                 `tag`data!(1j;enlist[`contShort]!enlist 4h))
                 ;0x02); '"failed"];

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
if[not .binp.parse[recordWithCaseDefault;0x000100000038020000030000004f040002;`main]~
    `tags`guard!((`tag`data!(0x00;enlist[`contInt]!enlist 1i);
    `tag`data!(0x38;enlist[`contShort]!enlist 2h);
    `tag`data!(0x00;enlist[`contInt]!enlist 3i);
    `tag`data!(0x4f;enlist[`contShort]!enlist 4h))
    ;0x02); '"failed"];

recordWithCase3:.binp.compileSchema"
    record caseNo
    end

    record caseYes
        field cont array byte x 4
    end

    record main
        field tag int
        field data
            case tag
                0       caseNo
                default caseYes
            end
    end
    ";
if[not .binp.parse[recordWithCase3;0x0100000002020202;`main]~`tag`data!(1i;enlist[`cont]!enlist 0x02020202);'"failed"];
if[not .binp.parse[recordWithCase3;0x00000000;`main]~`tag`data!(0i;(`$())!());'"failed"];

bigEndian:.binp.compileSchema"
    record main
        field f1 be short
        field f2 be int
    end
    ";

if[not .binp.parse[bigEndian;0x000100000001;`main]~`f1`f2!(1h;1i); '"failed"];

repeatingAtom:.binp.compileSchema"
    record main
        field f1 array int repeat
    end
    ";
if[not .binp.parse[repeatingAtom;0x010000000200000003000000;`main]~enlist[`f1]!enlist 1 2 3i; '"failed"];

repeatingRecord:.binp.compileSchema"
    record item
        field a int
        field b int
    end
    record main
        field f1 array record item repeat
    end
    ";
if[not .binp.parse[repeatingRecord;0x01000000020000000300000004000000;`main]~enlist[`f1]!enlist (`a`b!1 2i;`a`b!3 4i); '"failed"];

emptyRecordList:.binp.compileSchema"
    record item
        field a int
        field b int
    end
    record main
        field f1 int
        field f2 array record item x 0
        field f3 int
    end
    ";
if[not .binp.parse[emptyRecordList;0x0100000002000000;`main]~`f1`f2`f3!(1i;();2i); '"failed"];

sizeTaggedRecord:.binp.compileSchema"
    record item
        field a byte
        field b byte
    end
    record main
        field f1C byte
        field f1 parsedArray xv f1C array record item repeat
        field f2 byte
    end
    ";
if[not .binp.parse[sizeTaggedRecord;0x0401020304ff;`main]~`f1C`f1`f2!(0x04;(`a`b!0x0102;`a`b!0x0304);0xff); '"failed"];

runPastInputTot:.binp.compileSchema"
    record point
        field xpos int
        field ypos int
    end

    record main
        field points array record point x 3
    end
    ";
if[not .binp.parse[runPastInputTot;0x01000000020000000300000004000000;`main] ~enlist[`points]!enlist(`xpos`ypos!(1 2i);`xpos`ypos!(3 4i);`endOfBuffer); '"failed"];

emptyRecArray:.binp.compileSchema"
    record rEmpty
    end

    record main
        field f1 byte
        field f2 array record rEmpty x 1
        field f3 int
    end
    ";
if[not .binp.parse[emptyRecArray; 0x0000000000;`main]~`f1`f2`f3!(0x00;enlist(`symbol$())!();0i); '"failed"];

longArraySize:.binp.compileSchema"
    record r1
        field id short
    end

    record main
        field f1 array record r1 x 16
    end
    ";
if[not .binp.parse[longArraySize; 32#0x0100;`main]~enlist[`f1]!enlist([]id:16#1h); '"failed"];

emptyArrayLast:.binp.compileSchema"
    record main
        field f1L byte
        field f1 array short xv f1L
    end
    ";
if[not .binp.parse[emptyArrayLast; enlist 0x00;`main]~`f1L`f1!(0x00;`short$()); '"failed"];

remainingSimple:.binp.compileSchema"
    record main
        field f1 int
    end
    ";
if[not .binp.parse[remainingSimple; 0x0100000002000000;`main]~`f1`xxxRemainingData!(1i;0x02000000); '"failed"];

remainingGen:.binp.compileSchema"
    record main
        field f1 int
        field f2 short
    end
    ";
if[not .binp.parse[remainingGen; 0x0100000002000000;`main]~`f1`f2`xxxRemainingData!(1i;2h;0x0000); '"failed"];

varlengths:.binp.compileSchema"
    record main
        field f1 dotnetVarLengthInt
        field f2 dotnetVarLengthInt
    end
    ";
if[not .binp.parse[varlengths;0x01bc01;`main]~`f1`f2!1 188i; '"failed"];

unsigned:.binp.compileSchema"
    record main
        field f1 int
        field f2 short
        field f3 uint
        field f4 ushort
    end
    ";
if[not .binp.parse[unsigned; 0xffffffffffffffffffffffff;`main]~`f1`f2`f3`f4!(-1i;-1h;4294967295j;65535i); '"failed"];
