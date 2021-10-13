{
    .binp.priv.path:"/"sv -1_"/"vs ssr[;"\\";"/"]first -3#value .z.s;
    lib:`$":",.binp.priv.path,"/qbinparse";
    .binp.priv.parse:lib 2:(`k_binparse_parse;3);
    .binp.parseRepeat:lib 2:(`k_binparse_parseRepeat;3);
    }[];

.binp.debug:0b;
.binp.parse:{[schema;data;mainType]
    if[.binp.debug; (`$":",.binp.priv.path,"/lastExample") set (schema;data;mainType)];
    .binp.priv.parse[schema;data;mainType]};

.binp.unLE:{$[-4h=type x;enlist x;reverse 0x00 vs x]};

.binp.tokenize:{{x where not(1<count each x)&x[;0]in" /\t\n"} -4!x};

.binp.error:{[vars;fn]
    pos:vars[`tokenPos][vars`ptr];
    '$[null pos 0;"";string[pos 0]," "],"line ",string[pos 1]," char ",string[pos 2],": ",fn vars[`tokens]vars`ptr;
    };

//0x8001 = case without default
//0x8002 = case with default
//0x8003 = big endian short
//0x8004 = big endian int
//0x8005 = parsed array (with length, type)
//0x8006 = .NET style variable-length int
//0x8007 = unsigned short (returned as int)
//0x8008 = unsigned int (returned as long)

.binp.compileSchemaP1P1atom:{[vars;tname]
    tb:`byte$neg type value "`",tname,"$()";
    vars[`nFieldSchema],:tb;
    vars[`nFieldTypes],:tb;
    vars};

