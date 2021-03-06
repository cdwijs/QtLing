#ifndef GRAPHICS_H
#define GRAPHICS_H
#include<QGraphicsItem>
#include <QGraphicsView>
#include <QMouseEvent>
#include <QGraphicsScene>
#include <QPoint>
#include <QColor>
#include "supersignature.h"
#include <QGraphicsItem>
class CSignature;
class CSignatureCollection;
class MainWindow;
class lxa_graphics_scene;
class CSupersignature;

void triangle(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count, QColor color);
void square(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count);
void pentagon(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count);
void hexagon(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count);
void septagon(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count);
void octagon(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count);
void nonagon(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count);
void decagon(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count);
void elevenagon(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count);
void twelvagon(CSignature* pSig, int x, int y, int row_delta, lxa_graphics_scene * scene, int count);


/////////////////////////////////////////////////////////////////////////////
//          Graphic signature
//
/////////////////////////////////////////////////////////////////////////////
class graphic_signature : public QGraphicsRectItem // QGraphicsEllipseItem
{
    lxa_graphics_scene * m_graphics_scene;
    CSignature *         m_signature;
    Qt::GlobalColor      m_color;
    bool                 m_is_focused;

    public:
    graphic_signature   (int x, int y, CSignature*, lxa_graphics_scene* scene, int radius, int row_delta, QColor,  bool focus_flag = false);
    void                mousePressEvent (QGraphicsSceneMouseEvent*);
    void                set_color(Qt::GlobalColor this_color) { m_color = this_color;}
    CSignature*         get_signature() {return m_signature;}
    void                mark_as_focus();
};

class graphic_super_signature : public QRect
{
    lxa_graphics_scene *    m_graphics_scene;
    CSupersignature *       m_super_signature;
    Qt::GlobalColor         m_color;
    public:
    graphic_super_signature(int x, int y, CSupersignature*, lxa_graphics_scene* scene);
    void                    mousePressEvent (QGraphicsSceneMouseEvent*);
    void                    set_color(Qt::GlobalColor this_color) { m_color = this_color;}
    CSupersignature*        get_super_signature() {return m_super_signature;}
};


/////////////////////////////////////////////////////////////////////////////
//
//          lxa graphics view
//
/////////////////////////////////////////////////////////////////////////////

class lxa_graphics_view : public QGraphicsView
{   friend:: lxa_graphics_scene;

    QList<QList<CSignature*>*>      m_signature_lattice;
    MainWindow *                    m_main_window;
    lxa_graphics_scene*             m_graphics_scene;
    double                          m_scale;
    void                            mousePressEvent(QMouseEvent*);

public:
                                    lxa_graphics_view( MainWindow * );
    void                            expand() {scale(1.25,1.25);}
    void                            contract() {scale(0.8,0.8);}
    void                            move_up() ;
    void                            move_down() {translate(0,-50);}
    void                            move_left() {translate(-50,0);}
    void                            move_right() {translate(50,0);}
    lxa_graphics_scene*             get_graphics_scene() {return m_graphics_scene;}
    void                            set_graphics_scene( lxa_graphics_scene* pScene ) {m_graphics_scene = pScene;}


};

/////////////////////////////////////////////////////////////////////////////
//
//          lxa graphics scene
//
/////////////////////////////////////////////////////////////////////////////

class lxa_graphics_scene : public QGraphicsScene
{   friend          graphic_signature;

    MainWindow*                             m_main_window;
    eDisplayType                            m_display_type;
    lxa_graphics_view*                      m_graphics_view;
    QList<QList<CSignature*>*>              m_signature_lattice;
    QList<QPair<CSignature*,CSignature*>*>  m_signature_containment_edges;
    QMap<CSignature*, int>                  m_map_from_sig_to_column_no;
    graphic_signature *                     m_top_graphic_signature;
    CSignature*                             m_focus_signature_1;
    CSignature*                             m_focus_signature_2;
    CSignatureCollection *                  m_signature_collection;
    Qt::GlobalColor                         m_normal_color;
    Qt::GlobalColor                         m_focus_color;
    int                                     m_row_delta;
    int                                     m_column_delta;
    int                                     m_location_of_bottom_row;
    int                                     m_signature_radius;

private:
    void                                    mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent);
public:
    //                lxa_graphics_scene( MainWindow * , CSignatureCollection*, eDisplayType this_display_type );
                    ~lxa_graphics_scene();
                   lxa_graphics_scene(MainWindow *);
    void            clear_all();
    void            clear();
    void            set_graphics_view (lxa_graphics_view* );
//    void            set_parameters (CSignatureCollection*, eDisplayType);
    void            assign_scene_positions_to_signatures(CSignatureCollection*, eDisplayType );
    void            add_signature_containment_edge (QPair<CSignature*, CSignature*>* pPair)
                            {m_signature_containment_edges.append (pPair); }
    void            place_signatures();
    void            place_containment_edges();
    void            widen_columns();
    void            narrow_columns();
    void            move_rows_apart();
    void            move_rows_closer();
    void            mouseMoveEvent(QGraphicsSceneMouseEvent * event);
    void            set_focus_signature();
    void            set_focus_signature_1(CSignature* pSig)       {m_focus_signature_1 = pSig;}
    void            set_focus_signature_2(CSignature* pSig)       {m_focus_signature_2 = pSig;}
    graphic_signature* get_focus_signature_1();
    void            display_focus_signature();
};

class signature_node : public QGraphicsItem
{
public:
    QRectF boundingRect() const;
    void paint (QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget){
        painter->drawEllipse(100,100,10,10);
    }
};

#endif // GRAPHICS_H
