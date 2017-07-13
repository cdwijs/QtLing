#ifndef CWORDCOLLECTION_H
#define CWORDCOLLECTION_H

#include <QList>
#include <iterator>
#include "generaldefinitions.h"
#include "StringSurrogate.h"

class CWord;
class CLexicon;
class CParse;

class CWordCollection
{
protected:
    QList<CWord> m_WordList;
    QMap<QString, int> m_WordMap; //relates string to freq, doesn't require CWord object
    int m_CorpusCount;
    QString m_MemberName;
//    CWord** m_PointerArray; //what does this array do?
    CWord** m_SortArray;
    bool m_SortValidFlag;
    enum eSortStyle m_SortStyle;

    public:
    // construction/destruction.

    CWordCollection();
//    CWordCollection(CLexicon* Lex, QString MemberName = QString());
    ~CWordCollection();

// disable copy
private:
    CWordCollection(const CWordCollection& x);
    CWordCollection& operator=(const CWordCollection& x);

public:
    CWord* operator<< ( CParse* );
    CWord* operator<< ( CStringSurrogate );
    CWord* operator<< ( QString );
    CWord* operator^= ( CParse* );
    CWord* operator^= ( CStringSurrogate );
    CWord* operator^= ( QString );
//    CWord* operator[] ( uint n );

    CWord GetAt( uint );
    int GetLength() const { return m_WordList.length(); };

    QList<CWord>::iterator GetBegin()   { return m_WordList.begin();  };
    QList<CWord>::iterator GetEnd()     { return m_WordList.end();  };

};

#endif // CWORDCOLLECTION_H