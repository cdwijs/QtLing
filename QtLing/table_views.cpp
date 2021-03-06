#include <QDebug>
#include <QMap>
#include <QFont>
#include <QHeaderView>
#include "Lexicon.h"
#include "mainwindow.h"
#include "Word.h"
#include "WordCollection.h"
#include "graphics.h"
#include "lxamodels.h"
class sig_graph_edge;

/**
 * @brief LowerTableView::LowerTableView
 * This is the view on the lower right part of the screen.
 * It usually gives additional information when the user clicks on the Upper Table View.
 */
LowerTableView::LowerTableView()
{
   m_my_current_model =new  QStandardItemModel();
   m_number_of_columns = 20;
   QFont sansFont("Ariel", 20);
   setFont(sansFont);


}
LowerTableView::LowerTableView(MainWindow * window)
{
   m_my_current_model =new  QStandardItemModel();
   m_parent_window = window;
   m_number_of_columns = 20;
   m_lexicon = window->get_lexicon();
   m_current_sortstyle = 0;
   QFont sansFont("Ariel", 20);
   setFont(sansFont);

}
/**
  * @brief LowerTableView::display_this_item
  * @param index
  * This is the normal way to display something in this view. It starts with information
  * from the user clicking the upper TableView. The UpperTable may be display many different
  * kinds of things, so this function is broken up into pieces, each depending on what
  * the Upper TableView is currently displaying.
  * NB: This View can either by a Table or a Graphical View.
  * The choice of these two options is controlled by keypress: Ctrl-G (for Graphics).
  */
 void LowerTableView::display_this_item( const  QModelIndex & index )
 {
     eDocumentType              UpperView_type = m_parent_window->m_tableView_upper_left->get_document_type();
//     QString                    component = m_parent_window->m_tableView_upper_left->get_content();
     QString                    word, stem, prefix, suffix, signature;
     CLexicon *                 this_lexicon = get_parent_window()->get_lexicon();
     int                        row, column;
     QStandardItem*             p_item;
     QList<QStandardItem*>      item_list;

     switch (UpperView_type){
        case WORDS:{
            if (index.isValid()) {row = index.row();}
            QString word = index.sibling(row,0).data().toString();
            CWord* pWord = this_lexicon->get_words()->get_word(word);
            table_word(pWord);
            setModel( m_my_current_model);
            break;}
         //  ---------------------------------------------------//
        case  SIGNATURES:
        case  EPOSITIVE_SIGNATURES:{
            if (index.isValid()) {row = index.row();}
            signature = index.sibling(row,0).data().toString();
            CSignature*  pSig = this_lexicon->get_signatures()->get_signature(signature);

            // -->  If Graphics is NOT showing, we display the information about the signature <-- //
            if (m_parent_window->m_graphic_display_flag == false){
               table_signature(pSig);
               setModel( m_my_current_model);
            }
            break;}
         //  ---------------------------------------------------//
        case PREFIX_SIGNATURES:
        case EPOSITIVE_PREFIX_SIGNATURES:{
              item_list.clear();
              if (index.isValid()) {row = index.row();}
              signature = index.sibling(row,0).data().toString();
              CSignature*           pSig = this_lexicon->get_prefix_signatures()->get_signature(signature);
              // -->  If Graphics is NOT showing, we display the information about the signature <-- //
              if (m_parent_window->m_graphic_display_flag == false){
                 table_signature(pSig);
                 setModel( m_my_current_model);
              } // -->   Now, graphics display IS showing<-- //
              {
                 qDebug() << "trying to display a focus signature";
                 m_parent_window->get_graphics_scene()->set_focus_signature_1(pSig);
                 m_parent_window->get_graphics_scene()->display_focus_signature();
              }
              break;}
         //  ---------------------------------------------------//
        case RESIDUAL_PARASIGNATURES:{
              if (index.isValid()){row = index.row();}
              item_list.clear();
              signature = index.sibling(row,0).data().toString();
              CSignature*           pSig = this_lexicon->get_signatures()->get_signature(signature);
              CStem*                p_Stem;
              CStem_ptr_list     *  sig_stems = pSig->get_stems();
              QStandardItem*        p_item;
              if (m_my_current_model) {
                  delete m_my_current_model;
              }
              m_my_current_model = new QStandardItemModel();
              foreach (p_Stem, *sig_stems)  {
              p_item = new QStandardItem(p_Stem->get_key() );
              item_list.append(p_item);
              if (item_list.length() >= m_number_of_columns){
                         m_my_current_model->appendRow(item_list);
                         item_list.clear();
                }
              }
              setModel( m_my_current_model);
              break;}
         //  ---------------------------------------------------//
        case PASSIVE_SIGNATURES:{
            item_list.clear();
            if (index.isValid()){
                row = index.row();
                column = index.column();
            }
            if (m_my_current_model) {
                delete m_my_current_model;
            }
            m_my_current_model = new QStandardItemModel();
            sig_string sig = index.sibling(row,0).data().toString();
            CSignature* pSig;
            this_lexicon->get_suffix_flag()?
                     pSig = this_lexicon->get_signatures()->get_signature(sig):
                     pSig = this_lexicon->get_prefix_signatures()->get_signature(sig);
            table_passive_signature(pSig);
            setModel( m_my_current_model);
            break;}
           //  ---------------------------------------------------//
     case SUFFIXES:{
            break;
             }
         //  ---------------------------------------------------//

     case  SIGNATURE_GRAPH_EDGES:{
            item_list.clear();
            if (index.isValid()){
                 row = index.row();
                 column = index.column();
            }
            QString               edge_key = index.sibling(row,5).data().toString();
            sig_graph_edge*       this_edge = this_lexicon->get_sig_graph_edge_map()->value(edge_key);
            sig_graph_edge *      psig_graph_edge;
            CSignature*           pSig;
            CSignature*           pSig1 = this_edge->sig_1;
            CSignature*           pSig2 = this_edge->sig_2;
            QStandardItem*        p_item;
            CStem*                p_Stem;
            QStringList           sig1_stems;
            QStringList           sig2_stems;
            QStringList           words;
            word_stem_struct *    this_word_stem_item;
            if (m_parent_window->m_graphic_display_flag){
                //-->  Graphic display in lower right window <--//
                if (column == 1){
                    pSig = pSig1;
                } else { pSig = pSig2;}
                graphics_sig_graph_edges(pSig, this_lexicon );
            } else
             // -->   Tabular display in lower right window <--//
            {   foreach (this_word_stem_item, this_edge->shared_word_stems){
                    words.append(     this_word_stem_item->word);
                    sig1_stems.append(this_word_stem_item->stem_1);
                    sig2_stems.append(this_word_stem_item->stem_2);
                }
                if (m_my_current_model) {
                    delete m_my_current_model;
                }
                m_my_current_model = new QStandardItemModel();
                // --> first signature <-- //
                table_one_signature(pSig1, sig1_stems );
                // --> second signature <-- //
                table_one_signature(pSig2, sig2_stems );
                // --> words <-- //
                m_my_current_model->appendRow(item_list);    // blank row in table.
                item_list.clear();
                m_my_current_model->appendRow(item_list);
                item_list.clear();
                for (int wordno= 0; wordno< words.size();wordno++)  {
                    p_item = new QStandardItem(words[wordno]);
                    item_list.append(p_item);
                    if (item_list.length() >= m_number_of_columns){
                        m_my_current_model->appendRow(item_list);
                        item_list.clear();
                    }
                }
                if (item_list.size() > 0 ){
                    m_my_current_model->appendRow(item_list);
                    item_list.clear();
                }
                // --> end of words <-- //
                setModel( m_my_current_model);
                resizeColumnsToContents();
            } // end of tabular display, lower right window
         break;}
    case  HYPOTHESES:
          break;

    // add component 9
    default:
         break;
     }


    resizeColumnsToContents();
 }
