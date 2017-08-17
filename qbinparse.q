{
    path:"/"sv -1_"/"vs ssr[;"\\";"/"]first -3#value .z.s;
    lib:`$":",path,"/qbinparse";
    .binp.parse:lib 2:(`k_binparse_parse;3);
    .binp.parseRepeat:lib 2:(`k_binparse_parseRepeat;3);
    }[];

.binp.unLE:{$[-4h=type x;enlist x;reverse 0x00 vs x]};

.binp.compileSchemaP1P1atom:{[vars;tname]
    tb:`byte$neg type value "`",tname,"$()";
    vars[`nFieldSchema],:tb;
    vars[`nFieldTypes],:tb;
    vars};

.binp.compileSchemaP1P1record:{[vars;tname]
    tb:`byte$neg(vars[`out][`recName]?`$vars[`tokens][vars[`ptr]])+20;
    vars[`nFieldSchema],:tb;
    vars[`ptr]+:1; //process record name
    vars[`nFieldTypes],:tb;
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
    {'"unknown type in array"}[]];
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
    while[not (tokens[ptr]) like "end";  //'branch
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
    tokens:{x where 0<count each x}(-4!schema) except\:" \t\r\n";
    ptr:0;
    out:([]recName:`$(); fieldName:(); fieldSchema:());
    vars:`tokens`ptr`out!(tokens;ptr;out);
    while[ptr<count tokens; //'branch
        vars:.binp.compileSchemaP1[vars];
        ptr:vars`ptr;
    ];
    out:vars`out;
    value flip out};
