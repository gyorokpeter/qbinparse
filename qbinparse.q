{
    path:"/"sv -1_"/"vs ssr[;"\\";"/"]first -3#value .z.s;
    lib:`$":",path,"/qbinparse";
    .binp.parse:lib 2:(`k_binparse_parse;3);
    .binp.parseRepeat:lib 2:(`k_binparse_parseRepeat;3);
    }[];

.binp.unLE:{$[-4h=type x;enlist x;reverse 0x00 vs x]};

.binp.compileSchemaP1P1:{[vars]
    tokens:vars`tokens;
    ptr:vars`ptr;
    out:vars`out;
    nFieldNames:vars`nFieldNames;
    nFieldSchema:vars`nFieldSchema;
    nFieldTypes:vars`nFieldTypes;
    ptr+:1; //process "field"
    fname:`$tokens[ptr];
    ptr+:1; //process field name
    nFieldNames,:fname;
    tname:tokens[ptr];
    ptr+:1; //process type name
    $[tname in ("char";"byte";"short";"int";"long";"real";"float");
        [nFieldSchema,:tb:`byte$neg type value "`",tname,"$()"; nFieldTypes,:tb];
      tname like "record";
        [nFieldSchema,:tb:`byte$neg(out[`recName]?`$tokens[ptr])+20;
            ptr+:1; //process record name
            nFieldTypes,:tb
        ];
      tname like "array";
        [
            ename:tokens[ptr];
            ptr+:1; //process element type name
            $[ename in ("char";"byte";"short";"int";"long";"real";"float");
                tb:`byte$type value "`",ename,"$()";
              ename like "record";
                [
                    tb:`byte$(out[`recName]?`$tokens[ptr])+20;
                    ptr+:1 //process record name
                ];
            {'"unknown type in array"}[]];
            nFieldSchema,:tb;
            nFieldTypes,:tb;
            szt:tokens[ptr];
            szv:tokens[ptr+1];
            ptr+:2; //process "x/xv" and length
            len:$[szt like enlist"x";
                [nFieldSchema,:0x00;n:"J"$szv;if[null n;{'"invalid size"}[]];n];
              szt like "xv";
                [n:nFieldNames?`$szv;nFieldSchema,:nFieldTypes[n];n];
              {'"unknown length specifier"}[]];
            nFieldSchema,:.binp.unLE `int$len;
        ];
    {'"unknown type in field"}[]];
    `tokens`ptr`out`nFieldNames`nFieldSchema`nFieldTypes!(tokens;ptr;out;nFieldNames;nFieldSchema;nFieldTypes)};

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