.binp.compileSchemaP1P1atomEx:{[vars;tname]
    $[tname~"dotnetVarLengthInt";
        [
            vars[`nFieldSchema],:0x8006;
            vars[`nFieldTypes],:`byte$-6;
        ];
        '"unknown type: ",tname
    ];
    vars};

.binp.compileSchemaP1P1atomBE:{[vars]
    tname:`$vars[`tokens][vars[`ptr]];
    vars[`ptr]+:1; //process "be"
    if[not tname in `short`int; '"nyi type for \"le\": ",string[tname]];
    tb:`byte$neg type value "`",string[tname],"$()";
    extType:(`short`int!0x0304)tname;
    vars[`nFieldSchema],:0x80,extType;
    vars[`nFieldTypes],:tb;
    vars};

.binp.compileSchemaP1P1atomunsigned:{[vars;tname]
    $[tname~"ushort";
        [vars[`nFieldSchema],:0x8007;vars[`nFieldTypes],:`byte$-6];
      tname~"uint";
        [vars[`nFieldSchema],:0x8008;vars[`nFieldTypes],:`byte$-7];
      '"unknown unsgined type: ",tname
    ];
    vars};

.binp.compileSchemaP1P1record:{[vars;tname]
    recName:`$vars[`tokens][vars[`ptr]];
    if[not recName in vars[`out][`recName]; '"unknown record: ",string recName];
    recIndex:vars[`out][`recName]?recName;
    tb:`byte$neg recIndex+20;
    vars[`nFieldSchema],:tb;
    vars[`ptr]+:1; //process record name
    vars[`nFieldTypes],:tb;
    vars};

.binp.compileSchemaP1P1case:{[vars;tname]
    caseVar:`$vars[`tokens][vars[`ptr]];
    if[not caseVar in -1_vars[`nFieldNames]; .binp.error[vars;{"invalid field in case: ",x}]];
    caseVarIndex:vars[`nFieldNames]?caseVar;
    caseVarType:vars[`nFieldTypes][caseVarIndex];
    if[not caseVarType in `byte$-4 -5 -6 -7 10; .binp.error[vars;{"case variable not byte/short/int/long: ",x}]];
    vars[`ptr]+:1;    //process case variable
    caseSchema:`byte$();
    caseCount:0;
    fieldExtType:0x01;
    defaultCase:0;
    while[[
        if[vars[`ptr]>=count vars[`tokens]; .binp.error[vars;{"case \"end\" not found before end of input"}]];
        not "end"~vars[`tokens][vars[`ptr]]];
        caseLabel:vars[`tokens][vars[`ptr]];
        if[not caseLabel~"default";
            $[caseLabel[0]="\"";
                [
                    if[(not "\""=last caseLabel) or 6<>count caseLabel;
                        .binp.error[vars;{"invalid fourCC in case: ",x}]];
                    caseLabelN:0x00 sv reverse`byte$1_-1_caseLabel;
                ];[
                    caseLabelN:"I"$caseLabel;
                    if[null caseLabelN; .binp.error[vars;{"invalid case number: ",x}]];
                ]
            ]
        ];
        vars[`ptr]+:1;    //process case label

        caseRec:`$vars[`tokens][vars[`ptr]];
        if[not caseRec in vars[`out][`recName]; .binp.error[vars;{"invalid record in case: ",string caseRec}]];
        vars[`ptr]+:1;    //process case record
        caseRecN:`byte$vars[`out][`recName]?caseRec;

        $[caseLabel~"default";
            [
                if[fieldExtType=0x02; .binp.error[vars;{"more than one default case"}]];
                fieldExtType:0x02;
                defaultCase:caseRecN;
            ];[
                caseSchema,:.binp.unLE[caseLabelN],caseRecN;
                caseCount+:1;
            ]
        ];
    ];
    if[0=caseCount; .binp.error[vars;{"case field with 0 cases"}]];
    vars[`nFieldTypes],:0x80;
    vars[`nFieldSchema],:0x80  //type=ext
        ,fieldExtType   //0x01=case, 0x02=case with default
        ,(`byte$caseVarType)
        ,.binp.unLE[`int$caseVarIndex]
        ,$[fieldExtType=0x02;defaultCase;()]
        ,.binp.unLE[`int$caseCount]
        ,caseSchema;
    vars[`ptr]+:1;    //process "end"
    vars};

.binp.compileSchemaP1P1array:{[vars;parsed;tname]
    if[not parsed;
        ename:vars[`tokens][vars[`ptr]];
        vars[`ptr]+:1; //process element type name
        $[ename in ("char";"byte";"short";"int";"long";"real";"float");
            tb:`byte$type value "`",ename,"$()";
        ename like "record";
            [
                rn:`$vars[`tokens][vars[`ptr]];
                if[not rn in vars[`out][`recName]; '"rec used before defined: ",string rn];
                tb:`byte$(vars[`out][`recName]?rn)+20;
                vars[`ptr]+:1 //process record name
            ];
        {'"unknown type in array: ",x}[ename]];
        vars[`nFieldSchema],:tb;
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
    vars[`nFieldSchema],:.binp.unLE `int$len;
    if[parsed; vars:.binp.compileSchemaFieldType[vars]];
    vars};

.binp.compileSchemaFieldType:{[vars]
    tname:vars[`tokens][vars[`ptr]];
    vars[`ptr]+:1; //process type name
    $[tname in ("char";"byte";"short";"int";"long";"real";"float");
        vars:.binp.compileSchemaP1P1atom[vars;tname];
      tname in ("ushort";"uint");
        vars:.binp.compileSchemaP1P1atomunsigned[vars;tname];
      tname~"dotnetVarLengthInt";
        vars:.binp.compileSchemaP1P1atomEx[vars;tname];
      tname~"be";
        vars:.binp.compileSchemaP1P1atomBE[vars];
      tname~"record";
        vars:.binp.compileSchemaP1P1record[vars;tname];
      tname~"case";
        vars:.binp.compileSchemaP1P1case[vars;tname];
      tname~"array";
        vars:.binp.compileSchemaP1P1array[vars;0b;tname];
      tname~"parsedArray";
        vars:.binp.compileSchemaP1P1array[vars;1b;tname];
    {'"unknown type in field: ",x}[tname]];
    vars};

.binp.compileSchemaP1P1:{[vars]
    vars[`ptr]+:1; //process "field"
    fname:`$vars[`tokens][vars[`ptr]];
    vars[`ptr]+:1; //process field name
    vars[`nFieldNames],:fname;
    vars:.binp.compileSchemaFieldType[vars];
    vars};

.binp.compileSchemaP1:{[vars]
    if[not vars[`tokens][vars[`ptr]]~"record"; '"expected \"record\", found ",.Q.s1 vars[`tokens][vars[`ptr]]];
    vars[`ptr]+:1;
    nRecName:`$vars[`tokens][vars[`ptr]];
    vars[`nFieldNames]:`$();
    vars[`nFieldSchema]:`byte$();
    vars[`nFieldTypes]:`byte$();
    vars[`ptr]+:1; //process rec name
    while[not (vars[`tokens][vars[`ptr]])~"end";  //'branch
        if[not (vars[`tokens][vars[`ptr]])~"field"; '"expected \"field\", found ",.Q.s1 vars[`tokens][vars[`ptr]]];
        vars:.binp.compileSchemaP1P1[vars];
    ];
    if[not vars[`tokens][vars[`ptr]]~"end"; '"expected \"end\", found ",.Q.s1 vars[`tokens][vars[`ptr]]];
    vars[`ptr]+:1;  //process "end"
    vars[`out],:enlist(nRecName;vars`nFieldNames;`char$vars`nFieldSchema);
    vars};

.binp.getTokens:{[file;schema]
    tokens0:.binp.tokenize schema;
    tokenStarts:0,-1_sums count each tokens0;
    lineStarts:0,1+schema ss "\n";
    tokenLines:lineStarts bin tokenStarts;
    tokenPos0:(1+tokenLines),'1+tokenStarts-lineStarts tokenLines;
    keepTokens:where not tokens0 in enlist each " \t\r\n";
    tokens:tokens0 keepTokens;
    tokenPos:enlist[file],/:tokenPos0 keepTokens;
    (tokens;tokenPos)};

.binp.compileSchema:{[schema]
    if[-11h=type schema;
        schema:enlist schema;
    ];
    schema:$[11h=type schema;
        (`$1_/:string schema)!{"\n"sv read0 x} each schema;
        enlist[`]!enlist schema
    ];
    tokenres:raze flip .binp.getTokens'[key schema;value schema];
    ptr:0;
    out:([]recName:`$(); fieldName:(); fieldSchema:());
    tokens:tokenres 0;
    vars:`tokens`tokenPos`ptr`out!(tokens;tokenres 1;ptr;out);
    while[ptr<count tokens; //'branch
        vars:.binp.compileSchemaP1[vars];
        ptr:vars`ptr;
    ];
    out:vars`out;
    value flip out};
