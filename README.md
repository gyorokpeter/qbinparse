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
* byte, char, short, int, long, real, float: same meaning as in Q
* record _recordName_: a nested record
* array _elementType_ _size_: an array
  * elementType can be the atomic types or record, currently multi-dimensional array is not supported
  * size can be specified as:
    * *x* _number_: constant length
    * *xv* _fieldName_: length is the value of the specified field
    * *xz*: zero-terminated string
    * *tpb* _number_: array has a guard byte with the value _number_ after it
    * *tps* _number_: array has a guard short with the value _number_ after it
    * *tpi* _number_: array has a guard int with the value _number_ after it

## Parsing
First compile the schema:
```q
schema:.binp.compileSchema schemaStr;
```
Then use the compiled schema on the data:
```q
.binp.parse[schema;0x0000;`mainType]
```

## Examples

An array of 4 ints:
```
schemaStr:"
    record simple
        field nums array int x 4
    end";
schema:.binp.compileSchema schemaStr;
.binp.parse[schema;0x01000000020000000300000004000000;`simple]
```
Returns: enlist[`nums]!enlist 1 2 3 4i

A string with a two-byte length prepended:
```
schemaStr:"
    record stringWithShortLen
        field length short
        field str array char xv length
    end";
schema:.binp.compileSchema schemaStr;
.binp.parse[schema;0x060048656c6c6f;`stringWithShortLen]
```
Returns: `length`str!(6h;"Hello")

See also examples.q.
