#include <QPair>
#include <QList>
#include <QMap>
#include <QMapIterator>
#include <QMultiMap>
#include <QSet>
#include <QtDebug>
#include <QProgressBar>
#include <QApplication>
#include <algorithm>
#include <QChar>
#include "Lexicon.h"

#include "SignatureCollection.h"
#include "StemCollection.h"
#include "SuffixCollection.h"
#include "WordCollection.h"
#include "Word.h"

CLexicon::CLexicon( CLexicon* lexicon, bool suffix_flag)
{
    m_Signatures            = new CSignatureCollection(this, true);
    m_PrefixSignatures      = new CSignatureCollection(this,false);
    m_Words                 = new CWordCollection(this);
    m_Stems                 = new CStemCollection(this);
    m_Suffixes              = new CSuffixCollection(this);
    m_Prefixes              = new CPrefixCollection(this);
    m_Compounds             = new CWordCollection(this);
    m_Parses                = new QList<QPair<QString,QString>>();
    m_Protostems            = QMap<QString, int>();
    m_ParaSignatures        =  new CSignatureCollection(this, true);
    m_ParaSuffixes          = new CSuffixCollection(this);
    m_ResidualStems         = new CStemCollection(this);
    m_PassiveSignatures     = new CSignatureCollection(this, true);
    m_SuffixesFlag          = suffix_flag;
    m_Hypotheses            = new QList<CHypothesis*>;
    m_entropy_threshold_for_stems = 1.2;
    m_parent_lexicon        = lexicon;

//  This is part of an experiment.
    m_category_types["Words"]               = CT_word;
    m_category_types["Stems"]               = CT_stem;
    m_category_types["Suffixes"]            = CT_affix;
    m_category_types["Prefixes"]            = CT_affix;
    m_category_types["Suffix signatures"]   = CT_signature;
    m_category_types["Prefix signatures"]   = CT_signature;
    m_category_types["Compounds"]           = CT_compound;
    m_category_types["Parses"]              = CT_parse;
    m_category_types["Protostems"]          = CT_stem;
    m_category_types["Parasignatures"]      = CT_signature;
    m_category_types["Parasuffixes"]        = CT_affix;
    m_category_types["Hypotheses"]          = CT_hypothesis;
    m_category_types["Hypotheses 2"]          = CT_hypothesis2;
    // add component 2
}


QListIterator<simple_sig_graph_edge*> * CLexicon::get_sig_graph_edge_list_iter()
{
    QListIterator<simple_sig_graph_edge*> * iter = new QListIterator<simple_sig_graph_edge*>(m_SigGraphEdgeList);
    return iter;
}

