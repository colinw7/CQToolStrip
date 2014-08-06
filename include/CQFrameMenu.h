#ifndef CQFrameMenu_H
#define CQFrameMenu_H

#include <QMenu>
#include <QFrame>
#include <QWidgetAction>

class QScrollBar;
class QVBoxLayout;
class CQFrameMenuFrame;
class CQFrameMenuAction;
class CQFrameMenuScrollArea;

class CQFrameMenu : public QMenu {
  Q_OBJECT

  Q_PROPERTY(bool scrollable READ scrollable WRITE setScrollable)

 public:
  enum Side {
    NO_SIDE = 0,

    LEFT_SIDE   = (1<<0),
    RIGHT_SIDE  = (1<<1),
    TOP_SIDE    = (1<<2),
    BOTTOM_SIDE = (1<<3),

    TOP_LEFT_SIDE  = (LEFT_SIDE|TOP_SIDE),
    TOP_RIGHT_SIDE = (RIGHT_SIDE|TOP_SIDE),

    BOTTOM_LEFT_SIDE  = (LEFT_SIDE|BOTTOM_SIDE),
    BOTTOM_RIGHT_SIDE = (RIGHT_SIDE|BOTTOM_SIDE),

    ALL_SIDES = (LEFT_SIDE|RIGHT_SIDE|TOP_SIDE|BOTTOM_SIDE)
  };

 public:
  CQFrameMenu(bool scrollable=false);

  QFrame *frame() const;

  //! get/set scrollable
  bool scrollable() const { return scrollable_; }
  void setScrollable(bool scrollable);

  //! get/set resize sides
  uint resizeSides() const { return resizeSides_; }
  void setResizeSides(uint sides);

  //! get/set size hint is maximum size
  bool sizeHintIsMax() const { return sizeHintIsMax_; }
  void setSizeHintIsMax(bool v) { sizeHintIsMax_ = v; }

  //! get/set popup widget
  QWidget *popupWidget() const { return popupWidget_; }
  void setPopupWidget(QWidget *w);

  //! set associated widget
  void setWidget(QWidget *w);

  //! popup at global position
  void popup(const QPoint &gpos);

  void adjustMenuRect(int dxl, int dyb, int dxr, int dyt);

  void initSize(const QSize &s);

  void processFrameEvent(QFrame *frame, QEvent *e);

  bool insideBorder(QFrame *frame, const QPoint &p, Side &side) const;

  void updateCursor(QFrame *frame, const QPoint &p);

 signals:
  void openMenu();
  void closeMenu();

 private slots:
  void aboutToShowSlot();
  void aboutToHideSlot();

 private:
  void initFrame();

 private:
  bool                   scrollable_;
  uint                   resizeSides_;
  QWidget               *popupWidget_;
  bool                   sizeHintIsMax_;
  CQFrameMenuFrame      *frame_;
  CQFrameMenuAction     *action_;
  CQFrameMenuScrollArea *scrollArea_;
  bool                   sideInited_;
  bool                   pressed_;
  QPoint                 pressPos_;
  CQFrameMenu::Side      pressSide_;
};

class CQFrameMenuFrame : public QFrame {
  Q_OBJECT

 public:
  CQFrameMenuFrame(CQFrameMenu *menu);

  void setWidget(QWidget *w);

 private:
  void showEvent(QShowEvent *);
  void hideEvent(QHideEvent *);

  bool eventFilter(QObject *o, QEvent *e);

 private:
  CQFrameMenu *menu_;
  QVBoxLayout *layout_;
};

class CQFrameMenuAction : public QWidgetAction {
 public:
  CQFrameMenuAction(CQFrameMenu *menu);

  QWidget *createWidget(QWidget *parent);
  void     deleteWidget(QWidget *widget);

 private:
  CQFrameMenu *menu_;
};

class CQFrameMenuScrollArea : public QFrame {
  Q_OBJECT

 public:
  CQFrameMenuScrollArea(CQFrameMenu *menu);

  void setWidget(QWidget *w);

  QWidget *widget() const { return w_; }

  QSize initSize();

  void applySize(int w, int h);

  void resizeEvent(QResizeEvent *);

  void updateSize(int w, int h);

  QSize minimumSizeHint() const;
  QSize maximumSizeHint() const;

 private slots:
  void hscrollSlot(int value);
  void vscrollSlot(int value);

 private:
  void showEvent(QShowEvent *);
  void hideEvent(QHideEvent *);

  bool eventFilter(QObject *o, QEvent *e);

 private:
  CQFrameMenu *menu_;
  QWidget     *contents_;
  QWidget     *w_;
  QScrollBar  *hbar_;
  QScrollBar  *vbar_;
  int          cw_, ch_;
};

#endif
