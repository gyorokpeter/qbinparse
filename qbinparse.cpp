#define KXVER 3
#include "k.h"
#undef kC
#include <stdint.h>
#include <string.h>
#include <stdexcept>
#include <vector>
#include <map>

//Patches for k.h to work under C++
inline char *kC(K k) {
    return (char*)k->G0;
}

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

//Parsing

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

inline uint16_t flipEndian(uint16_t val) {
    return ((val & 0xff00) >> 8) | (val << 8);
}

inline H readBEShort(char *&ptr) {
    uint16_t num = *(uint16_t*)ptr;
    H result = flipEndian(num);
    ptr += 2;
    return result;
}

inline I readInt(char *&ptr) {
    I result = *(uint32_t*)ptr;
    ptr += 4;
    return result;
}

inline uint32_t flipEndian(uint32_t val) {
    return ((val & 0xff000000) >> 24) | ((val & 0x00ff0000) >> 8) | ((val & 0x0000ff00) << 8) | (val << 24);
}

inline I readBEInt(char *&ptr) {
    uint32_t num = *(uint32_t*)ptr;
    I result = flipEndian(num);
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
    uint32_t sizeFieldIndex = 0;
    bool constantSize = false;   //xv
    bool fixedSize = true;  //size known in advance, e.g. xv OR size from other field
    bool repeat = false;
    char sizeMode;
};

arraySizeResult readArraySizeFromSchema(char *&recschema) {
    arraySizeResult result;
    result.sizeMode = *recschema;
    ++recschema;
    switch(result.sizeMode) {
    case 0:
        result.constantSize = true;
        result.size = *(uint32_t*)recschema;
        break;
    case 1: //"xz" - zero terminated
        break;
    case 2: //"tpb" - byte guard
    case 3: //"tps" - short guard
    case 4: //"tpi" - int guard
        result.fixedSize = false;
        result.guard = *(uint32_t*)recschema;
        break;
    case 5: //"repeat"
        result.repeat = true;
        result.fixedSize = false;
        break;
    case -4:    //"xv" - use byte field
    case -5:    //"xv" - use short field
    case -6:    //"xv" - use int field
    case -7:    //"xv" - use long field
        result.sizeFieldIndex = *(uint32_t*)recschema;
        break;
    default:
        result.status = arraySizeResult::badSizeSpec;
        return result;
    }
    recschema += 4;
    return result;
}

arraySizeResult readArraySizeFromSchemaWithPartial(char *&recschema, K partialResult) {
    arraySizeResult result = readArraySizeFromSchema(recschema);
    switch(result.sizeMode) {
    case -4:    //"xv" - use byte field
        result.size = kK(partialResult)[result.sizeFieldIndex]->g;
        break;
    case -5:    //"xv" - use short field
        result.size = kK(partialResult)[result.sizeFieldIndex]->h;
        break;
    case -6:    //"xv" - use int field
        result.size = kK(partialResult)[result.sizeFieldIndex]->i;
        break;
    case -7:    //"xv" - use long field
        result.size = kK(partialResult)[result.sizeFieldIndex]->j;
        break;
    default:
        break;
    }
    return result;
}

arraySizeResult readArraySize(char *&ptr, char *end, char *&recschema, K partialResult) {
    arraySizeResult result = readArraySizeFromSchemaWithPartial(recschema, partialResult);
    if (ptr > end) {result.status = arraySizeResult::endOfBuffer; return result;}
    switch(result.sizeMode) {
    case 1:{    //"xz" - zero terminated
        char *eptr = ptr;
        while (eptr < end && *eptr != 0) ++eptr;
        ++eptr;
        if (eptr > end) result.status = arraySizeResult::endOfBuffer;
        result.size = eptr-ptr;
        break;
    };
    //size already read by readArraySizeFromSchema
    case 2:
    case 3:
    case 4:
        break;
    case 5:
        break;
    case 0:
        break;
    case -4:
    case -5:
    case -6:
    case -7:
        break;
    default:
        result.status = arraySizeResult::badSizeSpec;
    }
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

K parseField(K schema, char *&ptr, char *start, char *&end, char *&recschema, K partialResult);

K parseInterpretedArray(K schema, char *&ptr, char *end, char *&recschema, K partialResult) {
    arraySizeResult as = readArraySize(ptr, end, recschema, partialResult);
    if (as.status == arraySizeResult::badSizeSpec) return ksym("invalidArraySizeSpecifier");
    char *tmpStart = ptr;
    char *tmpPtr = ptr;
    char *tmpEnd = ptr+as.size;
    if (as.status == arraySizeResult::endOfBuffer) tmpEnd = ptr;
    K result = parseField(schema, tmpPtr, tmpStart, tmpEnd, recschema, partialResult);
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
    case 7:
        return ki((uint16_t)readShort(ptr));
    case 8:
        return kj((uint32_t)readInt(ptr));
    case 9:
        return ki((uint16_t)readBEShort(ptr));
    case 10:
        return kj((uint32_t)readBEInt(ptr));
    default:
        return ksym("invalidExtType");
    }
}

