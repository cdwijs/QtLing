#ifndef CLEXICON_H
#define CLEXICON_H

#include <QMap>
#include <QMapIterator>
#include <QString>
#include <QList>
#include <QPair>
#include <QSet>
#include <QStatusBar>
#include "SignatureCollection.h"
#include "Typedefs.h"

class CWordCollection;
class CStemCollection;
class CSuffixCollection;
class CPrefixCollection;
class QProgressBar;
class CHypothesis;

//  part of an experiment:
class Collection
{
    eComponentType      m_component_type;

public:
    Collection(eComponentType this_type)        {m_component_type = this_type;}
    QString             get_output();
};

//  end of experiment


class simple_sig_graph_edge{
public:
    CSignature*         sig_1;
    CSignature*         sig_2;
    morph_t             morph;
    word_t              word;
    stem_t              stem_1;
    stem_t              stem_2;
    simple_sig_graph_edge();
    simple_sig_graph_edge(CSignature* sig1, CSignature* sig2,morph_t m,word_t w, stem_t stem1, stem_t stem2)
    {
        sig_1 = sig1;
        sig_2 = sig2;
        morph = m;
        word = w;
            stem_1 = stem1;
        stem_2 = stem2;
    };
    QString label() {return morph + "/" + sig_1->get_key() + "/" + sig_2->get_key(); }
};



class sig_graph_edge{
public:
    CSignature* sig_1;
    CSignature* sig_2;
    morph_t     morph;
    QMap<QString, word_stem_struct*>         shared_word_stems;
    sig_graph_edge();

    sig_graph_edge(simple_sig_graph_edge this_edge){
             sig_1 = this_edge.sig_1;
             sig_2 = this_edge.sig_2;
             morph = this_edge.morph;
             word_stem_struct * this_word_stems = new word_stem_struct;
             this_word_stems->word = this_edge.word;
             this_word_stems->stem_1 = this_edge.stem_1;
             this_word_stems->stem_2 = this_edge.stem_2;
             //shared_word_stems.append(this_word_stems);
             shared_word_stems[this_word_stems->get_label()] = this_word_stems;
         }
    QString label() {return morph + "/" + sig_1->get_key() + "/" + sig_2->get_key(); }
    int     get_number_of_words() {return shared_word_stems.size();}
    CSignature*     get_sig_1() {return sig_1;}
    CSignature*     get_sig_2() {return sig_2;}
    morph_t         get_morph() {return morph;}


};

class protostem{
        QString     m_protostem;
        int         m_start_word;
        int         m_end_word;
public:
        protostem(QString stem, int start_word, int end_word=-1) { m_protostem = stem; m_start_word = start_word; m_end_word = end_word;}
        QString     get_stem() { return m_protostem;}
        int         get_start_word() { return m_start_word;}
        int         get_end_word()   {return m_end_word;}
};
class CLexicon
{
protected:
                                                         // this is part of an experiment.
    QMap<QString,eComponentType>    m_category_types;    // part of the experiment. It serves
                                                        // as the principal way in which the Lexicon communicates
                                                        // with the GUI as far as architecture is concerned.
                                                        // Each entry in this must have a pointer to a real Collection (of the sort that follows here):
    CWordCollection *               m_Words;
    CStemCollection *               m_Stems;
    CSuffixCollection *             m_Suffixes;
    CPrefixCollection *             m_Prefixes;
    CSignatureCollection *          m_Signatures;
    CSignatureCollection *          m_PrefixSignatures;
    CWordCollection *               m_Compounds;
    QList<QPair<QString,QString>> * m_Parses;
    QMap<QString,int>               m_Parse_map;
    QMap<QString, int>              m_Protostems;
    QMap<QString, protostem*>        m_protostems_2;

    bool                            m_SuffixesFlag;
    CLexicon*                       m_parent_lexicon;