QMapIterator<QString, sig_graph_edge*> * CLexicon::get_sig_graph_edge_map_iter()
{
    QMapIterator<QString, sig_graph_edge*> * iter = new QMapIterator<QString, sig_graph_edge*>(m_SigGraphEdgeMap);
    return iter;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
//linguistic methods
/**
 * @brief CLexicon::Crab_1
 * The division of the Crab algorithm into two parts (as I have done here) is only temporary.
 * I have done it so that I can do code development more quickly; sometimes I only need to
 * get part of the algorithm going to test what I am doing.
 */

void CLexicon::Crab_1()
{
    FindProtostems();

    CreateStemAffixPairs();

    AssignSuffixesToStems();

    collect_parasuffixes();

    m_SuffixesFlag?
        m_Signatures->compute_containment_list():
        m_PrefixSignatures->compute_containment_list();

    qDebug() << "finished crab 1.";
 }

/**
 * @brief CLexicon::Crab_2
 * Crab_2 is a bit slower than Crab_1.
 */
void CLexicon::Crab_2()
{
    ReSignaturizeWithKnownAffixes();
    FindGoodSignaturesInsideParaSignatures();
    m_SuffixesFlag ?
        m_Signatures->calculate_stem_entropy():
        m_PrefixSignatures->calculate_stem_entropy();
    compute_sig_graph_edges();
    compute_sig_graph_edge_map();
    generate_hypotheses();
    test_for_phonological_relations_between_signatures();
    qDebug() << "finished crab 2.";
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/*!
 * This is the first of the three initial parts of finding signatures.
 * This makes a cut at every point in a word where the successor frequency
 * is greater than 1.
 */
void CLexicon::FindProtostems()
{   word_t          this_word, previous_word;
    QStringList *   Words = get_word_collection()->GetSortedStringArray();
    bool            StartFlag = true;
    bool            DifferenceFoundFlag = false;
    stem_t          stem;
    int             this_word_length(0), previous_word_length(0);
    m_ProgressBar->reset();
    m_ProgressBar->setMinimum(0);
    m_ProgressBar->setMaximum(Words->size());
    m_StatusBar->showMessage("Proto-stems.");
    for (int wordno=0; wordno<Words->size(); wordno ++){
        m_ProgressBar->setValue(wordno);
        if (m_SuffixesFlag){
            this_word = Words->at(wordno);
        } else{
            this_word = get_words()->get_reverse_sort_list()->at(wordno);
        }
        this_word_length = this_word.length();
        if (StartFlag){
            StartFlag = false;
            previous_word = this_word;
            previous_word_length = this_word_length;
            continue;
        }
        DifferenceFoundFlag = false;
        int end_word;
        int wordno2;
        if (m_SuffixesFlag){
            int end = qMin(this_word_length, previous_word_length);
            for (int i=0; i <end; i++){
                if (previous_word[i] != this_word[i]){
                    stem = previous_word.left(i);
                    if (stem.length()== 0){continue;}
                    DifferenceFoundFlag = true;
                    if (!m_Protostems.contains(stem))                {
                        m_Protostems[stem] = 1;
                    }
                    if (!m_protostems_2.contains(stem)){
                        for (wordno2 = wordno; wordno2 < m_Words->get_count(); wordno2++ ){
                            if ( ! Words->at(wordno2).startsWith(stem) ){
                                end_word = wordno2 - 1;
                                break;                            }
                        } // add case for last word on list here.
                        protostem * this_protostem = new protostem(stem, wordno-1, end_word );
                        m_protostems_2[stem] = this_protostem;
                    }
                    break;
                }
            }
        }  // end of suffix case.
        else
        {       // -->   Prefix case   <-- //
            this_word_length = this_word.length();
            previous_word_length = previous_word.length();
            int end = qMin(this_word_length, previous_word_length);
            for (int i=0; i <end; i++){
                if (previous_word[previous_word_length - i] != this_word[ this_word_length - i]){
                    stem = previous_word.right(i-1);
                    DifferenceFoundFlag = true;

                    if (!m_Protostems.contains(stem))                {
                        m_Protostems[stem] = 1;
                    }
                    break;
                }
            }
        }
        if (DifferenceFoundFlag == true){
            previous_word = this_word;
            continue;
        }
        else {
            if (previous_word.length() < this_word.length()) {
                m_Protostems[previous_word] = 1;
            }
        }
        previous_word = this_word;
        previous_word_length = this_word_length;
    }
    return;
}

/*!
 * This is the second of the three initial parts of finding signatures.
 * This creates stem/affix pairs, which are put in a long list of "Parses".
 */
void CLexicon::CreateStemAffixPairs()
{
    m_ProgressBar->reset();
    m_ProgressBar->setMinimum(0);
    m_ProgressBar->setMaximum(m_Words->get_count());
    m_StatusBar->showMessage("Stem-affix pairs.");
    QString                     stem, suffix, word, prefix;
    int                         suffix_length, prefix_length, wordno (0);
    map_string_to_word_ptr_iter *   word_iter = m_Words->get_iterator();
    bool                        DoNotParseCompoundsFlag = true;
    while (word_iter->hasNext())   {
        word = word_iter->next().value()->GetWord();
        if (DoNotParseCompoundsFlag and word.contains("-")){
            *m_Compounds << word;
            continue;
        }
        wordno++;
        m_ProgressBar->setValue(wordno);
        int MinimumStemLength = 3;
        for (int letterno = MinimumStemLength; letterno < word.length(); letterno++){
            if (m_SuffixesFlag){
                stem = word.left(letterno);
                //if (m_Protostems.contains(stem)){
                if(m_protostems_2.contains(stem))  {
                    suffix_length = word.length() - letterno;
                    suffix = word.right(suffix_length);
                    m_Parses->append(QPair<QString,QString>(stem,suffix));
                    if (m_Words->contains(stem)){
                        m_Parses->append(QPair<QString,QString>(stem,QString("NULL")));
                    }
                }
            }else{
                stem = word.right(letterno);
                if (m_Protostems.contains(stem)){
                    prefix_length = word.length() - letterno;
                    prefix = word.left(prefix_length);
                    m_Parses->append(QPair<QString,QString>(prefix,stem));
                    if (m_Words->contains(stem)){
                        m_Parses->append(QPair<QString,QString>(QString("NULL"),stem));

                    }
                }
            } // end of prefixes.
        }
    }
}

/*!
 * This is the third of the three initial parts of finding signatures.
 * This creates signatures, which in turn creates stems and affixes.
 */
void   CLexicon::AssignSuffixesToStems()
{   const int MINIMUM_NUMBER_OF_STEMS = 2;
    CWord *                     pWord;
    QPair<QString,QString>      this_pair;
    CSignature*                 pSig;
    QString                     this_stem_t, this_suffix, this_prefix, this_affix, this_signature_string, this_word;
    stem_list *                 p_this_stem_list;
    affix_set *                 this_ptr_to_affix_set;
    CStem*                      pStem;
    map_sigstring_to_suffix_set      temp_stems_to_affix_set;
    map_sigstring_to_stem_list        temp_signatures_to_stems;
    morph_set *                 pSet;
    m_ProgressBar->reset();
    m_ProgressBar->setMinimum(0);
    m_ProgressBar->setMaximum(m_Parses->size());
    m_StatusBar->showMessage("Form signatures: 1. temporary stems and affixes.");
    //--> We establish a temporary map from stems to sets of affixes as we iterate through parses. <--//
    for (int parseno = 0; parseno < m_Parses->size(); parseno++){
        m_ProgressBar->setValue(parseno);
        this_pair = m_Parses->at(parseno);
        if (m_SuffixesFlag){
            this_stem_t = this_pair.first;
            this_affix = this_pair.second;
        } else{
            this_stem_t = this_pair.second;
            this_affix = this_pair.first;

        }
        if (! temp_stems_to_affix_set.contains(this_stem_t)){
            if (m_SuffixesFlag){
                pSet = new suffix_set();
            } else{
                pSet = new prefix_set();
            }
            temp_stems_to_affix_set.insert(this_stem_t,pSet);
        }
        temp_stems_to_affix_set.value(this_stem_t)->insert(this_affix);
    }
    qDebug() << "Step 1.";
    //--> We iterate through these stems and for each stem, create QStringLists of their affixes. <--//
    //--> then we create a "pre-signature" in a map that points to lists of stems. <--//
    m_ProgressBar->reset();
    m_ProgressBar->setMinimum(0);
    m_ProgressBar->setMaximum(temp_stems_to_affix_set.count());
    m_StatusBar->showMessage("Form signatures: 2. tentative signatures.");

    int count= 0;
    QMapIterator<QString, morph_set*>   stem_iter(temp_stems_to_affix_set);                       // part 1
    while (stem_iter.hasNext())                                                                  // make a presignature for each stem.
    {    qApp->processEvents();
         count ++;
         m_ProgressBar->setValue(count);
         stem_iter.next();
         this_stem_t            = stem_iter.key();
         this_ptr_to_affix_set  = stem_iter.value();
         QStringList temp_presignature;


         affix_set_iter affix_iter (*this_ptr_to_affix_set);
         while (affix_iter.hasNext()){
                 temp_presignature.append ( affix_iter.next() );
         }

         temp_presignature.sort();
         sigstring_t this_signature_string = temp_presignature.join("=");
         if ( ! temp_signatures_to_stems.contains(this_signature_string)){
            stem_list * pStemSet = new stem_list;
            temp_signatures_to_stems[this_signature_string] = pStemSet;
         }
         temp_signatures_to_stems.value(this_signature_string)->append(this_stem_t);
    }

    //-->  create signatures, stems, affixes:  <--//
    m_ProgressBar->reset();
    m_ProgressBar->setMinimum(0);
    m_ProgressBar->setMaximum(temp_signatures_to_stems. count());
    m_StatusBar->showMessage("Form signatures: 3. final step.");
    count = 0;
    QMapIterator<sigstring_t, stem_list*> iter_sigstring_to_stems ( temp_signatures_to_stems);
     // -->  Iterate through tentative signatures.    <-- //
    while (iter_sigstring_to_stems.hasNext())
    {
        qApp->processEvents();
        count ++;
        m_ProgressBar->setValue(count);
        iter_sigstring_to_stems.next();
        this_signature_string    = iter_sigstring_to_stems.key();
        p_this_stem_list         = iter_sigstring_to_stems.value();

        affix_set this_affix_set = QSet<QString>::fromList( this_signature_string.split("="));

        if (p_this_stem_list->size() >= MINIMUM_NUMBER_OF_STEMS)
        {
            if( m_SuffixesFlag) {
                pSig = *m_Signatures       << this_signature_string;
            } else {
                pSig = *m_PrefixSignatures << this_signature_string;
                pSig->set_suffix_flag(false);

            }
            pSig->add_memo("Pass 1");
            QSetIterator<suffix_t> affix_iter(this_affix_set);
            while(affix_iter.hasNext()){
                  this_affix = affix_iter.next();
                  if (m_SuffixesFlag){
                      CSuffix* pSuffix = m_Suffixes->find_or_add(this_affix);
                      pSuffix->increment_count();
                      pSig->add_affix_ptr(pSuffix);
                  }else{
                      CPrefix* pPrefix = m_Prefixes->find_or_add(this_affix);
                      pPrefix->increment_count();
                      pSig->add_affix_ptr(pPrefix);
                  }
            }
            m_StatusBar->showMessage("Form signatures: 3b");
            // --> We go through this sig's stems and reconstitute its words. <--//
            stem_list_iterator stem_iter(*p_this_stem_list);
            while (stem_iter.hasNext()){
                this_stem_t = stem_iter.next();

                pStem = m_Stems->find_or_add(this_stem_t);
                pStem->add_signature (this_signature_string);
                pSig->add_stem_pointer(pStem);

                //QStringList affixes = this_signature_string.split("=");
                QSetIterator<suffix_t> affix_iter(this_affix_set);
                while(affix_iter.hasNext()){
                    this_affix = affix_iter.next();
                    //                for (int affixno = 0; affixno < affixes.size(); affixno++)
                    //                {   this_affix = affixes[affixno];
                    if (this_affix == "NULL"){
                        this_word = this_stem_t;
                    } else{
                        m_SuffixesFlag ?
                            this_word = this_stem_t + this_affix :
                            this_word = this_affix + this_stem_t ;
                    }
                    CWord* pWord = m_Words->get_word(this_word);
                    pWord->add_stem_and_signature(pStem,pSig);
                    QString message = this_signature_string;
                    if (this_affix_set.size() > 50){message = "Super long signature";};
                    pWord->add_to_autobiography("Pass1= " + this_stem_t + "=" + message);
                }
            }
        }
    }
    m_Suffixes->sort_by_count();
    m_SuffixesFlag ?
        m_Signatures->calculate_stem_entropy():
        m_PrefixSignatures->calculate_stem_entropy();
    m_StatusBar->showMessage("Computation of Crab 1 completed.");
}

bool contains(QList<QString> * list2, QList<QString> * list1){
    for (int i=0; i < list1->length();i++){
        bool success = false;
        for (int j = 0; j < list2->length();j++){
            if (list1->at(i) == list2->at(j)){
                success = true;
            }
        }
        if (success == false){
            return false;
        }
    }
    return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//      End of Crab 1
/////////////////////////////////////////////////////////////////////////////////////////////////////////////




/*!
* This is lke AssignSuffixesToStems, but crucially: with Suffixes known ahead of time.
* This creates signatures and stems: signatures that only have approved affixes.
* First function of Crab_2.
* I need to refactorize this function.
*/
void CLexicon::ReSignaturizeWithKnownAffixes()

{   const int MINIMUM_NUMBER_OF_STEMS = 2;
   CWord *                     pWord;
   CSignature*                 pSig;
   QString                     this_stem_t, this_suffix_t, this_prefix, this_affix, this_signature_string, this_word;
   stem_list *                 p_this_stem_list;
   suffix_set *                this_ptr_to_suffix_set;
   affix_set *                 this_ptr_to_affix_set;
   CStem*                      pStem;
   map_sigstring_to_suffix_set temp_stems_to_affix_set;
   map_sigstring_to_morph_set& ref_stems_to_affix_set (temp_stems_to_affix_set);
   map_sigstring_to_stem_list  temp_signatures_to_stems;
   map_sigstring_to_stem_list& ref_temp_signatures_to_stems(temp_signatures_to_stems);

   m_StatusBar->showMessage("Resignaturize with known affixes");
   m_ProgressBar->reset();
   m_ProgressBar->setMinimum(0);
   m_ProgressBar->setMaximum(m_Parses->size());

   map_string_to_word_ptr_iter word_iter (*m_Words->get_map());
   while(word_iter.hasNext()){
       pWord = word_iter.next().value();
       pWord->clear_signatures();
   }
   //--> We establish a temporary map from stems to sets of affixes as we iterate through parses. <--//

   create_temporary_map_from_stems_to_affix_sets( ref_stems_to_affix_set, ref_temp_signatures_to_stems);

   //--> We iterate through these stems and for each stem, create QStringLists of their affixes. <--//
   //--> then we create a "pre-signature" in a map that points to lists of stems. <--//
   QMapIterator<QString, morph_set*>   stem_iter(temp_stems_to_affix_set);                       // part 1
   m_ProgressBar->reset();
   m_ProgressBar->setMinimum(0);
   m_ProgressBar->setMaximum(temp_stems_to_affix_set.size());
   int stem_count = 0;
   while (stem_iter.hasNext())                                                                  // make a presignature for each stem.
   {    stem_iter.next();
        m_ProgressBar->setValue(stem_count++);
        this_stem_t            = stem_iter.key();
        this_ptr_to_affix_set  = stem_iter.value();
        if (this_ptr_to_affix_set->size() < 2){continue;}
        QStringList temp_presignature;


        affix_set_iter affix_iter (*this_ptr_to_affix_set);
        while (affix_iter.hasNext()){
                temp_presignature.append ( affix_iter.next() );
        }

        temp_presignature.sort();
        sigstring_t this_signature_string = temp_presignature.join("=");
        if ( ! temp_signatures_to_stems.contains(this_signature_string)){
           stem_list * pStemSet = new stem_list;
           temp_signatures_to_stems[this_signature_string] = pStemSet;
        }
        temp_signatures_to_stems.value(this_signature_string)->append(this_stem_t);
   }

   //-->  create signatures, stems, affixes:  <--//
   m_Stems->clear();
   if (m_SuffixesFlag){
       m_Signatures->clear();
   }else {
       m_PrefixSignatures->clear();
   }
   QMapIterator<sigstring_t, stem_list*> iter_sigstring_to_stems ( temp_signatures_to_stems);
   while (iter_sigstring_to_stems.hasNext())
   {   iter_sigstring_to_stems.next();
       this_signature_string    = iter_sigstring_to_stems.key();
       p_this_stem_list         = iter_sigstring_to_stems.value();
       this_stem_t =  p_this_stem_list->first();
       affix_set this_affix_set = QSet<QString>::fromList( this_signature_string.split("="));
       if (p_this_stem_list->size() >= MINIMUM_NUMBER_OF_STEMS)
       {   m_SuffixesFlag ?
               pSig = *m_Signatures       << this_signature_string :
               pSig = *m_PrefixSignatures << this_signature_string;
           pSig->add_memo("Pass 2");
           QSetIterator<suffix_t> affix_iter(this_affix_set);
           while(affix_iter.hasNext()){
                 this_affix = affix_iter.next();
                 if (m_SuffixesFlag){
                     CSuffix* pSuffix = m_Suffixes->find_or_add(this_affix);
                     pSuffix->increment_count();
                     pSig->add_affix_ptr(pSuffix);
                 }else{
                     CPrefix* pPrefix = m_Prefixes->find_or_add(this_affix);
                     pPrefix->increment_count();
                     pSig->add_affix_ptr(pPrefix);
                 }
           }

           // --> We go through this sig's stems and reconstitute its words. <--//
           stem_list_iterator stem_iter(*p_this_stem_list);
           while (stem_iter.hasNext()){
               this_stem_t = stem_iter.next();
               pStem = m_Stems->find_or_add(this_stem_t);
               pStem->add_signature (this_signature_string);
               pSig->add_stem_pointer(pStem);
               pStem->add_memo ("Pass1= ");
               QStringList affixes = this_signature_string.split("=");
               foreach (affix_t this_affix,  affixes){
                   if (this_affix == "NULL"){
                       this_word = this_stem_t;
                   } else{
                       m_SuffixesFlag ?
                           this_word = this_stem_t + this_affix :
                           this_word = this_affix + this_stem_t ;
                   }
                   CWord* pWord = m_Words->get_word(this_word);
                   pWord->add_stem_and_signature(pStem,pSig);
                   QString message = this_signature_string;
                   if (affixes.size()> 50){message = "very long signature";}
                   pWord->add_to_autobiography("Pass2= " + this_stem_t + "=" + message );
               }
           }

       }else{       // insufficient number of stems ...
           this_signature_string =  iter_sigstring_to_stems.key();
           pSig =  *m_ParaSignatures << this_signature_string;
           pStem = *m_ResidualStems << this_stem_t;
           pSig->add_stem_pointer(pStem);
           foreach (this_affix, this_affix_set){
               if (this_affix == "NULL"){
                   this_word = this_stem_t;
               } else{
                   m_SuffixesFlag ?
                      this_word = this_stem_t + this_affix :
                      this_word = this_affix + this_stem_t;
               }
               pWord = m_Words->find_or_fail(this_word);
               QString message = this_signature_string;
               if (this_affix_set.size()> 50){message = "very long signature";}
               pWord->add_to_autobiography("*" + this_word + "=" + this_stem_t + "=" + message );
           }
       }
   }
   m_Signatures->sort_each_signatures_stems_alphabetically();
}

/**
 * helper function for preceeding function.
 *
 */
void CLexicon::create_temporary_map_from_stems_to_affix_sets(map_sigstring_to_morph_set & ref_stems_to_affix_set,
                                                             map_sigstring_to_stem_list & ref_temp_signatures_to_stems){
    QPair<QString,QString>      this_pair;
    QString                     this_stem_t, this_suffix_t;
    morph_set *                 pSet;

    // iterate through parselist, and assign to stem cand affix collections;
    for (int parseno = 0; parseno < m_Parses->size(); parseno++){
        this_pair = m_Parses->at(parseno);
        m_ProgressBar->setValue(parseno);
        if (m_SuffixesFlag){
            this_stem_t = this_pair.first;
            this_suffix_t = this_pair.second;
            if (! m_Suffixes->contains(this_suffix_t)){
                continue;
            }
        } else{
            this_stem_t = this_pair.second;
            this_suffix_t = this_pair.first;
            if (! m_Prefixes->contains(this_suffix_t)){
                continue;
            }
        }

        if (! ref_stems_to_affix_set.contains(this_stem_t)){
            if (m_SuffixesFlag){
                pSet = new suffix_set();
            } else{
                pSet = new prefix_set();
            }
            ref_stems_to_affix_set.insert(this_stem_t,pSet);
        }
        ref_stems_to_affix_set.value(this_stem_t)->insert(this_suffix_t);
        }
}

/*!
 *  This function looks at all the non-signatures that were rejected
 *  because they were associated with only one stem. For each one, it
 *  looks for the broadest signature that occurs inside it, and assigns
 *  its stem to that signature.
 *
 * 2nd function of Crab 2.
 */


void   CLexicon::FindGoodSignaturesInsideParaSignatures()
{   stem_t                      this_stem;
    word_t                      this_word;
    affix_t                    this_affix;
    sig_string                  this_signature_string;
    CStem*                      pStem;
    CWord*                      pWord;
    CSignature*                 pSig, *p_proven_sig;
    CSuffix*                    pSuffix1;
    int                         suffix_no;
    int                         count_of_new_stems = 0;
    int                         count_of_new_words = 0;
    int                         signature_count (0);
    affix_list                  affixes_of_residual_sig;
    CSuffix_ptr_list  *         list_of_CSuffixes_of_proven_sig;
    CSuffix_ptr_list            this_residual_sig_suffix_pointer_list;
    bool                        success_flag;
    CSignatureCollection*       signatures;
    suffix_t                    Null_string ("NULL");
    CSuffix*                    pNullSuffix = *m_Suffixes << Null_string;

                                m_ProgressBar->reset();
                                m_ProgressBar->setMinimum(0);
                                m_ProgressBar->setMaximum(m_ParaSignatures->get_count());
                                m_StatusBar->showMessage("Find good signatures inside bad.");

    //---->   We iterate through the list of Residual Signatures <-------//

    if (m_SuffixesFlag) {
         signatures = m_Signatures;
    } else{
         signatures = m_PrefixSignatures;
    }
    signatures->sort(SIG_BY_AFFIX_COUNT);
    map_sigstring_to_sig_ptr_iter   sig_iter(*  m_ParaSignatures->get_map());


    // instead of going through parasignatures, we will look at protostems that are not stems.
    // We will not keep Parasignatures or residual stems.

    int protostem_count = 0;
    m_ProgressBar->setMaximum(m_protostems_2.count());
    foreach (auto this_protostem, m_protostems_2)
    {
        affixes_of_residual_sig.clear();
        m_ProgressBar->setValue(protostem_count++);
        stem_t this_stem = this_protostem->get_stem();
        //qDebug() << this_stem << this_protostem->get_start_word() << this_protostem->get_end_word() << 710;
        int stem_length = this_stem.length();

        for (int wordno= this_protostem->get_start_word(); wordno <= this_protostem->get_end_word(); wordno++){
            //qDebug() << m_Words->get_count() << 714;
            QString this_word = m_Words->get_word_string(wordno);
            //qDebug() << this_stem << this_word << 716;
            QString ending = this_word.mid( stem_length );
            affixes_of_residual_sig.append( ending );
        }
        if (m_Words->contains(this_stem)) {
                affixes_of_residual_sig.append("NULL");
        }
        //qDebug() << affixes_of_residual_sig.join("-") << 721;
        //--> Now we look for largest true signature inside this list of suffixes. <--//
        //--> Inner loop, over all good signatures. <--//
        for (int sig_no=0; sig_no < signatures->get_count(); sig_no++){
            p_proven_sig = signatures->get_at_sorted(sig_no);
            QString p_proven_sigstring  = p_proven_sig->get_key();
            QList<QString> proven_sig_list = p_proven_sigstring.split("=");
            if ( contains(&affixes_of_residual_sig, &proven_sig_list) ){

                // We have found the longest signature contained in this_residual_suffix_set
                pStem = m_Stems->find_or_add(this_stem);
                pStem->add_signature(p_proven_sigstring);
                p_proven_sig->add_stem_pointer(pStem);

                //qDebug() << this_stem << affixes_of_residual_sig << proven_sig_list;
                //--> add to autobiographies <--//

                for (int affixno = 0; affixno < proven_sig_list.length(); affixno++){
                    this_affix = proven_sig_list[affixno];
                    if (this_affix == "NULL"){
                        this_word= this_stem;
                    }else{
                        m_SuffixesFlag?
                            this_word = this_stem + this_affix:
                            this_word = this_affix + this_stem;
                    }
                    pWord = m_Words->find_or_fail(this_word);
                    if (pWord){
                        pWord->add_stem_and_signature(pStem, p_proven_sig);
                        pWord->add_to_autobiography("from within parasigs="  + this_stem  + "=" +  p_proven_sigstring);
                    }
                }
                break;
            }
        } // loop over proven signatures;
    }

    //--> Outer loop, over all Residual Signatures. <--//

    /*
    m_ProgressBar->setMaximum(m_ParaSignatures->get_count());
    m_ProgressBar->reset();
    while (sig_iter.hasNext()){
        sig_iter.next();
        signature_count++;
        m_ProgressBar->setValue(signature_count);
        qApp->processEvents();
        CSignature*         pResidualSig              = sig_iter.value();
                            affixes_of_residual_sig  = pResidualSig->get_key().split("=");
                            this_stem = pResidualSig->get_stems()->first()->get_key(); // there is only 1 stem in these signatures, by construction.
                            if (m_Words->contains(this_stem)){
                              affixes_of_residual_sig.append ("NULL");
                            }
                            //--> Now we look for largest true signature inside this list of suffixes. <--//
                            //--> Inner loop, over all good signatures. <--//

                            for (int sig_no=0; sig_no < signatures->get_count(); sig_no++){
                                p_proven_sig = signatures->get_at_sorted(sig_no);
                                QString p_proven_sigstring  = p_proven_sig->get_key();
                                QList<QString> proven_sig_list = p_proven_sigstring.split("=");
                                if ( contains(&affixes_of_residual_sig, &proven_sig_list) ){

                                    // We have found the longest signature contained in this_residual_suffix_set
                                    pStem = m_Stems->find_or_add(this_stem);
                                    pStem->add_signature(p_proven_sigstring);
                                    p_proven_sig->add_stem_pointer(pStem);

                                    //--> add to autobiographies <--//

                                    for (int affixno = 0; affixno < proven_sig_list.length(); affixno++){
                                        this_affix = proven_sig_list[affixno];
                                        if (this_affix == "NULL"){
                                            this_word= this_stem;
                                        }else{
                                            m_SuffixesFlag?
                                                this_word = this_stem + this_affix:
                                                this_word = this_affix + this_stem;
                                        }
                                        pWord = m_Words->find_or_fail(this_word);
                                        if (pWord){
                                            pWord->add_stem_and_signature(pStem, p_proven_sig);
                                            pWord->add_to_autobiography("from within parasigs="  + this_stem  + "=" +  p_proven_sigstring);
                                        }
                                    }
                                    break;
                                }
                            } // loop over proven signatures;
    }

    */
    m_Signatures->sort_each_signatures_stems_alphabetically();
}



struct{
    bool operator ()(CSignature* pSig_a, CSignature* pSig_b) const {
    // return pSig_a->number_of_true_suffixes() > pSig_b->number_of_true_suffixes();
    }
}custom_compare_residual_sig;



/*!
 * Replace parse pairs from current signature structure. This allows us to
 * delete the old signature structure, and replace them with the set of
 * parse-pairs that exactly describe the current signature structure.
 */

void CLexicon::replace_parse_pairs_from_current_signature_structure(bool FindSuffixesFlag) {
    m_Parses->clear();
    m_Parse_map.clear();
    QString                         sig_string;
    CSignature*                     pSig;
    CStem*                          pStem;
    QList<CStem*> *                 stem_list;
    map_sigstring_to_sig_ptr_iter  * sig_iter =  get_signatures()->get_map_iterator();

    while (sig_iter->hasNext()){
        pSig = sig_iter->next().value();
        stem_list =  pSig->get_stems();
    }
}





/*!
 * We can build a graph whose nodes are the signatures, where an edge connects
 * any pair of signatures if there exists a word that they both analyze.
 * A sig_graph_edge has two flavors: this function uses the flavor that
 * contains the two signatures, the word, and the string-difference between
 * the stems of the word at the two signatures.
 * The edge has: signature1, signature2, a morph, and a pair of stems.
 *
 * This is Part 1 of the 3rd major function of Crab 2.
 */
void CLexicon::compute_sig_graph_edges()
{
    map_string_to_word *            WordMap = m_Words->GetMap();
    map_string_to_word_ptr_iter     word_iter(*WordMap);
    simple_sig_graph_edge *          p_SigTreeEdge;
    CSignatureCollection*           pSignatures;
    CWord*                          pWord;
    morph_t                         difference;
    int                             number_of_signatures;

    while (word_iter.hasNext())   {
        pWord = word_iter.next().value();
        number_of_signatures = pWord->GetSignatures()->size();
        if ( number_of_signatures > 1){
            for (int signo1=0; signo1 < number_of_signatures; signo1++){
                stem_sig_pair* pair1 =  pWord->GetSignatures()->value(signo1);
                CStem * stem1        = pair1->first;
                int stem1length      = stem1->get_key().length();
                CSignature* sig1     = pair1->second;
                for (int signo2=signo1 + 1; signo2 < number_of_signatures; signo2++){
                    stem_sig_pair * pair2 = pWord->GetSignatures()->value(signo2);
                    CStem *  stem2   = pair2->first;
                    CSignature* sig2 = pair2->second;
                    if (sig1 == sig2){continue;}
                    int stem2length = stem2->get_key().length();
                    // the following "if" is there so that the "difference" can be simply defined.
                    //
                    if (stem1length > stem2length){
                        int length_of_difference = stem1length - stem2length;
                        m_SuffixesFlag?
                            difference = stem1->get_key().mid(stem2->get_key().length()):
                            difference = stem1->get_key().left(length_of_difference);
                        p_SigTreeEdge = new simple_sig_graph_edge (sig1,sig2,difference, pWord->get_key(), stem1->get_key(), stem2->get_key());
                    } else{
                        int length_of_difference = stem2length - stem1length;
                        m_SuffixesFlag?
                            difference = stem2->get_key().mid(stem1->get_key().length()):
                            difference = stem2->get_key().left(length_of_difference);
                        p_SigTreeEdge =  new simple_sig_graph_edge (sig2,sig1,difference, pWord->get_key(), stem2->get_key(), stem1->get_key());
                    }
                    m_SigGraphEdgeList.append(p_SigTreeEdge);
                    pWord->add_to_autobiography("multiple parse=" + stem1->get_key() + "=" +  sig1->get_key() + "=" + stem2->get_key() + "=" + sig2->get_key());
                }
            }
        }
    }
}

/*!
 * This function takes the SigTreeEdge list, and makes a smaller list composed of
 * SigTreeEdges which share the same signature pair and string-difference. This
 * flavor of sig_graph_edge contains a list of all the words that participate in
 * this particular sig_graph_edge.
 *
 * This is Part 2 of the 3rd major function of Crab 2.
 */
void CLexicon::compute_sig_graph_edge_map() {
morph_t         edge_label;
word_t          this_word;
simple_sig_graph_edge * p_sig_graph_edge;
sig_graph_edge        * p_sig_graph_edge_2,
                     * p_sig_graph_edge_3;
lxa_sig_graph_edge_map* p_EdgeMap = & m_SigGraphEdgeMap;

QListIterator<simple_sig_graph_edge*> this_simple_sig_graph_edge_iter (m_SigGraphEdgeList);
while (this_simple_sig_graph_edge_iter.hasNext())
{
    p_sig_graph_edge = this_simple_sig_graph_edge_iter.next();
    edge_label = p_sig_graph_edge->label();
    this_word  = p_sig_graph_edge->word;
    // --> We iterate through the simple Edges contained in the TreeEdge List            <-- //
    // --> We build a map of larger TreeEdges, each containing multiple stems and words. <-- //
    if (p_EdgeMap->contains(edge_label)){
        p_sig_graph_edge_3 = p_EdgeMap->value(edge_label);
        word_stem_struct * this_word_stem_struct = new word_stem_struct;
        this_word_stem_struct->word = this_word;
        this_word_stem_struct->stem_1 = p_sig_graph_edge->stem_1;
        this_word_stem_struct->stem_2 = p_sig_graph_edge->stem_2;
        QString this_label = this_word_stem_struct->get_label();
        if ( ! p_sig_graph_edge_3->shared_word_stems.contains(this_label)){
               p_sig_graph_edge_3->shared_word_stems[this_label] = this_word_stem_struct;
        }
     } else {  // --> start a new sig_graph_edge with multiple stems <-- //
        sig_graph_edge * p_sig_graph_edge_2 = new sig_graph_edge(*p_sig_graph_edge);
        m_SigGraphEdgeMap.insert(p_sig_graph_edge_2->label(),p_sig_graph_edge_2);
     }
}
}

/*!
 * This function looks at pairs of signatures joined by a sig-tree-edge, where
 * the morpheme that separates them is a single letter. We look to see  how tight
 * the fit is between these two sets of signatures.
 */
void CLexicon::test_for_phonological_relations_between_signatures()
{
   lxa_sig_graph_edge_map_iter  sig_iter (m_SigGraphEdgeMap);
   QString difference;
   QSet<QString> differences_1_letter, differences_longer;
   while (sig_iter.hasNext()){
       sig_iter.next();
       QString morph = sig_iter.value()->morph;
       if (morph.length() == 1){
        differences_1_letter.insert(sig_iter.value()->morph);
       } else{
        differences_longer.insert(sig_iter.value()->morph);
       }
   }

   QStringList differences = differences_1_letter.toList();
   differences.sort();
   QSet<CSignature*> SignatureSet_1, SignatureSet_2;

   for (int i = 0; i < differences.size(); i++){
       difference = differences[i];
       //--> Pull out the sig_graph_edges with each particular difference (morpheme).
       sig_iter.toFront();
       while (sig_iter.hasNext()){
             sig_iter.next();
             if (sig_iter.value()->morph == difference){
                 CSignature* pSig1 = sig_iter.value()->sig_1;
                 CSignature* pSig2 = sig_iter.value()->sig_2;
                 SignatureSet_1.insert(pSig1);
                 SignatureSet_2.insert(pSig2);
                 *m_PassiveSignatures << pSig1;
             }
       }

       //--> Now these two sets have signatures which are from opposite sides of the phonological rule. <---//
       compare_opposite_sets_of_signatures(&SignatureSet_1, &SignatureSet_2, difference);

   }
}

/*!
 * This function takes two sets of signatures, each taken from the opposing signatures
 * in a sig-tree-edge, where the morph separating them is a specific morph of length = 1 letter.
 *
 * This is not currently being used.
 */
void CLexicon::compare_opposite_sets_of_signatures(QSet<CSignature*>* sig_set_1, QSet<CSignature*>* sig_set_2, QString morph)
{   sig_graph_edge * p_edge;
    CSignature* pSig_1, *pSig_2;
    QHash<QString,int> Counts;
    foreach(p_edge,  m_SigGraphEdgeMap){
        if (p_edge->morph == morph){
            pSig_1 = p_edge->sig_1;
            pSig_2 = p_edge->sig_2;
            QString code = pSig_1->get_key() + "@" + pSig_2->get_key();
            Counts[code] = p_edge->get_number_of_words();
        }
    }
}




void CLexicon::dump_suffixes(QList<QString> * pList)
{
    return m_Suffixes->get_suffixes(pList);
}

/**
 * @brief CLexicon::collect_parasuffixes
 * Parasignatures are signatures with only a single stem, hence are not used directly.
 * Parasuffixes are affixes found in a parasignature.
 *
 * Not currently in use.
 */
void CLexicon::collect_parasuffixes()
{   sigstring_t     sigstring;
    suffix_t        suffix;
    CSignature*     pSig;
    CSuffix *       pSuffix;
    QStringList     suffixes;
    map_sigstring_to_sig_ptr_iter sig_iter (* m_ParaSignatures->get_map());
    while (sig_iter.hasNext())
    { pSig = sig_iter.next().value();
        sigstring = pSig->get_key();
        suffixes = sigstring.split("=");
        foreach (suffix, suffixes){
            pSuffix = *m_ParaSuffixes <<  suffix;
            pSuffix->increment_count();
        }
    }
    m_ParaSuffixes->sort_by_count();
};