//------------------->         <---------------------------------------------------//
 /**
  * @brief LeftSideTreeView::LeftSideTreeView
  * @param window
  *
  */
 LeftSideTreeView::LeftSideTreeView(MainWindow* window)
 {
     m_parent_window = window;
 }

 /**
 * @brief UpperTableView::display_signatures
 * When we click on an affix in the upper-table on the left,
 * we display signatures on the right upper-table that contain that affix.
 * We should just put that in a model, and link the model to that view.
 */
/*void UpperTableView::display_signatures()
{
    if (index.isValid()){
        row = index.row();
        column = index.column();
    }
    qDebug() << 145 <<  "table views suffixes";
    m_my_current_model->clear();
    CSignature* pSig;
    QStandardItem*        p_item;
    suffix_t suffix = index.sibling(row,0).data().toString();
    auto sig_iter (get_lexicon()->get_signatures()->get_map_iterator());
    while (sig_iter->hasNext()){
        pSig = sig_iter->next().value();
        if (pSig->contains_affix_string(suffix)){
            p_item = new QStandardItem(pSig->get_key());
            m_my_current_model->appendRow(p_item);
        }
    }
    get_parent_window()->get
}
*/
 /**
  * @brief UpperTableView::display_this_affixes_signatures
  * This is used when we click on the left upper view to identify an affix;
  * then we display all signatures containing that affix on the right.
  */
 void  UpperTableView::display_this_affixes_signatures(const QModelIndex & index)
 {
     if (index.isValid()){

     }

 }

 /**
 * @brief UpperTableView::UpperTableView
 * @param window
 * This TableView responds to the User's click on the LeftSideTreeView.
 */
