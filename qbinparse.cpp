#define KXVER 3
#include "k.h"
#include <stdint.h>
#include <string.h>

extern "C" {

K kerror(const char *err) {
    return krr(const_cast<S>(err));
}

K ksym(const char *s) {
    return ks(const_cast<S>(s));
}

S ssym(const char *s) {
    return ss(const_cast<S>(s));
}

K kdup(K k) {
    if (k->r == 0) return k;
    if (k->t != 11) return kerror("kdup: NYI type");
    K result = ktn(k->t, k->n);
    memcpy(kS(result), kS(k), sizeof(char*)*k->n);
    r0(k);
    return result;
}

inline G readByte(char *&ptr) {
    G result = *ptr;
    ptr += 1;
    return result;
}

inline H readShort(char *&ptr) {
    H result = *(uint16_t*)ptr;
    ptr += 2;
    return result;
}

inline H readBEShort(char *&ptr) {
    uint16_t num = *(uint16_t*)ptr;
    H result = ((num & 0xff00) >> 8) | (num << 8);
    ptr += 2;
    return result;
}

inline I readInt(char *&ptr) {
    I result = *(uint32_t*)ptr;
    ptr += 4;
    return result;
}

inline I readBEInt(char *&ptr) {
    uint32_t num = *(uint32_t*)ptr;
    I result = ((num & 0xff000000) >> 24) | ((num & 0x00ff0000) >> 8) | ((num & 0x0000ff00) << 8) | (num << 24);
    ptr += 4;
    return result;
}

inline J readLong(char *&ptr) {
    J result = *(uint64_t*)ptr;
    ptr += 8;
    return result;
}

inline E readReal(char *&ptr) {
    E result = *(float*)ptr;
    ptr += 4;
    return result;
}

inline F readFloat(char *&ptr) {
    F result = *(double*)ptr;
    ptr += 8;
    return result;
}

inline C readChar(char *&ptr) {
    C result = *ptr;
    ptr += 1;
    return result;
}

inline I readDotNetVarLengthInt(char *&ptr) {
    I result = 0;
    int bit = 0;
    uint8_t next;
    do {
        next = *ptr++;
        result |= (next & 0x7f) << bit;
        bit += 7;
    } while (next & 0x80);
    return result;
}

inline int getTypeSize(int typeNum) {
    switch(typeNum) {
        case -4: return 1;
        case -5: return 2;
        case -6: return 4;
        case -7: return 8;
        case -8: return 4;
        case -9: return 8;
        case -10: return 1;
    }
    return 0;
}

K parseRecord(K schema, char *&ptr, char *end, size_t schemaindex);

inline K tot(K records) {   //to table - q should expose this feature
    if(records->n == 0) return records;
    for (J i=0; i<records->n; ++i) {    //error messages may replace some records
        if (kK(records)[i]->t != 99) return records;
    }
    J columnCount = kK(kK(records)[0])[0]->n;
    if (columnCount == 0) return records;
    K columns = ktn(0, columnCount);
    K labels = r1(kK(kK(records)[0])[0]);
    for (J i=0; i<columns->n; ++i) {
        K col;
        J length = records->n;
        int recType = kK(kK(records)[0])[1]->t;
        if (recType != 0) { //all record fields are the same atomic type
            switch(recType) {
            case KG:
                col = ktn(KG, length);
                for (J j=0; j<length; ++j) kG(col)[j] = kG(kK(kK(records)[j])[1])[i];
                break;
            case KH:
                col = ktn(KH, length);
                for (J j=0; j<length; ++j) kH(col)[j] = kH(kK(kK(records)[j])[1])[i];
                break;
            case KI:
                col = ktn(KI, length);
                for (J j=0; j<length; ++j) kI(col)[j] = kI(kK(kK(records)[j])[1])[i];
                break;
            case KJ:
                col = ktn(KJ, length);
                for (J j=0; j<length; ++j) kJ(col)[j] = kJ(kK(kK(records)[j])[1])[i];
                break;
            case KE:
                col = ktn(KE, length);
                for (J j=0; j<length; ++j) kE(col)[j] = kE(kK(kK(records)[j])[1])[i];
                break;
            case KF:
                col = ktn(KF, length);
                for (J j=0; j<length; ++j) kF(col)[j] = kF(kK(kK(records)[j])[1])[i];
                break;
            case KC:
                col = ktn(KC, length);
                for (J j=0; j<length; ++j) kF(col)[j] = kC(kK(kK(records)[j])[1])[i];
                break;
            default:
                return kerror("unhandled atomic type in array!");
            }
        } else {    //heterogeneous records
            int colType = kK(kK(kK(records)[0])[1])[i]->t;
            switch(colType) {
            case -KG:
                col = ktn(KG, length);
                for (J j=0; j<length; ++j) kG(col)[j] = kK(kK(kK(records)[j])[1])[i]->g;
                break;
            case -KH:
                col = ktn(KH, length);
                for (J j=0; j<length; ++j) kH(col)[j] = kK(kK(kK(records)[j])[1])[i]->h;
                break;
            case -KI:
                col = ktn(KI, length);
                for (J j=0; j<length; ++j) kI(col)[j] = kK(kK(kK(records)[j])[1])[i]->i;
                break;
            case -KJ:
                col = ktn(KJ, length);
                for (J j=0; j<length; ++j) kJ(col)[j] = kK(kK(kK(records)[j])[1])[i]->j;
                break;
            case -KE:
                col = ktn(KE, length);
                for (J j=0; j<length; ++j) kE(col)[j] = kK(kK(kK(records)[j])[1])[i]->e;
                break;
            case -KF:
                col = ktn(KF, length);
                for (J j=0; j<length; ++j) kF(col)[j] = kK(kK(kK(records)[j])[1])[i]->f;
                break;
            case -KC:
                col = ktn(KC, length);
                for (J j=0; j<length; ++j) kC(col)[j] = kK(kK(kK(records)[j])[1])[i]->g;
                break;
            default:
                col = ktn(0, length);
                for (J j=0; j<length; ++j) kK(col)[j] = r1(kK(kK(kK(records)[j])[1])[i]);
            }
        }
        kK(columns)[i] = col;
    }
    r0(records);
    K result = xT(xD(labels, columns));
    return result;
}

K appendGen(K list, K value) {
    K result = ktn(0, list->n+1);
    for (size_t i=0; i<list->n; ++i) {
        switch(list->t) {
            case 0: kK(result)[i] = r1(kK(list)[i]); break;
            case KC: kK(result)[i] = kc(kC(list)[i]); break;
            case KG: kK(result)[i] = kg(kG(list)[i]); break;
            case KH: kK(result)[i] = kh(kH(list)[i]); break;
            case KI: kK(result)[i] = ki(kI(list)[i]); break;
            case KJ: kK(result)[i] = kj(kJ(list)[i]); break;
            case KE: kK(result)[i] = ke(kE(list)[i]); break;
            case KF: kK(result)[i] = kf(kF(list)[i]); break;
            default: kK(result)[i] = ki(ni);
        }
    }
    kK(result)[list->n] = value;
    r0(list);
    return result;
}

struct arraySizeResult {
    enum {ok, endOfBuffer, badSizeSpec} status = ok;
    uint32_t size = 0;
    uint32_t guard = 0;
    bool fixedSize = true;
    bool repeat = false;
    char sizeMode;
};

arraySizeResult readArraySize(char *&ptr, char *end, char *&recschema, K partialResult) {
    arraySizeResult result;
    if (ptr > end) {result.status = arraySizeResult::endOfBuffer; return result;}
    result.sizeMode = *recschema;
    ++recschema;
    switch(result.sizeMode) {
    case 1:{
        char *eptr = ptr;
        while (eptr < end && *eptr != 0) ++eptr;
        ++eptr;
        if (eptr > end) result.status = arraySizeResult::endOfBuffer;
        result.size = eptr-ptr;
        break;
    };
    case 2:
    case 3:
    case 4:
        result.fixedSize = false;
        result.guard = *(uint32_t*)recschema;
        break;
    case 5:
        result.repeat = true;
        result.fixedSize = false;
        break;
    case 0:
        result.size = *(uint32_t*)recschema;
        break;
    case -4:
        result.size = kK(partialResult)[*(uint32_t*)recschema]->g;
        break;
    case -5:
        result.size = kK(partialResult)[*(uint32_t*)recschema]->h;
        break;
    case -6:
        result.size = kK(partialResult)[*(uint32_t*)recschema]->i;
        break;
    case -7:
        result.size = kK(partialResult)[*(uint32_t*)recschema]->j;
        break;
    default:
        result.status = arraySizeResult::badSizeSpec;
        return result;
    }
    recschema += 4;
    return result;
}

inline K parseArray(K schema, char *&ptr, char *end, char *&recschema, K partialResult, int elementType) {
    arraySizeResult as = readArraySize(ptr, end, recschema, partialResult);
    if (as.status == arraySizeResult::endOfBuffer) return ksym("endOfBuffer");
    if (as.status == arraySizeResult::badSizeSpec) return ksym("invalidArraySizeSpecifier");
    if (as.fixedSize) {
        if(as.size >= 2100000000) {
            ptr = end;
            return ksym("tooLargeArray");
        }
        if (elementType > -20) {
            K result = ktn(-elementType, as.size);
            uint64_t fullsize = uint64_t(as.size)*getTypeSize(elementType);
            if(fullsize > 4200000000) {
                ptr = end;
                return ksym("tooLargeArray");
            }
            if(ptr+fullsize > end) return ksym("arrayRunsPastInput");
            memcpy(kG(result), ptr, fullsize);
            ptr += fullsize;
            return result;
        } else if (elementType <= -20) {
            uint64_t fullsize = uint64_t(as.size)*sizeof(void*);
            if(fullsize > 4200000000) {
                ptr = end;
                return ksym("tooLargeArray");
            }
            K result = ktn(0,as.size);
            for (size_t i=0;i<as.size; ++i) {
                kK(result)[i] = parseRecord(schema, ptr, end, (-elementType)-20);
            }
            K res = tot(result);
            return res;
        }
    } else {    //not fixedSize
        if(elementType > -20) { //elements are atoms
            K result = ktn(-elementType,0);
            int elementSize = getTypeSize(elementType);
            while(ptr < end) {
                bool cond = false;
                if (!as.repeat) {
                    switch(as.sizeMode) {
                        case 2:
                            cond = *(uint8_t *)ptr == as.guard;
                            break;
                        case 3:
                            cond = *(uint16_t *)ptr == as.guard;
                            break;
                        case 4:
                            cond = *(uint32_t *)ptr == as.guard;
                            break;
                    }
                }
                if (cond) break;
                uint64_t atom;
                memcpy(&atom, ptr, elementSize);
                ptr += elementSize;
                ja(&result, &atom);
            }
            return result;
        } else {    //elements are records
            K result = ktn(0,0);
            while(ptr < end) {
                bool cond = false;
                if (!as.repeat) {
                    switch(as.sizeMode) {
                        case 2:
                            cond = *(uint8_t *)ptr == as.guard;
                            break;
                        case 3:
                            cond = *(uint16_t *)ptr == as.guard;
                            break;
                        case 4:
                            cond = *(uint32_t *)ptr == as.guard;
                            break;
                    }
                }
                if (cond) break;
                K rec = parseRecord(schema, ptr, end, (-elementType)-20);
                jk(&result, rec);
            }
            return tot(result);
        }
    }
    return ki(ni);
}

K parseCase(K schema, char *&ptr, char *end, char *&recschema, bool hasDefault,
    K partialResult
) {
    int8_t caseFieldType = *recschema;
    ++recschema;
    uint32_t caseFieldIndex = *(uint32_t*)recschema;
    recschema += 4;
    uint8_t defaultRec = 0;
    if(hasDefault) {
        defaultRec = *recschema;
        ++recschema;
    }
    uint32_t caseFieldValue = 0;
    switch(caseFieldType) {
    case -4:
        caseFieldValue = kK(partialResult)[caseFieldIndex]->g;
        break;
    case -5:
        caseFieldValue = kK(partialResult)[caseFieldIndex]->h;
        break;
    case -6:
        caseFieldValue = kK(partialResult)[caseFieldIndex]->i;
        break;
    case -7:
        caseFieldValue = kK(partialResult)[caseFieldIndex]->j;
        break;
    case 10:
        caseFieldValue = *(uint32_t*)kG(kK(partialResult)[caseFieldIndex]);
        break;
    default:
        return ksym("invalidCaseFieldType");
    }
    uint32_t caseCount = *(uint32_t*)recschema;
    recschema += 4;
    bool found = false;
    uint8_t caseRec = 0;
    for (uint32_t i=0; i<caseCount; ++i) {
        found = (*(uint32_t*)recschema) == caseFieldValue;
        if(found) {
            caseRec = *(recschema+4);
            recschema += 5*(caseCount-i);
            break;
        }
        recschema += 5;
    }
    if (!found) {
        if(!hasDefault)
            return ksym("noCaseMatch");
        caseRec = defaultRec;
    }
    return parseRecord(schema, ptr, end, caseRec);
}

K parseField(K schema, char *&ptr, char *end, char *&recschema, K partialResult);

K parseInterpretedArray(K schema, char *&ptr, char *end, char *&recschema, K partialResult) {
    arraySizeResult as = readArraySize(ptr, end, recschema, partialResult);
    if (as.status == arraySizeResult::badSizeSpec) return ksym("invalidArraySizeSpecifier");
    char *tmpPtr = ptr;
    char *tmpEnd = ptr+as.size;
    if (as.status == arraySizeResult::endOfBuffer) tmpEnd = ptr;
    K result = parseField(schema, tmpPtr, tmpEnd, recschema, partialResult);
    ptr = tmpEnd;
    return result;
}

K parseExtType(K schema, char *&ptr, char *end, char *&recschema, K partialResult) {
    uint8_t extSubtype = *recschema;
    ++recschema;
    switch(extSubtype) {
    case 1:
        return parseCase(schema, ptr, end, recschema, false, partialResult);
    case 2:
        return parseCase(schema, ptr, end, recschema, true, partialResult);
    case 3:
        return kh(readBEShort(ptr));
    case 4:
        return ki(readBEInt(ptr));
    case 5:
        return parseInterpretedArray(schema, ptr, end, recschema, partialResult);
    case 6:
        return ki(readDotNetVarLengthInt(ptr));
    default:
        return ksym("invalidExtType");
    }
}

K parseField(K schema, char *&ptr, char *end, char *&recschema, K partialResult) {
    char fieldType = *recschema;
    ++recschema;
    if (fieldType == -128) {
        return parseExtType(schema, ptr, end, recschema, partialResult);
    } else if (fieldType <= -20) {
        return parseRecord(schema, ptr, end, (-fieldType)-20);
    } else if (fieldType > 0) {
        return parseArray(schema, ptr, end, recschema, partialResult, -fieldType);
    } else switch(fieldType) {
    case -4:
        return kg(readByte(ptr));
        break;
    case -5:
        return kh(readShort(ptr));
        break;
    case -6:
        return ki(readInt(ptr));
        break;
    case -7:
        return kj(readLong(ptr));
        break;
    case -8:
        return ke(readReal(ptr));
        break;
    case -9:
        return kf(readFloat(ptr));
        break;
    case -10:
        return kc(readChar(ptr));
        break;
    default:
        return ki(ni);
    }
}

K parseRecord(K schema, char *&ptr, char *end, size_t schemaindex) {
    if (schemaindex >= kK(schema)[1]->n) {
        return ksym("invalidSchemaId");
    }
    K fieldLabels = kK(kK(schema)[1])[schemaindex];
    size_t fieldCount = fieldLabels->n;
    if (fieldCount > 0 && ptr >= end) {
        return ksym("endOfBuffer");
    }
    char *recschema = (char*)kC(kK(kK(schema)[2])[schemaindex]);
    int resultType = 0; //create a simple list if all fields in the record are atomic types
    char *schemaScan = recschema;
    for (size_t i=0; i<fieldCount; ++i) {
        int8_t nextType = *schemaScan;
        ++schemaScan;
        if (nextType==-128) {
            int8_t ext = *schemaScan;
            ++schemaScan;
            switch(ext) {
                case 3: nextType=-5; break;
                case 4: nextType=-6; break;
                case 6: nextType=-6; break;
            }
        }
        if (nextType>=0 || nextType<=-20) { resultType = 0; break; }
        if (i > 0) {
            if (resultType != -nextType) { resultType = 0; break; }
        } else
            resultType = -nextType;
    }
    K result = ktn(resultType, fieldCount);
    for (size_t i=0; i<fieldCount; ++i) {
        if (resultType == 0) {
            kK(result)[i] = parseField(schema, ptr, end, recschema, result);
        } else {
            char fieldType = *recschema;
            ++recschema;
            switch(fieldType) {
            case -4:
                kG(result)[i] = readByte(ptr);
                break;
            case -5:
                kH(result)[i] = readShort(ptr);
                break;
            case -6:
                kI(result)[i] = readInt(ptr);
                break;
            case -7:
                kJ(result)[i] = readLong(ptr);
                break;
            case -8:
                kE(result)[i] = readReal(ptr);
                break;
            case -9:
                kF(result)[i] = readFloat(ptr);
                break;
            case -10:
                kC(result)[i] = readChar(ptr);
                break;
            case -128:
                char ext = *recschema;
                ++recschema;
                switch(ext) {
                case 3:
                    kH(result)[i] = readBEShort(ptr);
                    break;
                case 4:
                    kI(result)[i] = readBEInt(ptr);
                    break;
                case 6:
                    kI(result)[i] = readDotNetVarLengthInt(ptr);
                    break;
                }
                break;
            }
        }
    }
    return xD(r1(fieldLabels),result);
}

K k_binparse_parse(K schema, K input, K mainType) {
    if (schema->t != 0) { return kerror("parse: schema must be general list"); }
    if (input->t != 4) { return kerror("parse: data must be bytelist"); }
    if (mainType->t != -11) { return kerror("parse: main type must be a symbol"); }
    char *ptr = (char*)kG(input);
    char *end = ptr+input->n;
    for (size_t i=0; i<kK(schema)[0]->n; ++i) {
        if (kS(kK(schema)[0])[i] == ssym(mainType->s)) {
            K result = parseRecord(schema, ptr, end, i);
            if (ptr < end) {
                kK(result)[0] = kdup(kK(result)[0]);
                S fillerKey = ssym("xxxRemainingData");
                K fillerValue = ktn(4, end-ptr);
                memcpy(kG(fillerValue), ptr, end-ptr);
                js(&kK(result)[0], fillerKey);
                kK(result)[1] = appendGen(kK(result)[1], fillerValue);
            }
            return result;
        }
    }
    return kerror("main type not found in schema");
}

K k_binparse_parseRepeat(K schema, K input, K mainType) {
    if (input->t != 4) { return kerror("parse: data must be bytelist"); }
    if (mainType->t != -11) { return kerror("parse: main type must be a symbol"); }
    char *ptr = (char*)kG(input);
    char *end = ptr+input->n;
    for (size_t i=0; i<kK(schema)[0]->n; ++i) {
        if (kS(kK(schema)[0])[i] == mainType->s) {
            K result = ktn(0,0);
            while (ptr < end)
                jk(&result, parseRecord(schema, ptr, end, i));
            return result;
        }
    }
    return kerror("main type not found in schema");
}

}
/*
lib:`$":D:/Projects/c++/qutils/qbinparse";
.binp.parse:lib 2:(`k_binparse_parse;1)
*/