    CSignatureCollection*           m_ParaSignatures;   /*!<  the information we have about stems which we have not yet integrated into a morphological system. */
    CSuffixCollection *             m_ParaSuffixes;
    CStemCollection *               m_ResidualStems;
    CSignatureCollection *          m_ResidualPrefixSignatures;
    CStemCollection *               m_StemsFromSubsignatures;
    CSignatureCollection*           m_Subsignatures;
    QList<simple_sig_graph_edge*>   m_SigGraphEdgeList; /*!< the sig_graph_edges in here contain only one word associated with each. */
    lxa_sig_graph_edge_map          m_SigGraphEdgeMap;  /*!< the sig_graph_edges in here contain lists of words associated with them. */
    CSignatureCollection *          m_PassiveSignatures;  /*!< these signatures have stems one letter off from another signature. */
    CSignatureCollection *          m_SequentialSignatures; /*! signatures where one affix leads to another signature. */
    QList<CHypothesis*> *            m_Hypotheses;
// add component 1

    QProgressBar*                   m_ProgressBar;
    QStatusBar *                      m_StatusBar;

    double                          m_entropy_threshold_for_stems;

public:
    CLexicon(CLexicon* parent_lexicon = NULL, bool suffix_flag = true);
public:
    // accessors and protostems
//  part of an experiment:
    QMap<QString, eComponentType> &             get_category_types()        { return m_category_types;}
//  end of experiment;


    CWordCollection*                            get_word_collection()       { return m_Words; }
    CStemCollection*                            GetStemCollection()         { return m_Stems; }
    CSuffixCollection*                          GetSuffixCollection()       { return m_Suffixes; }
    CSuffixCollection*                          get_suffixes()              {return m_Suffixes;}
    CStemCollection *                           get_stems()                 { return m_Stems;}
    void                                        dump_suffixes(QList<QString>*);
    CWordCollection *                           get_words()                 { return m_Words;}
    CSignatureCollection*                       get_signatures()            { return m_Signatures;}
    CSignatureCollection*                       get_prefix_signatures()     { return m_PrefixSignatures;}
    CSignatureCollection*                       GetSignatureCollection()    { return m_Signatures; }
    CSignatureCollection *                      get_residual_signatures()   { return m_ParaSignatures;}
    CSuffixCollection*                          get_parasuffixes()          { return m_ParaSuffixes;}
    CSignatureCollection*                       get_passive_signatures()    { return m_PassiveSignatures;}
    CSignatureCollection *                      get_sequential_signatures() { return m_SequentialSignatures;}
    QList<CHypothesis*>*                        get_hypotheses ()           {return m_Hypotheses;}
    void                                        collect_parasuffixes();
    void                                        generate_hypotheses();
    double                                      get_entropy_threshold_for_positive_signatures() {return m_entropy_threshold_for_stems;}

    QList<QPair<QString,QString>>*              GetParses()                 { return m_Parses;}
    QMap<QString,int>*                          get_protostems()            { return &m_Protostems;}
    void                                        compute_sig_graph_edges();
    QList<simple_sig_graph_edge*> *             get_sig_graph_edges()        { return &m_SigGraphEdgeList;}
    lxa_sig_graph_edge_map *                    get_sig_graph_edge_map()     { return & m_SigGraphEdgeMap;}
    sig_graph_edge*                             get_sig_graph_edge(QString label) {return m_SigGraphEdgeMap[label];}
    QListIterator<simple_sig_graph_edge*>    *  get_sig_graph_edge_list_iter();
    lxa_sig_graph_edge_map_iter *               get_sig_graph_edge_map_iter();
    void                                        set_progress_bar (QProgressBar * pPB) { m_ProgressBar = pPB;}
    void                                        set_status_bar(QStatusBar* pBar) {m_StatusBar = pBar;}
    void                                        set_prefixes_flag()         { m_SuffixesFlag = false;}
    bool                                        get_suffix_flag()           { return m_SuffixesFlag; }


public:
    // insert functions here
    void Crab_1();
    void Crab_2();
    void FindProtostems();
    void CreateStemAffixPairs();
    void AssignSuffixesToStems();
    void ReSignaturizeWithKnownAffixes();
    void FindGoodSignaturesInsideParaSignatures();
    void compute_sig_graph_edge_map();
    void test_for_phonological_relations_between_signatures();
    void compare_opposite_sets_of_signatures(QSet<CSignature*>* sig_set_1, QSet<CSignature*>* sig_set_2,QString letter);
    void replace_parse_pairs_from_current_signature_structure(bool FindSuffixesFlag=true);
    void create_temporary_map_from_stems_to_affix_sets(map_sigstring_to_morph_set   &, map_sigstring_to_stem_list &);
    void create_sublexicon ();
};

#endif // CLEXICON_H
