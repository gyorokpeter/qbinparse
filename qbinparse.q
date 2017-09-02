{
    path:"/"sv -1_"/"vs ssr[;"\\";"/"]first -3#value .z.s;
    lib:`$":",path,"/qbinparse";
    .binp.parse:lib 2:(`k_binparse_parse;3);
    .binp.parseRepeat:lib 2:(`k_binparse_parseRepeat;3);
    }[];

.binp.unLE:{$[-4h=type x;enlist x;reverse 0x00 vs x]};

.binp.tokenize:{{x where not(1<count each x)&x[;0]in" /\t\n"} -4!x};

.binp.compileSchemaP1P1atom:{[vars;tname]
    tb:`byte$neg type value "`",tname,"$()";
    vars[`nFieldSchema],:tb;
    vars[`nFieldTypes],:tb;
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
    vars[`ptr]+:1;    //process case variable
    if[not caseVar in -1_vars[`nFieldNames]; '"invalid field in case: ",string caseVar];
    caseVarIndex:vars[`nFieldNames]?caseVar;
    caseVarType:vars[`nFieldTypes][caseVarIndex];
    if[not caseVarType in `byte$-4 -5 -6 -7; '"case variable not byte/short/int/long: ",string caseVar];
    caseSchema:`byte$();
    caseCount:0;
    fieldExtType:0x01;
    defaultCase:0;
    while[[
        if[vars[`ptr]>=count vars[`tokens]; '"case \"end\" not found before end of input"];
        not "end"~vars[`tokens][vars[`ptr]]];
        caseLabel:vars[`tokens][vars[`ptr]];
        vars[`ptr]+:1;    //process case label
        caseRec:`$vars[`tokens][vars[`ptr]];
        vars[`ptr]+:1;    //process case record
        if[not caseRec in vars[`out][`recName]; '"invalid record in case: ",string caseRec];
        caseRecN:`byte$vars[`out][`recName]?caseRec;
        $[caseLabel~"default";
            [
                if[fieldExtType=0x02; '"more than one default case"];
                fieldExtType:0x02;
                defaultCase:caseRecN;
            ];[
                caseLabelN:"I"$caseLabel;
                if[null caseLabelN; '"invalid case number: ",caseLabel];
                caseSchema,:.binp.unLE[caseLabelN],caseRecN;
                caseCount+:1;
            ]
        ];
    ];
    if[0=caseCount; '"case field with 0 cases"];
    vars[`nFieldTypes],:0x80;
    vars[`nFieldSchema],:0x80  //type=ext
        ,fieldExtType   //0x01=case, 0x02=case with default
        ,(`byte$0-caseVarType)
        ,.binp.unLE[`int$caseVarIndex]
        ,$[fieldExtType=0x02;defaultCase;()]
        ,.binp.unLE[`int$caseCount]
        ,caseSchema;
    vars[`ptr]+:1;    //process "end"
    vars};

.binp.compileSchemaP1P1array:{[vars;tname]
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
      {'"unknown length specifier: ",x}[szt]];
    vars[`nFieldSchema],:.binp.unLE `int$len;
    vars};

.binp.compileSchemaP1P1:{[vars]
    vars[`ptr]+:1; //process "field"
    fname:`$vars[`tokens][vars[`ptr]];
    vars[`ptr]+:1; //process field name
    vars[`nFieldNames],:fname;
    tname:vars[`tokens][vars[`ptr]];
    vars[`ptr]+:1; //process type name
    $[tname in ("char";"byte";"short";"int";"long";"real";"float");
        vars:.binp.compileSchemaP1P1atom[vars;tname];
      tname~"record";
        vars:.binp.compileSchemaP1P1record[vars;tname];
      tname~"case";
        vars:.binp.compileSchemaP1P1case[vars;tname];
      tname~"array";
        vars:.binp.compileSchemaP1P1array[vars;tname];
    {'"unknown type in field: ",x}[tname]];
    vars};

.binp.compileSchemaP1:{[vars]
    tokens:vars`tokens;
    ptr:vars`ptr;
    out:vars`out;
    ptr+:1; //process "record"
    nRecName:`$tokens[ptr];
    nFieldNames:`$();
    nFieldSchema:`byte$();
    nFieldTypes:`byte$();
    ptr+:1; //process rec name
    vars2:`tokens`ptr`out`nFieldNames`nFieldSchema`nFieldTypes!(tokens;ptr;out;nFieldNames;nFieldSchema;nFieldTypes);
    while[not (tokens[ptr])~"end";  //'branch
        if[not (tokens[ptr])~"field"; '"expected \"field\", found ",(tokens[ptr])];
        vars2:.binp.compileSchemaP1P1[vars2];
        ptr:vars2`ptr;
    ];
    ptr+:1; //process "end"
    nFieldNames:vars2`nFieldNames;
    nFieldSchema:vars2`nFieldSchema;
    nFieldTypes:vars2`nFieldTypes;
    out,:enlist(nRecName;nFieldNames;`char$nFieldSchema);
    `tokens`ptr`out!(tokens;ptr;out)};

.binp.compileSchema:{[schema]
    tokens:{x where 0<count each x}(.binp.tokenize schema) except\:" \t\r\n";
    ptr:0;
    out:([]recName:`$(); fieldName:(); fieldSchema:());
    vars:`tokens`ptr`out!(tokens;ptr;out);
    while[ptr<count tokens; //'branch
        vars:.binp.compileSchemaP1[vars];
        ptr:vars`ptr;
    ];
    out:vars`out;
    value flip out};