K parseField(K schema, char *&ptr, char *start, char *&end, char *&recschema, K partialResult) {
    char fieldType = *recschema;
    ++recschema;
    bool isRecSize = false;
    while (fieldType == 127) {
        char op = *recschema;
        ++recschema;
        if (op == 1) {
            isRecSize = true;
        }
        fieldType = *recschema;
        ++recschema;
    }
    if (fieldType == -128) {
        return parseExtType(schema, ptr, end, recschema, partialResult);
    } else if (fieldType <= -20) {
        return parseRecord(schema, ptr, end, (-fieldType)-20);
    } else if (fieldType > 0) {
        return parseArray(schema, ptr, end, recschema, partialResult, -fieldType);
    } else switch(fieldType) {
    case -4:{
        G val = readByte(ptr);
        if (isRecSize) end = start+val;
        return kg(val);
        break;
    }
    case -5:{
        H val = readShort(ptr);
        if (isRecSize) end = start+val;
        return kh(val);
        break;
    }
    case -6:{
        I val = readInt(ptr);
        if (isRecSize) end = start+val;
        return ki(val);
        break;
    }
    case -7:{
        J val = readLong(ptr);
        if (isRecSize) end = start+val;
        return kj(val);
        break;
    }
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
        if (nextType == 127) {
            schemaScan+=2;
        }
        if (nextType>=0 || nextType<=-20) { resultType = 0; break; }
        if (i > 0) {
            if (resultType != -nextType) { resultType = 0; break; }
        } else
            resultType = -nextType;
    }
    char *start = ptr;
    K result = ktn(resultType, fieldCount);
    for (size_t i=0; i<fieldCount; ++i) {
        if (resultType == 0) {
            kK(result)[i] = parseField(schema, ptr, start, end, recschema, result);
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

// Serialization

class Buffer {
    uint8_t *data_;
    size_t pos_;
    size_t limit_;
    void ensureSpace(size_t size) {
        while (pos_+size>limit_) grow();
    }
public:
    Buffer() : data_(new uint8_t[1000]), pos_(0), limit_(1000) {}
    Buffer(const Buffer &other) = delete;
    ~Buffer() { delete[] data_; }
    Buffer &operator=(const Buffer &other) = delete;
    inline void grow() {
        uint8_t *ndata = new uint8_t[10*limit_];
        memcpy(ndata, data_, pos_);
        delete[] data_;
        data_ = ndata;
        limit_ = 10*limit_;
    }
    Buffer &write(const Buffer &other) {
        size_t size = other.size();
        ensureSpace(size);
        memcpy(data_+pos_, other.data(), size);
        pos_+=size;
        return *this;
    }
    Buffer &write(const uint8_t *from, size_t size) {
        ensureSpace(size);
        memcpy(data_+pos_, from, size);
        pos_+=size;
        return *this;
    }
    Buffer &writeFiller(size_t size) {
        ensureSpace(size);
        memset(data_+pos_, 0, size);
        pos_+=size;
        return *this;
    }
    template<typename T>
    Buffer &write(T from) {
        ensureSpace(sizeof(T));
        *(T*)(data_+pos_) = from;
        pos_+=sizeof(T);
        return *this;
    }
    size_t size() const { return pos_; }
    const uint8_t *data() const { return data_; }
};

template<typename Ct>
Ct getElemFromK(K k, size_t index) {
    switch(k->t) {
        case 0: {
            K elem = kK(k)[index];
            switch(elem->t) {
            case -4:
                return elem->g;
            case -5:
                return elem->h;
            case -6:
                return elem->i;
            case -7:
                return elem->j;
            case -8:
                return elem->e;
            case -9:
                return elem->f;
            case -10:
                return elem->g;
            default:
                throw std::runtime_error("getElemFromK can't read type "+std::to_string(elem->t)+" from general list");
            }
        }
        case 4:
            return kG(k)[index];
        case 5:
            return kH(k)[index];
        case 6:
            return kI(k)[index];
        case 7:
            return kJ(k)[index];
        case 8:
            return kE(k)[index];
        case 9:
            return kF(k)[index];
        case 10:
            return kC(k)[index];
        default:
            throw std::runtime_error("getElemFromK can't read type "+std::to_string(k->t)+" from simple list");
    }
}

template<typename Ct, int Qt>
Buffer &unparseArray(Buffer &buf, char *&recschema, K inFieldElem) {
    arraySizeResult arrsz = readArraySizeFromSchema(recschema);
    if (arrsz.constantSize && arrsz.size != inFieldElem->n) {
        throw std::runtime_error("expected "+std::to_string(arrsz.size)+" items, got "+std::to_string(inFieldElem->n));
    }
    if (inFieldElem->t == Qt) { //no conversion required
        buf.write(kG(inFieldElem), sizeof(Ct)*inFieldElem->n);
    } else {
        for (size_t i=0; i<inFieldElem->n; ++i) {
            Ct val = getElemFromK<Ct>(inFieldElem, i);
            buf.write((uint8_t*)&val, sizeof(Ct));
        }
    }
    return buf;
}

template<typename Ct>
Buffer &unparseArrayElement(Buffer &buf, K arr, J index) {
    if (index > arr->n) {
        throw std::runtime_error("array index out of bounds ("+std::to_string(index)+">"+std::to_string(arr->n)+")");
    }
    Ct val = getElemFromK<Ct>(arr, index);
    buf.write((uint8_t*)&val, sizeof(Ct));
    return buf;
}

Buffer &unparseRecord(Buffer &buf, K schema, K input, size_t schemaindex);

struct caseDesc {
    //int8_t caseFieldType;
    uint32_t caseFieldIndex;
    bool hasDefault;
    uint8_t defaultRec;
    std::map<uint32_t, uint8_t> caseToRec;
};

caseDesc readCaseDesc(char *&recschema, bool hasDefault) {
    caseDesc result;
    //result.caseFieldType = *recschema;
    ++recschema;
    result.caseFieldIndex = *(uint32_t*)recschema;
    recschema += 4;
    result.defaultRec = 0;
    result.hasDefault = hasDefault;
    if(hasDefault) {
        result.defaultRec = *recschema;
        ++recschema;
    }
    uint32_t caseCount = *(uint32_t*)recschema;
    recschema += 4;
    for (uint32_t i=0; i<caseCount; ++i) {
        uint32_t caseValue = *(uint32_t*)recschema;
        uint8_t caseRec = *(recschema+4);
        result.caseToRec[caseValue] = caseRec;
        recschema += 5;
    }
    return result;
}

std::vector<int> matchFieldPos(K fieldLabels, K dict) {
    K keys = kK(dict)[0];
    if (keys->t != KS)
        throw std::runtime_error("unexpected type in columns, type "+std::to_string(keys->t));
    size_t fieldCount = fieldLabels->n;
    std::vector<int> fieldPos(fieldCount);
    for (size_t i=0; i<fieldCount; ++i) {
        S fieldName = kS(fieldLabels)[i];
        bool found = false;
        for (size_t j=0; j<keys->n; ++j) {
            if (fieldName == kS(keys)[j]) {
                fieldPos[i] = j;
                found = true;
                break;
            }
        }
        if (!found) {
            throw std::runtime_error("missing field: "+std::string(fieldName));
        }
    }
    return fieldPos;
}

Buffer &unparseRecordFieldsFromTable(Buffer &buf, K schema, size_t schemaindex, K values, const std::vector<int> &fieldPos, J i) {
    K fieldLabels = kK(kK(schema)[1])[schemaindex];
    size_t fieldCount = fieldLabels->n;
    char *recschema = (char*)kC(kK(kK(schema)[2])[schemaindex]);
    for (size_t j=0; j<fieldCount; ++j) {
        K fieldArr = kK(values)[fieldPos[j]];
        K inFieldElem = 0;
        char fieldType = *recschema;
        ++recschema;
        if ((fieldType<=-20 && fieldType > -128) || fieldType>0) {
            if (fieldArr->t != 0) {
                throw std::runtime_error("can't populate array field - record value is a simple list");
            }
            inFieldElem = kK(fieldArr)[i];
        }
        if (fieldType<=-20 && fieldType > -128) {
            unparseRecord(buf, schema, inFieldElem, (-fieldType)-20);
        } else try {
            switch(fieldType) {
            case -4:
                unparseArrayElement<uint8_t>(buf,fieldArr,i);
                break;
            case -5:
                unparseArrayElement<int16_t>(buf,fieldArr,i);
                break;
            case -6:
                unparseArrayElement<int32_t>(buf,fieldArr,i);
                break;
            case -7:
                unparseArrayElement<int64_t>(buf,fieldArr,i);
                break;
            case -8:
                unparseArrayElement<float>(buf,fieldArr,i);
                break;
            case -9:
                unparseArrayElement<double>(buf,fieldArr,i);
                break;
            case -10:
                unparseArrayElement<char>(buf,fieldArr,i);
                break;
            case 4:
                unparseArray<uint8_t,4>(buf, recschema, inFieldElem);
                break;
            case 10:
                unparseArray<char,4>(buf, recschema, inFieldElem);
                break;
            case -128:{
                char ext = *recschema;
                ++recschema;
                switch(ext) {
                case 1:
                case 2:{
                    caseDesc cd = readCaseDesc(recschema, ext==2);
                    uint32_t caseFieldVal = getElemFromK<uint32_t>(kK(values)[fieldPos[cd.caseFieldIndex]], i);
                    size_t innerSchemaIndex = 0;
                    auto cur = cd.caseToRec.find(caseFieldVal);
                    if (cur == cd.caseToRec.end()) {
                        if (cd.hasDefault)
                            innerSchemaIndex = cd.defaultRec;
                        else
                            throw std::runtime_error("invalid case value "+std::to_string(caseFieldVal));
                    } else
                        innerSchemaIndex = cur->second;
                    unparseRecord(buf, schema, kK(fieldArr)[i], innerSchemaIndex);
                    break;
                }
                case 7:
                    unparseArrayElement<int16_t>(buf,fieldArr,i);
                    break;
                case 8:
                    unparseArrayElement<int32_t>(buf,fieldArr,i);
                    break;
                default:
                    throw std::runtime_error("unparseArrayOfRecords NYI extended field type "+std::to_string(ext));
                }
                break;
            }
            default:
                throw std::runtime_error("unparseArrayOfRecords NYI field type "+std::to_string(fieldType));
            }
        } catch(const std::exception &e) {
            throw std::runtime_error("["+std::to_string(i)+"]"+"."+kS(fieldLabels)[j]+": "+e.what());
        }
    }
    return buf;
}


Buffer &unparseRecordFromTableRow(Buffer &buf, K schema, K table, J index, size_t schemaindex) {
    K fieldLabels = kK(kK(schema)[1])[schemaindex];
    K dict = table->k;
    if (dict->t != XD)
        throw std::runtime_error("unparseRecordFromTableRow unexpected value in table, type "+std::to_string(dict->t));
    K values = kK(dict)[1];
    if (values->t != 0)
        throw std::runtime_error("unparseRecordFromTableRow unexpected type in values, type "+std::to_string(values->t));

    std::vector<int> fieldPos = matchFieldPos(fieldLabels, dict);
    unparseRecordFieldsFromTable(buf, schema, schemaindex, values, fieldPos, index);
    return buf;
}

Buffer &unparseArrayOfRecords(Buffer &buf, char *&recschema, K schema, K inFieldElem, size_t schemaindex) {
    arraySizeResult arrsz = readArraySizeFromSchema(recschema);
    K fieldLabels = kK(kK(schema)[1])[schemaindex];
    if (inFieldElem->t == 0) {  //non-collapsed records
        J elemCount = inFieldElem->n;
        if (arrsz.constantSize && arrsz.size != elemCount) {
            throw std::runtime_error("expected "+std::to_string(arrsz.size)+" items, got "+std::to_string(elemCount));
        }
        for (size_t i=0; i<elemCount; ++i) {
            unparseRecord(buf, schema, kK(inFieldElem)[i], schemaindex);
        }
    } else if (inFieldElem->t == XT) {  //collapsed records
        K dict = inFieldElem->k;
        if (dict->t != XD)
            throw std::runtime_error("unparseArrayOfRecords unexpected value in table, type "+std::to_string(dict->t));
        K values = kK(dict)[1];
        if (values->t != 0)
            throw std::runtime_error("unparseArrayOfRecords unexpected type in values, type "+std::to_string(values->t));

        K firstField = kK(values)[0];
        J elemCount = firstField->n;

        if (arrsz.constantSize && arrsz.size != firstField->n) {
            throw std::runtime_error("expected "+std::to_string(arrsz.size)+" items, got "+std::to_string(firstField->n));
        }
        std::vector<int> fieldPos = matchFieldPos(fieldLabels, dict);
        for (size_t i=0; i<elemCount; ++i) {
            unparseRecordFieldsFromTable(buf, schema, schemaindex, values, fieldPos, i);
        }
    } else {
        throw std::runtime_error("unparseArrayOfRecords can't handle type "+std::to_string(inFieldElem->t));
    }
    return buf;
}

Buffer &unparseRecord(Buffer &buf, K schema, K input, size_t schemaindex) {
    if (input->t != 99) {
        throw std::runtime_error(std::string()+kS(kK(schema)[0])[schemaindex]+": input is not a dictionary (found type "+std::to_string(input->t)+")");
    }
    K inputVal = kK(input)[1];
    K fieldLabels = kK(kK(schema)[1])[schemaindex];
    K inFieldElem = 0;
    size_t fieldCount = fieldLabels->n;
    char *recschema = (char*)kC(kK(kK(schema)[2])[schemaindex]);
    std::vector<int> fieldPos = matchFieldPos(fieldLabels, input);
    for (size_t i=0; i<fieldCount; ++i) {
        size_t j = fieldPos[i];
        S fieldName = kS(fieldLabels)[i];
        char fieldType = *recschema;
        ++recschema;
        while(fieldType == 127) {
            ++recschema;
            fieldType = *recschema;
            ++recschema;
        }
        char fieldExtType = 0;
        if (fieldType == -128) {
            fieldExtType = *recschema;
            ++recschema;
        }
        try {
            if (fieldType>0 || (fieldType == -128 && (fieldExtType == 1 || fieldExtType == 2 || fieldExtType == 5))) {
                if (inputVal->t != 0) {
                    throw std::runtime_error("can't populate array field - record value is a simple list");
                }
                inFieldElem = kK(inputVal)[j];
            }
            if (fieldType>=20 && fieldType<127) {
                unparseArrayOfRecords(buf, recschema, schema, inFieldElem, fieldType-20);
            } else if (fieldType<=-20 && fieldType>-128) {
                switch(inputVal->t) {
                case 0:
                    unparseRecord(buf, schema, kK(inputVal)[j], (-fieldType)-20);
                    break;
                case 98:
                    unparseRecordFromTableRow(buf, schema, inputVal, j, (-fieldType)-20);
                    break;
                default:
                    throw std::runtime_error("can't parse record from type "+std::to_string(inputVal->t));
                }
            } else switch(fieldType) {
            case -4:{
                unparseArrayElement<uint8_t>(buf, inputVal, j);
                break;
            }
            case -5:{
                unparseArrayElement<int16_t>(buf, inputVal, j);
                break;
            }
            case -6:{
                unparseArrayElement<int32_t>(buf, inputVal, j);
                break;
            }
            case -7:{
                unparseArrayElement<int64_t>(buf, inputVal, j);
                break;
            }
            case -8:{
                unparseArrayElement<float>(buf, inputVal, j);
                break;
            }
            case -9:{
                unparseArrayElement<double>(buf, inputVal, j);
                break;
            }
            case -10:{
                unparseArrayElement<char>(buf, inputVal, j);
                break;
            }
            case 4:{
                unparseArray<uint8_t,4>(buf, recschema, inFieldElem);
                break;
            }
            case 5:{
                unparseArray<int16_t,5>(buf, recschema, inFieldElem);
                break;
            }
            case 6:{
                unparseArray<int32_t,6>(buf, recschema, inFieldElem);
                break;
            }
            case 7:{
                unparseArray<int64_t,7>(buf, recschema, inFieldElem);
                break;
            }
            case 8:{
                unparseArray<float,8>(buf, recschema, inFieldElem);
                break;
            }
            case 9:{
                unparseArray<double,9>(buf, recschema, inFieldElem);
                break;
            }
            case 10:{
                unparseArray<char,10>(buf, recschema, inFieldElem);
                break;
            }
            case -128:{
                switch(fieldExtType) {
                case 1:
                case 2:{
                    caseDesc cd = readCaseDesc(recschema, fieldExtType==2);
                    uint32_t caseFieldVal = getElemFromK<int32_t>(inputVal, fieldPos[cd.caseFieldIndex]);
                    size_t innerSchemaIndex = 0;
                    auto cur = cd.caseToRec.find(caseFieldVal);
                    if (cur == cd.caseToRec.end()) {
                        if (cd.hasDefault)
                            innerSchemaIndex = cd.defaultRec;
                        else
                            throw std::runtime_error("invalid case value "+std::to_string(caseFieldVal));
                    } else
                        innerSchemaIndex = cur->second;
                    unparseRecord(buf, schema, inFieldElem, innerSchemaIndex);
                    break;
                }
                case 3:{
                    uint16_t out = flipEndian(getElemFromK<uint16_t>(inputVal, j));
                    buf.write(out);
                    break;
                }
                case 4:{
                    uint32_t out = flipEndian(getElemFromK<uint32_t>(inputVal, j));
                    buf.write(out);
                    break;
                }
                case 5:{
                    arraySizeResult arrsz = readArraySizeFromSchemaWithPartial(recschema, inputVal);
                    Buffer tmpbuf;
                    char innerFieldType = *recschema;
                    ++recschema;
                    //char innerFieldExtType = 0;
                    if (innerFieldType == -128) {
                        //innerFieldExtType = *recschema;
                        ++recschema;
                    }
                    if (innerFieldType>=20) {
                        unparseArrayOfRecords(tmpbuf, recschema, schema, inFieldElem, innerFieldType-20);
                    } else switch(innerFieldType) {
                    default:
                        throw std::runtime_error("unparseRecord NYI field type in parsedArray: "+std::to_string(innerFieldType));
                    }
                    size_t innerSize = tmpbuf.size();
                    if (arrsz.size<innerSize) {
                        throw std::runtime_error("can't fit parsed data: max("+std::to_string(arrsz.size)+")<actual("+std::to_string(innerSize)+")");
                    } else if (arrsz.size>innerSize) {
                        tmpbuf.writeFiller(arrsz.size-innerSize);
                    }
                    buf.write(tmpbuf);
                    break;
                }
                case 6:{
                    uint32_t val = getElemFromK<uint32_t>(inputVal, j);
                    while (val>=0x80) {
                        uint8_t b = (val & 0x7f) | 0x80;
                        buf.write(b);
                        val >>= 7;
                    }
                    buf.write((uint8_t)val);
                    break;
                }
                case 7:{
                    unparseArrayElement<uint16_t>(buf, inputVal, j);
                    break;
                }
                case 8:{
                    unparseArrayElement<uint32_t>(buf, inputVal, j);
                    break;
                }
                case 9:{
                    uint16_t out = flipEndian(getElemFromK<uint16_t>(inputVal, j));
                    buf.write(out);
                    break;
                }
                case 10:{
                    uint32_t out = flipEndian(getElemFromK<uint32_t>(inputVal, j));
                    buf.write(out);
                    break;
                }
                default:
                    throw std::runtime_error("unparseRecord NYI extended field type "+std::to_string(fieldExtType));
                }
                break;
            }
            default:
                throw std::runtime_error("unparseRecord NYI field type "+std::to_string(fieldType));
            }
        } catch(const std::exception &e) {
            throw std::runtime_error(std::string(kS(kK(schema)[0])[schemaindex])+"."+fieldName+": "+e.what());
        }
    }
    return buf;
}

//API

extern "C" {

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
    if (schema->t != 0) { return kerror("parse: schema must be general list"); }
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

K k_binparse_unparse(K schema, K input, K mainType) {
    if (mainType->t != -11) { return kerror("unparse: main type must be a symbol"); }
    if (schema->t != 0) { return kerror("unparse: schema must be general list"); }
    for (size_t i=0; i<kK(schema)[0]->n; ++i) {
        if (kS(kK(schema)[0])[i] == mainType->s) {
            Buffer buf;
            try {
                unparseRecord(buf, schema, input, i);
            } catch(const std::exception &e) {
                return kerror(e.what());
            }
            long size = buf.size();
            K result = ktn(KG,size);
            memcpy(kC(result), buf.data(), size);
            return result;
        }
    }
    return kerror("main type not found in schema");
}

}
