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

K parseRecord(K schema, char *&ptr, size_t schemaindex);

inline K parseArray(K schema, char *&ptr, char *&recschema, K partialResult, int elementType) {
    char sizeMode = *recschema;
    uint32_t size = 0;
    ++recschema;
    switch(sizeMode) {
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
    if (elementType > -20) {
        K result = ktn(-elementType,size);
        size *= getTypeSize(elementType);
        memcpy(kG(result), ptr, size);
        ptr += size;
        return result;
    } else if (elementType <= -20) {
        K result = ktn(0,size);
        for (size_t i=0;i<size; ++i) {
            kK(result)[i] = parseRecord(schema, ptr, (-elementType)-20);
        }
        return result;
    }
    return ki(ni);
}

K parseRecord(K schema, char *&ptr, size_t schemaindex) {
    K fieldLabels = kK(kK(schema)[1])[schemaindex];
    size_t fieldCount = fieldLabels->n;
    char *recschema = (char*)kC(kK(kK(schema)[2])[schemaindex]);
    K result = ktn(0, fieldCount);
    for (size_t i=0; i<fieldCount; ++i) {
        char inst = *recschema;
        ++recschema;
        if (inst <= -20) {
            kK(result)[i] = parseRecord(schema, ptr, (-inst)-20);
        } else if (inst > 0) {
            kK(result)[i] = parseArray(schema, ptr, recschema, result, -inst);
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
    if (input->t != 4) { return kerror("parse: data must be bytelist"); }
    if (mainType->t != -11) { return kerror("parse: main type must be a symbol"); }
    char *ptr = (char*)kG(input);
    char *end = ptr+input->n;
    for (size_t i=0; i<kK(schema)[0]->n; ++i) {
        if (kS(kK(schema)[0])[i] == mainType->s) {
            K result = parseRecord(schema, ptr, i);
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
                jk(&result, parseRecord(schema, ptr, i));
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