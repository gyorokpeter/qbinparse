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

inline K parseByte(char *&ptr) {
    K result = kg(*ptr);
    ptr += 1;
    return result;
}

inline K parseShort(char *&ptr) {
    K result = kh(*(uint16_t*)ptr);
    ptr += 2;
    return result;
}

inline K parseInt(char *&ptr) {
    K result = ki(*(uint32_t*)ptr);
    ptr += 4;
    return result;
}

inline K parseLong(char *&ptr) {
    K result = kj(*(uint64_t*)ptr);
    ptr += 8;
    return result;
}

inline K parseReal(char *&ptr) {
    K result = ke(*(float*)ptr);
    ptr += 4;
    return result;
}

inline K parseFloat(char *&ptr) {
    K result = kf(*(double*)ptr);
    ptr += 8;
    return result;
}

inline K parseChar(char *&ptr) {
    K result = kc(*ptr);
    ptr += 1;
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

inline K parseArray(K schema, char *&ptr, char *end, char *&recschema, K partialResult, int elementType) {
    if (ptr >= end) return ksym("endOfBuffer");
    char sizeMode = *recschema;
    uint32_t size = 0;
    uint32_t guard = 0;
    bool fixedSize = true;
    ++recschema;
    switch(sizeMode) {
    case 1:{
        char *eptr = ptr;
        while (eptr < end && *eptr != 0) ++eptr;
        ++eptr;
        if (eptr > end) size = 2100000000;
        size = eptr-ptr;
        break;
    };
    case 2:
    case 3:
    case 4:
        fixedSize = false;
        guard = *(uint32_t*)recschema;
        break;
    case 0:
        size = *(uint32_t*)recschema;
        break;
    case -4:
        size = kK(partialResult)[*(uint32_t*)recschema]->g;
        break;
    case -5:
        size = kK(partialResult)[*(uint32_t*)recschema]->h;
        break;
    case -6:
        size = kK(partialResult)[*(uint32_t*)recschema]->i;
        break;
    case -7:
        size = kK(partialResult)[*(uint32_t*)recschema]->j;
        break;
    }
    recschema += 4;
    if(fixedSize) {
        if(size >= 2100000000) {
            ptr = end;
            return ksym("tooLargeArray");
        }
        if (elementType > -20) {
            K result = ktn(-elementType,size);
            uint64_t fullsize = uint64_t(size)*getTypeSize(elementType);
            if(fullsize > 4200000000) {
                ptr = end;
                return ksym("tooLargeArray");
            }
            if(ptr+fullsize > end) return ksym("arrayRunsPastInput");
            memcpy(kG(result), ptr, fullsize);
            ptr += fullsize;
            return result;
        } else if (elementType <= -20) {
            uint64_t fullsize = uint64_t(size)*sizeof(void*);
            if(fullsize > 4200000000) {
                ptr = end;
                return ksym("tooLargeArray");
            }
            if(ptr+fullsize > end) {
                ptr = end;
                return ksym("arrayRunsPastInput");
            }
            K result = ktn(0,size);
            for (size_t i=0;i<size; ++i) {
                kK(result)[i] = parseRecord(schema, ptr, end, (-elementType)-20);
            }
            return result;
        }
    } else {    //not fixedSize
        if(elementType > -20) {
            K result = ktn(-elementType,0);
            int elementSize = getTypeSize(elementType);
            while(ptr < end) {
                bool cond = false;
                switch(sizeMode) {
                    case 2:
                        cond = *(uint8_t *)ptr == guard;
                        break;
                    case 3:
                        cond = *(uint16_t *)ptr == guard;
                        break;
                    case 4:
                        cond = *(uint32_t *)ptr == guard;
                        break;
                }
                if (cond) break;
                uint64_t atom;
                memcpy(&atom, ptr, elementSize);
                ptr += elementSize;
                ja(&result, &atom);
            }
            return result;
        } else {
            K result = ktn(0,0);
            while(ptr < end) {
                bool cond = false;
                switch(sizeMode) {
                    case 2:
                        cond = *(uint8_t *)ptr == guard;
                        break;
                    case 3:
                        cond = *(uint16_t *)ptr == guard;
                        break;
                    case 4:
                        cond = *(uint32_t *)ptr == guard;
                        break;
                }
                if (cond) break;
                K rec = parseRecord(schema, ptr, end, (-elementType)-20);
                jk(&result, rec);
            }
            return result;
        }
    }
    return ki(ni);
}

K parseCase(K schema, char *&ptr, char *end, char *&recschema, bool hasDefault,
    K partialResult
) {
    uint8_t caseFieldType = *recschema;
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
    case 4:
        caseFieldValue = kK(partialResult)[caseFieldIndex]->g;
        break;
    case 5:
        caseFieldValue = kK(partialResult)[caseFieldIndex]->h;
        break;
    case 6:
        caseFieldValue = kK(partialResult)[caseFieldIndex]->i;
        break;
    case 7:
        caseFieldValue = kK(partialResult)[caseFieldIndex]->j;
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

K parseExtType(K schema, char *&ptr, char *end, char *&recschema, K partialResult) {
    uint8_t extSubtype = *recschema;
    ++recschema;
    switch(extSubtype) {
    case 1:
        return parseCase(schema, ptr, end, recschema, false, partialResult);
    case 2:
        return parseCase(schema, ptr, end, recschema, true, partialResult);
    default:
        return ksym("invalidExtType");
    }
}

K parseRecord(K schema, char *&ptr, char *end, size_t schemaindex) {
    if (ptr >= end) return ksym("endOfBuffer");
    if (schemaindex >= kK(schema)[1]->n) {
        return ksym("invalidSchemaId");
    }
    K fieldLabels = kK(kK(schema)[1])[schemaindex];
    size_t fieldCount = fieldLabels->n;
    char *recschema = (char*)kC(kK(kK(schema)[2])[schemaindex]);
    K result = ktn(0, fieldCount);
    for (size_t i=0; i<fieldCount; ++i) {
        char inst = *recschema;
        ++recschema;
        if (inst == -128) {
            kK(result)[i] = parseExtType(schema, ptr, end, recschema, result);
        } else if (inst <= -20) {
            kK(result)[i] = parseRecord(schema, ptr, end, (-inst)-20);
        } else if (inst > 0) {
            kK(result)[i] = parseArray(schema, ptr, end, recschema, result, -inst);
        } else switch(inst) {
        case -4:
            kK(result)[i] = parseByte(ptr);
            break;
        case -5:
            kK(result)[i] = parseShort(ptr);
            break;
        case -6:
            kK(result)[i] = parseInt(ptr);
            break;
        case -7:
            kK(result)[i] = parseLong(ptr);
            break;
        case -8:
            kK(result)[i] = parseReal(ptr);
            break;
        case -9:
            kK(result)[i] = parseFloat(ptr);
            break;
        case -10:
            kK(result)[i] = parseChar(ptr);
            break;
        default:
            kK(result)[i] = ki(ni);
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
        if (kS(kK(schema)[0])[i] == mainType->s) {
            K result = parseRecord(schema, ptr, end, i);
            if (ptr < end) {
                kK(result)[0] = kdup(kK(result)[0]);
                S fillerKey = ssym("xxxRemainingData");
                K fillerValue = ktn(4, end-ptr);
                memcpy(kG(fillerValue), ptr, end-ptr);
                js(&kK(result)[0], fillerKey);
                jk(&kK(result)[1], fillerValue);
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