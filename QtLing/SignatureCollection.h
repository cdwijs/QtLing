#ifndef CSIGNATURECOLLECTION_H
#define CSIGNATURECOLLECTION_H

#include <QList>
#include <iterator>
#include "generaldefinitions.h"
//#include "StringSurrogate.h"
#include "Signature.h"

//class CSignature;
class CLexicon;
class CParse;

class CSignatureCollection
{
protected:

    QMap<QString, CSignature*> m_SignatureMap;
    int m_CorpusCount;
    QString m_MemberName;
    QList<CSignature*> m_SortList;
    bool m_SortValidFlag;
    enum eSortStyle m_SortStyle;

public:
    CSignatureCollection();
//    CSignatureCollection(CLexicon* Lex, QString MemberName = QString());
    ~CSignatureCollection();

// disable copy
private:
    CSignatureCollection(const CSignatureCollection& x);
    CSignatureCollection& operator=(const CSignatureCollection& x);


public:
    friend class CLexicon;
    CSignature* operator<< ( CParse* );
//    CSignature* operator<< ( CStringSurrogate );
    CSignature* operator<< ( QString );
    void        operator<< ( CSignature* );
    CSignature* operator^= ( CParse* );
 //   CSignature* operator^= ( CStringSurrogate );
    CSignature* operator^= ( QString );
    CSignature* find_or_add ( QString); // same as operatorˆ=

    CSignature* GetAt( uint );
    int get_count() const { return m_SignatureMap.size(); }
    QMap<QString, CSignature*> GetSignatures() const { return m_SignatureMap; }
    QList<CSignature*>*  GetSortedSignatures() { return &  m_SortList;}

    CSignature* get_signature(QString sig) {return m_SignatureMap.value(sig); }

    void sort();


};
#endif // CSIGNATURECOLLECTION_H
