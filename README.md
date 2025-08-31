# qbinparse
Customizable binary data parser

The goal is to be able to parse various binary formats (e.g. network protocols and data files)
in a flexible rapid-development compatible way.

# Build
Download k.h and libq.a from Kx website.

An example build script is in ```b.cmd```. You might have to fix the -I and -L parameters 
so they point to the directories of k.h and libq.a.

# Loading 
Requires KDB 3.5 for the enhanced lambda metadata

```q
\l path/to/qbinparse/qbinparse.q
```

# Usage
## Schema format
The parser uses a simple language to describe the schema of the data.

The schema is a series of record definitions in the form *record* _fields_ *end*.

Each field definition is in the form *field* _name_ _type_.

The possible types are:
* `byte`, `char`, `short`, `int`, `long`, `real`, `float`: same meaning as in q
* `uint`, `ushort`: unsigned value that is represented in q with the next bigger integer type (an `ushort` is returned as an int and an `uint` is returned as a long). There is no `ulong` because there is no integer type in q that is able to represent all of its values.
* `dotnetVarLengthInt`: an integer that is stored in a way compatible with .NET's variable-length integer serialization format (each byte encodes 7 bits of the original integer, with the most significant bit indicating that there are more bytes to follow)
* `record` _recordName_: a nested record
* `array` _elementType_ _size_: an array
  * _elementType_ can be the atomic types or record, currently multi-dimensional array is not supported
  * _size_ can be specified as:
    * `x` _number_: constant length
    * `xv` _fieldName_: length is the value of the specified field
    * `xz`: zero-terminated string
    * `tpb` _number_: array has a guard byte with the value _number_ after it
    * `tps` _number_: array has a guard short with the value _number_ after it
    * `tpi` _number_: array has a guard int with the value _number_ after it
    * `repeat`: array extends up to the end of the available input - this should be the last element in the main record or used in a `parsedArray`
* `parsedArray` _size_ _elementType_: an array with internal structure. The size specifies the number of bytes the array takes up, then the parsing process is recursively called on the array. _elementType_ must be a full field type, typically an array or record. If a regular `array` is used within a `parsedArray`, it will have its own _size_, which can be `repeat` to make it cover the entire `parsedArray`.
* `case` _fieldName_ _val1_ _rec1_ _val2_ _rec2_ ... [`default` _recD_]: a variable-type field that is parsed as one of the specified records based on the value of the tag field. _valN_ are either integers or four-character strings. An optional default case can be added that covers values not listed in the cases.

In addition the type may be preceded by an operator. The following operator is supported:
* `recSize`: the field contains the record size, during parsing the "end of record" (for determining which fields run past the end of the input and how much data a `repeat` field can consume) is set according to this size.

## Parsing
First compile the schema:
```q
schema:.binp.compileSchema schemaStr;
```
Then use the compiled schema on the data:
```q
.binp.parse[schema;0x0000;`mainType]
```

## Serialization (unparsing)
This is the inverse operation of .binp.parse:
```q
.binp.unparse[schema;`a`b!1 2;`mainType]
```

## Examples

An array of 4 ints:
```q
schemaStr:"
    record simple
        field nums array int x 4
    end";
schema:.binp.compileSchema schemaStr;
.binp.parse[schema;0x01000000020000000300000004000000;`simple]
```
Returns: ```enlist[`nums]!enlist 1 2 3 4i```

The inverse operation:
```q
q).binp.unparse[schema;enlist[`nums]!enlist 1 2 3 4;`simple]
0x01000000020000000300000004000000
```

A string with a two-byte length prepended:
```q
schemaStr:"
    record stringWithShortLen
        field length short
        field str array char xv length
    end";
schema:.binp.compileSchema schemaStr;
.binp.parse[schema;0x060048656c6c6f;`stringWithShortLen]
```
Returns: ``` `length`str!(6h;"Hello") ```

See also [examples/parse.q](examples/parse.q) for parsing and [examples/unparse.q](examples/unparse.q) for unparsing.

## Error handling
Parsing failures don't throw errors but instead return a partial object with the error inserted into the value of the problematic field as a symbol. Possible errors include:
* `endOfBuffer`: attempt to read a field when the read position is already at the end of the input
* `arrayRunsPastInput`: an array has a size that would make it cover data past the end of the input
* `tooLargeArray`: the array size wouldn't fit into 32 bits
* `noCaseMatch`: a `case` field encountered an input value that is not among the cases and there is no `default` case

Furthermore if there are extra bytes left over after parsing the main record, the leftover bytes are added to the record with a field named `xxxRemainingData`. This is also considered a type of error and in particular the `.binp.unparse` function will ignore this field. To describe a format that allows garbage/padding/irrelevant data at the end, use an `array byte repeat` field as the last field to capture all the remaining bytes.

# Rationale
*Why not use Google Protocol Buffers?*

Google Protocol Buffers (GPB) is another schema-based serialization library. If you are developing a new application, you might want to use GPB for communication. However, a major difference is that GPB places its own constraints on the binary data. GPB uses its own format and only stuffs the content based on the schema into that format. If you need to interface with an existing application, you can only use GPB if you are able to modify the application or you are lucky and it is already using GPB. In contrast, qbinparse places no constraints on what the binary data must look like - it is the schema that describes every single byte. It is meant for interfacing with existing applications that are not modifiable, as well as for file formats with a pre-existing format specification.
