\c 100000 100000

{
    path:"/"sv -1_"/"vs ssr[;"\\";"/"]first -3#value .z.s;
    system"l ",path,"/../qbinparse.q";
    }[];

simpleRecordByte:.binp.compileSchema"
    record point
        field xpos byte
        field ypos byte
    end
    ";
if[not .binp.unparse[simpleRecordByte;`ypos`xpos!(0x0201);`point]~0x0102;'"failed"];
if[not .binp.unparse[simpleRecordByte;`xpos`ypos!(0x0102);`point]~0x0102;'"failed"];

simpleRecord:.binp.compileSchema"
    record point
        field xpos real
        field ypos real
    end
    ";
if[not .binp.unparse[simpleRecord;`xpos`ypos!(120e;130e);`point]~0x0000f04200000243;'"failed"];

mixedRecord:.binp.compileSchema"
    record point
        field xpos real
        field ypos short
    end
    ";
if[not .binp.unparse[mixedRecord;`xpos`ypos!(120e;-10000h);`point]~0x0000f042f0d8;'"failed"];

simpleArray:.binp.compileSchema"
    record a
        field ints array int x 4
    end
    ";
if[not .binp.unparse[simpleArray;enlist[`ints]!enlist 1 2 3 4i;`a]~0x01000000020000000300000004000000;'"failed"];
if[not .binp.unparse[simpleArray;enlist[`ints]!enlist"abcd";`a]~0x61000000620000006300000064000000;'"failed"];

if[not .[.binp.unparse;(simpleArray;enlist[`ints]!enlist 1 2 3 4 5i;`a);::]~"a.ints: expected 4 items, got 5";'"failed"];

multipleSimpleArrays:.binp.compileSchema"
    record a
        field ints array int x 4
        field shorts array short x 5
        field bytes array byte x 4
        field chars array char x 4
    end
    ";
if[not .binp.unparse[multipleSimpleArrays;`ints`shorts`bytes`chars!(1 2 3 4i;5 6 7 8 9h;10 11 12 13;14 15 16 17);`a]~
    0x01000000020000000300000004000000050006000700080009000a0b0c0d0e0f1011; '"failed"];

varLengthArray:.binp.compileSchema"
    record a
        field intsL short
        field ints array int xv intsL
    end
    ";
if[not .binp.unparse[varLengthArray;`intsL`ints!(3h;1 2 3i);`a]~0x0300010000000200000003000000;'"failed"];

recArray:.binp.compileSchema"
    record point
        field xpos int
        field ypos int
    end

    record a
        field points array record point x 2
    end
    ";
if[not .[.binp.unparse;(recArray;enlist[`points]!enlist(`xpos`ypos!(1i;2i);`xpos`ypos!(3i;4i);`xpos`ypos!(5i;6i));`a);::]~"a.points: expected 2 items, got 3";'"failed"];
if[not .[.binp.unparse;(recArray;enlist[`points]!enlist(`xpos`ypos!(1i;2i);`xpos`ypos!(3i;4i);`xpos`zpos!(5i;6i));`a);::]~"a.points: expected 2 items, got 3";'"failed"];
if[not .[.binp.unparse;(recArray;enlist[`points]!enlist(`xpos`zpos!(1i;2i);`xpos`zpos!(3i;4i));`a);::]~"a.points: missing field: ypos";'"failed"];
if[not .binp.unparse[recArray;enlist[`points]!enlist(`xpos`ypos!(1i;2i);`xpos`ypos!(3i;4i));`a]~0x01000000020000000300000004000000;'"failed"];
if[not .binp.unparse[recArray;enlist[`points]!enlist(`xpos`ypos!(1i;2i);`xpos`ypos`zpos!(3i;4i;5i));`a]~0x01000000020000000300000004000000;'"failed"];

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
if[not .binp.unparse[varLengthRecArray;`pointsL`points!(2h;(`xpos`ypos!(1i;2i);`xpos`ypos!(3i;4i)));`a]~0x020001000000020000000300000004000000;'"failed"];

byteGuardAtomicArray:.binp.compileSchema"
    record a
        field items array short tpb 3
        field guard byte
        field other byte
    end
    ";
if[not .binp.unparse[byteGuardAtomicArray;`items`guard`other!(513 1541 2055 2826h;0x03;0x04);`a]~0x0102050607080a0b0304;'"failed"];

shortGuardAtomicArray:.binp.compileSchema"
    record a
        field items array short tps 3
        field guard short
        field other byte
    end
    ";
if[not .binp.unparse[shortGuardAtomicArray;`items`guard`other!(513 1027 1541 2055h;3h;0x0a);`a]~0x010203040506070803000a;'"failed"];

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
if[not .binp.unparse[byteGuardRecordArray;`items`guard`other!((`xpos`ypos!1 2i;`xpos`ypos!3 4i);0xff;0x04);`a]~0x01000000020000000300000004000000ff04;'"failed"];

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
if[not .binp.unparse[recordWithCase;
    `tags`guard!((`tag`data!(0x00;enlist[`contInt]!enlist 1i);
                 `tag`data!(0x01;enlist[`contShort]!enlist 2h);
                 `tag`data!(0x00;enlist[`contInt]!enlist 3i);
                 `tag`data!(0x01;enlist[`contShort]!enlist 4h));
                 0x02);
    `main]~0x0001000000010200000300000001040002;'"failed"];

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
if[not .binp.unparse[recordWithCase2;
    `tags`guard!((`tag`data!(0j;enlist[`contInt]!enlist 1i);
                 `tag`data!(1j;enlist[`contShort]!enlist 2h);
                 `tag`data!(0j;enlist[`contInt]!enlist 3i);
                 `tag`data!(1j;enlist[`contShort]!enlist 4h))
                 ;0x02);
    `main]~0x000000000000000001000000010000000000000002000000000000000000030000000100000000000000040002;'"failed"];

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
if[not .binp.unparse[recordWithCaseDefault;
    `tags`guard!((`tag`data!(0x00;enlist[`contInt]!enlist 1i);
    `tag`data!(0x38;enlist[`contShort]!enlist 2h);
    `tag`data!(0x00;enlist[`contInt]!enlist 3i);
    `tag`data!(0x4f;enlist[`contShort]!enlist 4h))
    ;0x02);`main]~(0x000100000038020000030000004f040002);'"failed"];

bigEndian:.binp.compileSchema"
    record main
        field f1 be short
        field f2 be int
    end
    ";

if[not .binp.unparse[bigEndian;`f1`f2!(1h;1i);`main]~0x000100000001;'"failed"];

repeatingAtom:.binp.compileSchema"
    record main
        field f1 array int repeat
    end
    ";
if[not .binp.unparse[repeatingAtom;enlist[`f1]!enlist 1 2 3i;`main]~0x010000000200000003000000;'"failed"];

repeatingRecord:.binp.compileSchema"
    record item
        field a int
        field b int
    end
    record main
        field f1 array record item repeat
    end
    ";
if[not .binp.unparse[repeatingRecord;enlist[`f1]!enlist(`a`b!1 2i;`a`b!3 4i);`main]~0x01000000020000000300000004000000;'"failed"];

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
if[not .binp.unparse[emptyRecordList;`f1`f2`f3!(1i;();2i);`main]~0x0100000002000000;'"failed"];

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
if[not .[.binp.unparse;(sizeTaggedRecord;`f1C`f1`f2!(0x04;(`a`b!0x0102;`a`b!0x0304;`a`b!0x0506);0xff);`main);::]~"main.f1: can't fit parsed data: max(4)<actual(6)";'"failed"];
if[not .binp.unparse[sizeTaggedRecord;`f1C`f1`f2!(0x04;(`a`b!0x0102;`a`b!0x0304);0xff);`main]~0x0401020304ff;'"failed"];
if[not .binp.unparse[sizeTaggedRecord;`f1C`f1`f2!(0x04;enlist`a`b!0x0102;0xff);`main]~0x0401020000ff;'"failed"];    //zero-fill

emptyRecArray:.binp.compileSchema"
    record rEmpty
    end

    record main
        field f1 byte
        field f2 array record rEmpty x 1
        field f3 int
    end
    ";
if[not .binp.unparse[emptyRecArray;`f1`f2`f3!(0x00;enlist(`symbol$())!();0i);`main]~0x0000000000;'"failed"];

longArraySize:.binp.compileSchema"
    record r1
        field id short
    end

    record main
        field f1 array record r1 x 16
    end
    ";
if[not .binp.unparse[longArraySize;enlist[`f1]!enlist([]id:16#1h);`main]~32#0x0100;'"failed"];

emptyArrayLast:.binp.compileSchema"
    record main
        field f1L byte
        field f1 array short xv f1L
    end
    ";
if[not .binp.unparse[emptyArrayLast;`f1L`f1!(0x00;`short$());`main]~enlist 0x00;'"failed"];

varlengths:.binp.compileSchema"
    record main
        field f1 dotnetVarLengthInt
        field f2 dotnetVarLengthInt
    end
    ";
if[not .binp.unparse[varlengths;`f1`f2!1 188i;`main]~0x01bc01; '"failed"];

unsigned:.binp.compileSchema"
    record main
        field f1 int
        field f2 short
        field f3 uint
        field f4 ushort
    end
    ";
if[not .binp.unparse[unsigned;`f1`f2`f3`f4!(-1i;-1h;4294967295j;65535i);`main]~0xffffffffffffffffffffffff;'"failed"];