UpperTableView::UpperTableView (MainWindow* window, eSortStyle this_sort_style)
{
        m_parent_window = window;
        m_signature_sort_style = this_sort_style;
        QFont sansFont("Ariel", 20);
        setFont(sansFont);

}

/**
 * @brief UpperTableView::ShowModelsUpperTableView
 * @param index
 * What is displayed in the UpperTableView is the direct result
 * of what item the user clicks on the LeftTree; that is the
 * input to this function.
 * This function chooses one of the many Qt models that
 * Crab generated when it had finished its work.
 *
 * I have set up two knds of UpperTableViews, so that they can be seen
 * side by side. They differ by sorting style. Signatures can be
 * sorted by the number of stems or the number of affixes, for example.
 *
 * TODO: Make the different sortings be done by a Surrogate QView sorter.
 * Right now I have created two different models for different sorts
 * of the Signatures, which isn't the right way to do things in principle. Dec 2017.
 * And I have had to do this several times.
 */
void UpperTableView::ShowModelsUpperTableView(const QModelIndex& index)
{
    CLexicon* lexicon = m_parent_window->get_lexicon();
    QString component;
    if (index.isValid()){
        component = index.data().toString();
    }
    if (component == "Words"){
        setModel(m_parent_window->m_Models["Words"]);
        set_document_type( WORDS );
    }
    else     if (component == "Stems"){
        setModel(m_parent_window->m_Models["Stems"]);
        set_document_type( STEMS );
    }
    else     if (component == "Suffixes"){
        m_parent_window->display_suffixes();
        //setModel(m_parent_window->m_Models["Suffixes"]);
        //set_document_type( SUFFIXES );
    }
    else     if (component == "Signatures"){
        m_parent_window->display_suffix_signatures();
    }
    else     if (component == "EPositive signatures"){
        setModel(m_parent_window->m_Models["EPositive signatures"]);
        set_document_type( EPOSITIVE_SIGNATURES );
        set_content_type( "signatures");

        m_parent_window->m_graphics_scene->clear_all();
        m_parent_window->m_graphics_scene->assign_scene_positions_to_signatures(lexicon->get_signatures(), DT_Positive_Suffix_Signatures);
        m_parent_window->m_graphics_scene->place_signatures();
        qDebug() << "epositive sigs"<< 292;
    }
    else     if (component == "Prefix signatures"){
        m_parent_window->display_prefix_signatures();
    }
    else     if (component == "EPositive prefix signatures"){
        setModel(m_parent_window->m_Models["EPositive prefix signatures"]);
        set_document_type( EPOSITIVE_PREFIX_SIGNATURES );
        set_content_type( "signatures");
    }
    else     if (component == "Signature graph edges"){
        setModel(m_parent_window->m_Models["SigGraphEdges"]);
        set_document_type( SIGNATURE_GRAPH_EDGES );
        sortByColumn(-1);
    }
    else     if (component == "Residual parasignatures"){
        setModel(m_parent_window->m_Models["Residual parasignatures"]);
        set_document_type( SIGNATURE_RESIDUES );
        sortByColumn(1);
        qDebug() << 238 << "table views";
    }
    else     if (component == "Parasuffixes"){
        setModel(m_parent_window->m_Models["Parasuffixes"]);
        set_document_type( PARASUFFIXES );
        sortByColumn(1);
    }
    else     if (component == "Singleton signatures"){
        set_document_type( SINGLETON_SIGNATURES );
        sortByColumn(1);
    }
    else     if (component == "Passive signatures"){
        setModel(m_parent_window->m_Models["Passive signatures"]);
        set_document_type( PASSIVE_SIGNATURES );
        sortByColumn(1);    }
    else     if (component == "Hypotheses"){
        m_parent_window->display_hypotheses();
    }
// add component 10



    resizeColumnsToContents();
}














