debug:0b;

lib:$[.z.o like "w*";  //hack until fixed in core
    {
        path:"/"sv -1_"/"vs ssr[;"\\";"/"]first -3#value .z.s;
        .Q.m.SP,:enlist path;
        use`qbinparse_c
    }[];
    use`..qbinparse_c];

.z.m.parse:{[schema;data;mainType]
    if[debug; `:::lastExample set (schema;data;mainType)];
    lib.parse[schema;data;mainType]};

unLE:{$[-4h=type x;enlist x;reverse 0x00 vs x]};

tokenize:{{x where not(1<count each x)&x[;0]in" /\t\n"} -4!x};

error:{[vars;fn]
    pos:vars[`tokenPos][vars`ptr];
    '$[null pos 0;"";string[pos 0]," "],"line ",string[pos 1]," char ",string[pos 2],": ",fn vars[`tokens]vars`ptr;
    };

//extended types:
//0x8001 = case without default
//0x8002 = case with default
//0x8003 = big endian short
//0x8004 = big endian int
//0x8005 = parsed array (with length, type)
//0x8006 = .NET style variable-length int
//0x8007 = unsigned short (returned as int)
//0x8008 = unsigned int (returned as long)
//0x8009 = unsigned big endian short (returned as int)
//0x800a = unsigned big endian int (returned as long)
//operators:
//0x7f01 = recSize


compileSchemaP1P1atom:{[vars;tname]
    tb:`byte$neg type value "`",tname,"$()";
    vars[`nFieldSchema],:tb;
    vars[`nFieldTypes],:tb;
    vars};

compileSchemaP1P1atomEx:{[vars;tname]
    $[tname~"dotnetVarLengthInt";
        [
            vars[`nFieldSchema],:0x8006;
            vars[`nFieldTypes],:`byte$-6;
        ];
        '"unknown type: ",tname
    ];
    vars};

compileSchemaP1P1atomBE:{[vars]
    tname:`$vars[`tokens][vars[`ptr]];
    vars[`ptr]+:1; //process "be"
    if[not tname in `short`int`ushort`uint; '"nyi type for \"be\": ",string[tname]];
    tb:`byte$neg type (`short`int`ushort`uint!`short`int`short`int)[tname]$();
    extType:(`short`int`ushort`uint!0x0304090a)tname;
    vars[`nFieldSchema],:0x80,extType;
    vars[`nFieldTypes],:tb;
    vars};

compileSchemaP1P1atomunsigned:{[vars;tname]
    $[tname~"ushort";
        [vars[`nFieldSchema],:0x8007;vars[`nFieldTypes],:`byte$-6];
      tname~"uint";
        [vars[`nFieldSchema],:0x8008;vars[`nFieldTypes],:`byte$-7];
      '"unknown unsgined type: ",tname
    ];
    vars};

compileSchemaP1P1record:{[vars;tname]
    recName:`$vars[`tokens][vars[`ptr]];
    if[not recName in vars[`out][`recName]; '"unknown record: ",string recName];
    recIndex:vars[`out][`recName]?recName;
    tb:`byte$neg recIndex+20;
    vars[`nFieldSchema],:tb;
    vars[`ptr]+:1; //process record name
    vars[`nFieldTypes],:tb;
    vars};

compileSchemaP1P1case:{[vars;tname]
    caseVar:`$vars[`tokens][vars[`ptr]];
    if[not caseVar in -1_vars[`nFieldNames]; error[vars;{"invalid field in case: ",x}]];
    caseVarIndex:vars[`nFieldNames]?caseVar;
    caseVarType:vars[`nFieldTypes][caseVarIndex];
    if[not caseVarType in `byte$-4 -5 -6 -7 10; error[vars;{"case variable not byte/short/int/long: ",x}]];
    vars[`ptr]+:1;    //process case variable
    caseSchema:`byte$();
    caseCount:0;
    fieldExtType:0x01;
    defaultCase:0;
    while[[
        if[vars[`ptr]>=count vars[`tokens]; error[vars;{"case \"end\" not found before end of input"}]];
        not "end"~vars[`tokens][vars[`ptr]]];
        caseLabel:vars[`tokens][vars[`ptr]];
        if[not caseLabel~"default";
            $[caseLabel[0]="\"";
                [
                    if[(not "\""=last caseLabel) or 6<>count caseLabel;
                        error[vars;{"invalid fourCC in case: ",x}]];
                    caseLabelN:0x00 sv reverse`byte$1_-1_caseLabel;
                ];[
                    caseLabelN:"I"$caseLabel;
                    if[null caseLabelN; error[vars;{"invalid case number: ",x}]];
                ]
            ]
        ];
        vars[`ptr]+:1;    //process case label

        caseRec:`$vars[`tokens][vars[`ptr]];
        if[not caseRec in vars[`out][`recName]; error[vars;{[caseRec;x]"invalid record in case: ",string caseRec}[caseRec]]];
        vars[`ptr]+:1;    //process case record
        caseRecN:`byte$vars[`out][`recName]?caseRec;

        $[caseLabel~"default";
            [
                if[fieldExtType=0x02; error[vars;{"more than one default case"}]];
                fieldExtType:0x02;
                defaultCase:caseRecN;
            ];[
                caseSchema,:unLE[caseLabelN],caseRecN;
                caseCount+:1;
            ]
        ];
    ];
    if[0=caseCount; error[vars;{"case field with 0 cases"}]];
    vars[`nFieldTypes],:0x80;
    vars[`nFieldSchema],:0x80  //type=ext
        ,fieldExtType   //0x01=case, 0x02=case with default
        ,(`byte$caseVarType)
        ,unLE[`int$caseVarIndex]
        ,$[fieldExtType=0x02;defaultCase;()]
        ,unLE[`int$caseCount]
        ,caseSchema;
    vars[`ptr]+:1;    //process "end"
    vars};

compileSchemaP1P1array:{[vars;parsed;tname]
    if[not parsed;
        ename:vars[`tokens][vars[`ptr]];
        vars[`ptr]+:1; //process element type name
        $[ename in ("char";"byte";"short";"int";"long";"real";"float");
            ta:tb:`byte$type value "`",ename,"$()";
        ename like "record";
            [
                rn:`$vars[`tokens][vars[`ptr]];
                if[not rn in vars[`out][`recName]; // '"rec used before defined: ",string rn];
                    vars[`out],:enlist(rn;::;::);
                ];
                ta:tb:`byte$(vars[`out][`recName]?rn)+20;
                vars[`ptr]+:1 //process record name
            ];
        ename in ("uint";"ushort");
            [
                ind:("ushort";"uint")?ename;
                ta:0x00,(0x8007;0x8008)ind;
                tb:`byte$-5 -6h ind;
            ];
        ename like "be";
            [
                ename2:vars[`tokens][vars[`ptr]];
                vars[`ptr]+:1; //process next part of element type name
                ind:("short";"int";"ushort";"uint")?ename2;
                if[ind=4;{'x}"nyi type for \"be\": ",ename2];
                ta:0x0080,0x0304090a ind;
                tb:`byte$5 6 5 6h ind;
            ];
        {'"unknown type in array: ",x}[ename]];
        vars[`nFieldSchema],:ta;
        vars[`nFieldTypes],:tb;
    ];
    if[parsed;
        vars[`nFieldSchema],:0x8005;
    ];
    szt:vars[`tokens][vars[`ptr]];
    szv:vars[`tokens][vars[`ptr]+1];
    vars[`ptr]+:1; //process "x/xv"
    len:$[szt~enlist"x";
        [vars[`nFieldSchema],:0x00;
            n:"J"$szv;
            if[null n;{'"invalid size"}[]];
            vars[`ptr]+:1; //process length
            n];
      szt~"xv";
        [n:vars[`nFieldNames]?`$szv;
            if[n=count vars[`nFieldNames];{'"uknown field in length"}[]];
            vars[`nFieldSchema],:vars[`nFieldTypes][n];
            vars[`ptr]+:1; //process length
            n];
      szt~"xz";
        [vars[`nFieldSchema],:0x01;0];
      szt~"tpb";
        [vars[`nFieldSchema],:0x02;
            guard:"I"$vars[`tokens][vars[`ptr]];
            vars[`ptr]+:1;  //process guard
            guard];
      szt~"tps";
        [vars[`nFieldSchema],:0x03;
            guard:"I"$vars[`tokens][vars[`ptr]];
            vars[`ptr]+:1;  //process guard
            guard];
      szt~"tpi";
        [vars[`nFieldSchema],:0x04;
            guard:"I"$vars[`tokens][vars[`ptr]];
            vars[`ptr]+:1;  //process guard
            guard];
      szt~"repeat";
        [vars[`nFieldSchema],:0x05;
            0i];
      {'"unknown length specifier: ",x}[szt]];
    vars[`nFieldSchema],:unLE `int$len;
    if[parsed; vars:compileSchemaFieldType[vars]];
    vars};

operators:enlist[""]!enlist(::);
operators["recSize"]:{x[`nFieldSchema],:0x7f01;x};

compileSchemaFieldType:{[vars]
    tname:vars[`tokens][vars[`ptr]];
    vars[`ptr]+:1; //process type name
    while[tname in key operators;
        vars:operators[tname][vars];
        tname:vars[`tokens][vars[`ptr]];
        vars[`ptr]+:1; //process type name
    ];
    $[tname in ("char";"byte";"short";"int";"long";"real";"float");
        vars:compileSchemaP1P1atom[vars;tname];
      tname in ("ushort";"uint");
        vars:compileSchemaP1P1atomunsigned[vars;tname];
      tname~"dotnetVarLengthInt";
        vars:compileSchemaP1P1atomEx[vars;tname];
      tname~"be";
        vars:compileSchemaP1P1atomBE[vars];
      tname~"record";
        vars:compileSchemaP1P1record[vars;tname];
      tname~"case";
        vars:compileSchemaP1P1case[vars;tname];
      tname~"array";
        vars:compileSchemaP1P1array[vars;0b;tname];
      tname~"parsedArray";
        vars:compileSchemaP1P1array[vars;1b;tname];
    {'"unknown type in field: ",x}[tname]];
    vars};

compileSchemaP1P1:{[vars]
    vars[`ptr]+:1; //process "field"
    fname:`$vars[`tokens][vars[`ptr]];
    vars[`ptr]+:1; //process field name
    vars[`nFieldNames],:fname;
    vars:compileSchemaFieldType[vars];
    vars};

compileSchemaP1:{[vars]
    if[not vars[`tokens][vars[`ptr]]~"record"; '"expected \"record\", found ",.Q.s1 vars[`tokens][vars[`ptr]]];
    vars[`ptr]+:1;
    nRecName:`$vars[`tokens][vars[`ptr]];
    vars[`nFieldNames]:`$();
    vars[`nFieldSchema]:`byte$();
    vars[`nFieldTypes]:`byte$();
    vars[`ptr]+:1; //process rec name
    while[not (vars[`tokens][vars[`ptr]])~"end";  //'branch
        if[not (vars[`tokens][vars[`ptr]])~"field"; '"expected \"field\", found ",.Q.s1 vars[`tokens][vars[`ptr]]];
        vars:compileSchemaP1P1[vars];
    ];
    if[not vars[`tokens][vars[`ptr]]~"end"; '"expected \"end\", found ",.Q.s1 vars[`tokens][vars[`ptr]]];
    vars[`ptr]+:1;  //process "end"
    $[nRecName in vars[`out;`recName];
        [
            ind:vars[`out;`recName]?nRecName;
            vars[`out;ind]:(nRecName;vars`nFieldNames;`char$vars`nFieldSchema);
        ];
        vars[`out],:enlist(nRecName;vars`nFieldNames;`char$vars`nFieldSchema)
    ];
    vars};

getTokens:{[file;schema]
    tokens0:tokenize schema;
    tokenStarts:0,-1_sums count each tokens0;
    lineStarts:0,1+schema ss "\n";
    tokenLines:lineStarts bin tokenStarts;
    tokenPos0:(1+tokenLines),'1+tokenStarts-lineStarts tokenLines;
    keepTokens:where not tokens0 in enlist each " \t\r\n";
    tokens:tokens0 keepTokens;
    tokenPos:enlist[file],/:tokenPos0 keepTokens;
    (tokens;tokenPos)};

compileSchema:{[schema]
    if[-11h=type schema;
        schema:enlist schema;
    ];
    schema:$[11h=type schema;
        (`$1_/:string schema)!{"\n"sv read0 x} each schema;
        enlist[`]!enlist schema
    ];
    tokenres:raze flip getTokens'[key schema;value schema];
    ptr:0;
    out:([]recName:`$(); fieldName:(); fieldSchema:());
    tokens:tokenres 0;
    vars:`tokens`tokenPos`ptr`out!(tokens;tokenres 1;ptr;out);
    while[ptr<count tokens; //'branch
        vars:compileSchemaP1[vars];
        ptr:vars`ptr;
    ];
    out:vars`out;
    if[count bad:exec recName from out where (::)~/:fieldName; '"record not found: ",", "sv string bad];
    value flip out};

export:([.z.m.parse;lib.parseRepeat;lib.unparse;compileSchema]);