/**
 * @brief LowerTableView::graphics_sig_graph_edges
 * @param pSig
 * @param p_lexicon
 *
 *
 */
void LowerTableView::graphics_sig_graph_edges(CSignature* pSig, CLexicon* p_lexicon)
{
    sig_graph_edge *       psig_graph_edge;
    //-->  Graphic display in lower right window <--//
    // "Signatures" is what should be sent to the Model in the mainwindow.
    CSignatureCollection Signatures(p_lexicon);
    Signatures << pSig;
    // We iterate through the SigTreeEdges in the SigTreeEdgeMap. If its Sig1 is pSig, then we enter it into Signatures;
    QMap<QString,sig_graph_edge*>::iterator edge_iter = p_lexicon->get_sig_graph_edge_map()->begin();
    while (edge_iter !=  p_lexicon->get_sig_graph_edge_map()->constEnd()){
        psig_graph_edge = edge_iter.value();
        if (psig_graph_edge->sig_1 == pSig){
            Signatures << psig_graph_edge->sig_2;
        }
        ++edge_iter;
    }

    // put this back in when things are figured out ****
//    m_parent_window->m_graphics_scene = new lxa_graphics_scene(m_parent_window,  &Signatures,  DT_sig_graph_edges);
//    m_parent_window->m_graphics_scene->place_signatures();
//    m_parent_window->m_graphics_view->setScene(m_parent_window->m_graphics_scene);
}

/**
 * @brief LowerTableView::table_word
 * @param pWord
 * What follows is a set of functions that display different kinds of user-requested information on the Lower Table View.
 */
void LowerTableView::table_word(CWord* pWord ){
    QList<QStandardItem*>      item_list;
    QStandardItem *            p_item, *q_item;

    // Create a clean model.
    if (m_my_current_model){
        delete m_my_current_model;
    }
    m_my_current_model = new QStandardItemModel();

    // Find the word's autobiography and set it, line by line, in the lower TableView.
    QListIterator<QString> line_iter(*pWord->get_autobiography());
    while (line_iter.hasNext()){
        QString report_line = line_iter.next();
        item_list.clear();
        QStringList report_line_items = report_line.split("=");
        for (int i = 0; i < report_line_items.size(); i++){
            p_item = new QStandardItem(report_line_items[i]);
            if (i == 0 && report_line_items[i][0] == "*"){
                p_item->setBackground(Qt::red);
            } else{
                p_item->setBackground(Qt::white);
            }
            item_list.append(p_item);
        }
        m_my_current_model->appendRow(item_list);
    }
    // Display the signatures this word bears:
    for (int signo = 0; signo < pWord->m_Signatures.size(); signo++){
        qDebug() << signo;
        QString sig = pWord->GetSignatures()->at(signo)->second->get_key();
        QString stem = pWord->GetSignatures()->at(signo)->first->get_key();
        item_list.clear();
        p_item = new QStandardItem(stem);
        item_list.append(p_item);
        q_item = new QStandardItem(sig);
        item_list.append(q_item);
        m_my_current_model->appendRow(item_list);
    }

}
void LowerTableView::table_signature(CSignature* pSig ){

    QStandardItem*             p_item;
    QList<QStandardItem*>      item_list;

    item_list.clear();
    CStem*                p_Stem;
    CStem_ptr_list    *   sig_stems = pSig->get_stems();
    if (m_my_current_model) { delete m_my_current_model;}
    m_my_current_model = new QStandardItemModel();

    p_item = new QStandardItem("Stem final letter entropy");

    m_my_current_model->appendRow(item_list);
    item_list.append(p_item);
    p_item = new QStandardItem(QString::number(pSig->get_stem_entropy()));
    item_list.append(p_item);
    m_my_current_model->appendRow(item_list);

    item_list.clear();
    foreach (p_Stem, *sig_stems)  {
        p_item = new QStandardItem(p_Stem->get_key() );
        item_list.append(p_item);
        if (item_list.length() >= m_number_of_columns){
            m_my_current_model->appendRow(item_list);
            item_list.clear();
        }
    }
    if (item_list.size() > 0){
        m_my_current_model->appendRow(item_list);
    }
    //resizeColumnsToContents();

}

/**
 * @brief LowerTableView::table_one_signature
 * @param pSig
 * @param stems
 *
 * This displays information about a single signature.
 */
void LowerTableView::table_one_signature(CSignature* pSig, QStringList stems)
{
    QStandardItem*             p_item;
    QList<QStandardItem*>      item_list;
    CStem*                     p_Stem;

    p_item = new QStandardItem(pSig->get_key());
    item_list.append(p_item);
    m_my_current_model->appendRow(item_list);
    item_list.clear();
    foreach(p_Stem, *pSig->get_stems()){
        stem_t this_stem_t = p_Stem->get_key();
        p_item = new QStandardItem(this_stem_t);
        if (stems.contains(this_stem_t)){
            p_item->setBackground(Qt::red);
        }
        item_list.append(p_item);
        if (item_list.length() >= m_number_of_columns){
            m_my_current_model->appendRow(item_list);
            item_list.clear();
        }
    }
    if (item_list.size() > 0 ){
        m_my_current_model->appendRow(item_list);
        item_list.clear();
    }
}
/**
 * @brief LowerTableView::table_passive_signature
 * @param p_this_sig
 * A passive signature is a signature in which the set of stems all end (in the case of a suffixal system) with the same letter, or nearly so.
 * Such signatures are unhealthy but sometimes very useful.
 */
void LowerTableView::table_passive_signature(CSignature *p_this_sig)
{
    QStandardItem*              p_item;
    QList<QStandardItem*>       item_list;
    sig_graph_edge *             p_edge;
    QMap<CSignature*, int>      stem_counter; // key is signature, value is number of stems shared with p_this_sig;
    QList<CSignature*>          sig_list;
    m_my_current_model =        new QStandardItemModel();
    QString                     morph;
    QMap<CSignature*, QString>  Morphs;

    // put signatures in a Map, values are numbers of stems shared
    QMap<QString, sig_graph_edge*> * pMap = get_lexicon()->get_sig_graph_edge_map();
    QMapIterator<QString, sig_graph_edge*> this_sig_graph_edge_iter (*pMap);
    while (this_sig_graph_edge_iter.hasNext()){
        sig_graph_edge * p_edge  = this_sig_graph_edge_iter.next().value();
        if (p_this_sig == p_edge->sig_1){
            stem_counter[p_edge->sig_2] = p_edge->get_number_of_words();
            Morphs[p_edge->sig_2] = p_edge->morph;
        }
    }

    QStringList         temp_signatures;
    QList<CSignature*>  sorted_signatures;
    QList<int>          counts = stem_counter.values();
    std::sort(counts.begin(), counts.end());
    int                 count;

    for (int i = 0; i < counts.size(); i++){
            qDebug() << i <<404;
            count = counts[i];
            QList<CSignature*> signatures =stem_counter.keys(count);
            std::sort(signatures.begin(), signatures.end());
            foreach (CSignature* pSig, signatures){
                sorted_signatures.append(pSig);
            }
     }






 //        // --> this sorts by morph, then sig in column 2 <-- //
//        foreach (int count, counts){
//            QList<CSignature*> signatures =stem_counter.keys(count);
//            std::sort(signatures.begin(), signatures.end());
//            foreach (CSignature* pSig, signatures){
//                sorted_signatures.append(pSig);
//            }
//        }
//    }

    for (int signo = sorted_signatures.count()-1; signo >= 0 ;signo-- ){
        CSignature* pSig2 = sorted_signatures[signo];
        sig_string sig2 = pSig2->get_key();
        item_list.clear();
        p_item = new QStandardItem(Morphs[pSig2]);
        item_list.append(p_item);
        p_item = new QStandardItem(sig2);
        item_list.append(p_item);
        p_item = new QStandardItem(QString::number(stem_counter[pSig2]) );
        item_list.append(p_item);
        m_my_current_model->appendRow(item_list);
        }

}
