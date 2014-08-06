#include <CQFrameMenu.h>
#include <CQWidgetUtil.h>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <iostream>

CQFrameMenu::
CQFrameMenu(bool scrollable) :
 QMenu(0), scrollable_(scrollable), resizeSides_(ALL_SIDES), popupWidget_(0),
 sizeHintIsMax_(false), frame_(0), scrollArea_(0), sideInited_(false), pressed_(false)
{
  action_ = new CQFrameMenuAction(this);

  QMenu::addAction(action_);

  connect(this, SIGNAL(aboutToShow()), this, SLOT(aboutToShowSlot()));
  connect(this, SIGNAL(aboutToHide()), this, SLOT(aboutToHideSlot()));
}

void
CQFrameMenu::
setScrollable(bool scrollable)
{
  scrollable_ = scrollable;
}

void
CQFrameMenu::
setResizeSides(uint sides)
{
  resizeSides_ = sides;
}

void
CQFrameMenu::
setPopupWidget(QWidget *w)
{
  popupWidget_ = w;
}

void
CQFrameMenu::
initFrame()
{
  if (scrollable_) {
    if (! scrollArea_)
      scrollArea_ = new CQFrameMenuScrollArea(this);
  }
  else {
    if (! frame_)
      frame_ = new CQFrameMenuFrame(this);
  }
}

QFrame *
CQFrameMenu::
frame() const
{
  const_cast<CQFrameMenu *>(this)->initFrame();

  if (scrollable_)
    return scrollArea_;
  else
    return frame_;
}

void
CQFrameMenu::
setWidget(QWidget *w)
{
  initFrame();

  if (scrollable_)
    scrollArea_->setWidget(w);
  else
    frame_->setWidget(w);
}

void
CQFrameMenu::
popup(const QPoint &gpos)
{
  QMenu::popup(gpos);
}

void
CQFrameMenu::
aboutToShowSlot()
{
  sideInited_ = false;

  emit openMenu();

  if (scrollable_) {
    QSize s = scrollArea_->initSize();

    initSize(s);
  }
}

void
CQFrameMenu::
aboutToHideSlot()
{
  emit closeMenu();
}

void
CQFrameMenu::
adjustMenuRect(int dxl, int dyb, int dxr, int dyt)
{
  QRect ar = actionGeometry(action_);

  int dx = std::max(ar.x(), 0);
  int dy = std::max(ar.y(), 0);

  QRect r = geometry();

  QRect dr = r.adjusted(dxl, dyb, dxr, dyt);
//std::cerr << "adjustMenuRect " << dr.width() << " " << dr.height() << std::endl;

  if (scrollable_) {
    QSize minSize = scrollArea_->minimumSizeHint();
    QSize maxSize = scrollArea_->maximumSizeHint();

    int minW = minSize.width () + 2*dx;
    int minH = minSize.height() + 2*dy;

    int maxW = maxSize.width () + 2*dx;
    int maxH = maxSize.height() + 2*dy;

//std::cerr << "minSize " << minW << " " << minH << std::endl;

    if (dr.width() < minW || dr.width() > maxW) {
      dxl = 0;
      dxr = 0;
    }

    if (dr.height() < minH || dr.height() > maxH) {
      dyb = 0;
      dyt = 0;
    }

    dr = r.adjusted(dxl, dyb, dxr, dyt);
  }

  move(dr.x(), dr.y());

  int fw = dr.width () - 2*dx;
  int fh = dr.height() - 2*dy;

  if (scrollable_) {
    scrollArea_->applySize(fw, fh);

    fw = scrollArea_->width ();
    fh = scrollArea_->height();
  }
  else
    frame_->setFixedSize(QSize(fw, fh));

  int mw = fw + 2*dx;
  int mh = fh + 2*dy;

  //resize(mw, mh);
  setFixedSize(mw, mh);
}

void
CQFrameMenu::
initSize(const QSize &s)
{
//std::cerr << "initSize " << s.width() << " " << s.height() << std::endl;
  QRect ar = actionGeometry(action_);

  int dx = std::max(ar.x(), 0);
  int dy = std::max(ar.y(), 0);
//std::cerr << "dx/dy " << dx << " " << dy << std::endl;

  int fw = s.width ();
  int fh = s.height();

  if (scrollable_) {
    scrollArea_->applySize(fw, fh);

    fw = scrollArea_->width ();
    fh = scrollArea_->height();
  }
  else
    frame_->setFixedSize(QSize(fw, fh));

  int mw = fw + 2*dx;
  int mh = fh + 2*dy;

  //resize(mw, mh);
  setFixedSize(mw, mh);
}

void
CQFrameMenu::
processFrameEvent(QFrame *frame, QEvent *e)
{
  switch (e->type()) {
    case QEvent::MouseButtonPress: {
      QMouseEvent *me = static_cast<QMouseEvent *>(e);

      Side side;

      if (insideBorder(frame, me->pos(), side)) {
        //std::cerr << "MouseButtonPress" << std::endl;
        pressSide_ = side;
        pressPos_  = me->globalPos();
        pressed_   = true;
      }

      break;
    }
    case QEvent::MouseButtonRelease: {
      if (pressed_) {
        //std::cerr << "MouseButtonRelease" << std::endl;
        pressed_ = false;
      }

      break;
    }
    case QEvent::MouseMove: {
      QMouseEvent *me = static_cast<QMouseEvent *>(e);

      if (pressed_) {
        bool moving = (me->modifiers() & Qt::AltModifier);

        QPoint pos = me->globalPos();

        int dx = pos.x() - pressPos_.x();
        int dy = pos.y() - pressPos_.y();

        if (dx || dy) {
          if (! moving) {
            if      (pressSide_ & LEFT_SIDE)
              adjustMenuRect(dx, 0, 0, 0);
            else if (pressSide_ & RIGHT_SIDE)
              adjustMenuRect(0, 0, dx, 0);

            if      (pressSide_ & TOP_SIDE)
              adjustMenuRect(0, dy, 0, 0);
            else if (pressSide_ & BOTTOM_SIDE)
              adjustMenuRect(0, 0, 0, dy);
          }
          else {
            QRect r = geometry();

            move(r.x() + dx, r.y() + dy);
          }
        }

        pressPos_ = pos;
      }
      else
        updateCursor(frame, me->pos());

      break;
    }
    case QEvent::Enter: {
      QEnterEvent *ee = static_cast<QEnterEvent *>(e);

      updateCursor(frame, ee->pos());

      break;
    }
    case QEvent::Leave: {
      setCursor(Qt::ArrowCursor);

      break;
    }
    default:
      break;
  }
}

void
CQFrameMenu::
updateCursor(QFrame *frame, const QPoint &p)
{
  Side side;

  if (insideBorder(frame, p, side)) {
    switch (side) {
      case TOP_LEFT_SIDE: case BOTTOM_RIGHT_SIDE:
        setCursor(Qt::SizeFDiagCursor);
        break;
      case BOTTOM_LEFT_SIDE: case TOP_RIGHT_SIDE:
        setCursor(Qt::SizeBDiagCursor);
        break;
      case TOP_SIDE: case BOTTOM_SIDE:
        setCursor(Qt::SizeVerCursor);
        break;
      case LEFT_SIDE: case RIGHT_SIDE:
        setCursor(Qt::SizeHorCursor);
        break;
      default:
        setCursor(Qt::ArrowCursor);
        break;
    }
  }
  else
    setCursor(Qt::ArrowCursor);
}

bool
CQFrameMenu::
insideBorder(QFrame *frame, const QPoint &p, Side &side) const
{
#if 0
  if (! sideInited_ && popupWidget()) {
    QPoint pos1 = mapToGlobal(pos());
    QPoint pos2 = popupWidget()->mapToGlobal(popupWidget()->pos());

    CQFrameMenu *th = const_cast<CQFrameMenu *>(this);

    th->resizeSides_ = ALL_SIDES;

    if (pos1.y() > pos2.y()) th->resizeSides_ &= ~TOP_SIDE;
    else                     th->resizeSides_ &= ~BOTTOM_SIDE;

    th->sideInited_ = true;
  }
#endif

  //-----

  side = NO_SIDE;

  uint side1 = NO_SIDE;

  int w = frame->width ();
  int h = frame->height();

  int fw = frame->frameWidth();
//std::cerr << "pos " << p.x() << " " << p.y() << std::endl;
//std::cerr << "frame size " << w << " " << h << " " << fw << std::endl;

  if      (p.x() >= 0 && p.x() < fw)
    side1 |= LEFT_SIDE;
  else if (p.x() < w && p.x() >= w - fw)
    side1 |= RIGHT_SIDE;

  if      (p.y() >= 0 && p.y() < fw)
    side1 |= TOP_SIDE;
  else if (p.y() < h && p.y() >= h - fw)
    side1 |= BOTTOM_SIDE;

  // remove non-allowed sides
  side1 &= resizeSides();

  if (side1 == NO_SIDE)
    return false;

  side = (Side) side1;

//std::cerr << "insideBorder " << p.x() << " " << p.y() << std::endl;
  return true;
}

//------

CQFrameMenuFrame::
CQFrameMenuFrame(CQFrameMenu *menu) :
 QFrame(0), menu_(menu)
{
  setFrameStyle(QFrame::Panel | QFrame::Raised);
  setLineWidth(3);

  layout_ = new QVBoxLayout(this);
  layout_->setMargin(0); layout_->setSpacing(0);
}

void
CQFrameMenuFrame::
setWidget(QWidget *w)
{
  layout_->addWidget(w);
}

void
CQFrameMenuFrame::
showEvent(QShowEvent *)
{
  setMouseTracking(true);

  installEventFilter(this);
}

void
CQFrameMenuFrame::
hideEvent(QHideEvent *)
{
  setMouseTracking(false);

  removeEventFilter(this);
}

bool
CQFrameMenuFrame::
eventFilter(QObject *, QEvent *e)
{
  menu_->processFrameEvent(this, e);

  return true;
}

//------

CQFrameMenuAction::
CQFrameMenuAction(CQFrameMenu *menu) :
 QWidgetAction(menu), menu_(menu)
{
}

QWidget *
CQFrameMenuAction::
createWidget(QWidget *parent)
{
  QFrame *frame = menu_->frame();

  frame->setParent(parent);

  frame->show();

  return frame;
}

void
CQFrameMenuAction::
deleteWidget(QWidget *)
{
  QFrame *frame = menu_->frame();

  frame->setParent(0);

  frame->hide();
}

//------

CQFrameMenuScrollArea::
CQFrameMenuScrollArea(CQFrameMenu *menu) :
 QFrame(0), menu_(menu), w_(0), cw_(-1), ch_(-1)
{
  // set frame size
  setFrameStyle(QFrame::Panel | QFrame::Raised);
  setLineWidth(3);

  // create scrollbars
  hbar_ = new QScrollBar(Qt::Horizontal, this);
  vbar_ = new QScrollBar(Qt::Vertical  , this);

  hbar_->setCursor(Qt::ArrowCursor);
  vbar_->setCursor(Qt::ArrowCursor);

  connect(hbar_, SIGNAL(valueChanged(int)), this, SLOT(hscrollSlot(int)));
  connect(vbar_, SIGNAL(valueChanged(int)), this, SLOT(vscrollSlot(int)));

  // create contents widget
  contents_ = new QWidget(this);

  contents_->setCursor(Qt::ArrowCursor);

  resizeEvent(0);
}

void
CQFrameMenuScrollArea::
setWidget(QWidget *w)
{
  if (w_)
    w_->setParent(0);

  w_ = w;

  w_->setParent(contents_);

  resizeEvent(0);
}

QSize
CQFrameMenuScrollArea::
initSize()
{
  if (cw_ <= 0 || ch_ <= 0) {
    QSize s = CQWidgetUtil::SmartMinSize(w_);
//std::cerr << "minSize " << s.width() << " " << s.height() << std::endl;

    int fw = frameWidth();

    cw_ = s.width () + 2*fw;
    ch_ = s.height() + 2*fw;
  }

  return QSize(cw_, ch_);
}

// handle horizontal scroll
void
CQFrameMenuScrollArea::
hscrollSlot(int value)
{
  // scroll area widget
  if (w_)
    w_->move(-value, 0);
}

// handle vertical scroll
void
CQFrameMenuScrollArea::
vscrollSlot(int value)
{
  // scroll area widget
  if (w_)
    w_->move(0, -value);
}

void
CQFrameMenuScrollArea::
showEvent(QShowEvent *)
{
  setMouseTracking(true);

  installEventFilter(this);
}

void
CQFrameMenuScrollArea::
hideEvent(QHideEvent *)
{
  setMouseTracking(false);

  removeEventFilter(this);
}

void
CQFrameMenuScrollArea::
applySize(int w, int h)
{
  cw_ = std::max(w, minimumSizeHint().width ());
  ch_ = std::max(h, minimumSizeHint().height());

  //resize(QSize(cw_, ch_));
  setFixedSize(cw_, ch_);

  updateSize(cw_, ch_);
}

void
CQFrameMenuScrollArea::
resizeEvent(QResizeEvent *)
{
  updateSize(width(), height());
}

void
CQFrameMenuScrollArea::
updateSize(int w, int h)
{
//std::cerr << "CQFrameMenuScrollArea::resizeEvent " << w << " " << h << std::endl;

  //int l, r, t, b;
  //getContentsMargins(&l, &t, &r, &b);

  int fw = frameWidth();
//std::cerr << "frameWidth " << frameWidth() << std::endl;

  int iw = w - 2*fw;
  int ih = h - 2*fw;

  QSize s = (w_ ? CQWidgetUtil::SmartMinSize(w_) : minimumSizeHint());

  //---

  int sw = vbar_->sizeHint().width ();
  int sh = hbar_->sizeHint().height();

  bool hvis = s.width () > iw;
  bool vvis = s.height() > ih;

  if (! hvis && vvis) hvis = (s.width () > iw - sw);
  if (! vvis && hvis) vvis = (s.height() > ih - sh);

  hbar_->setVisible(hvis);
  vbar_->setVisible(vvis);

  if (! hvis) sh = 0;
  if (! vvis) sw = 0;

  int cw = std::max(iw - sw, s.width ());
  int ch = std::max(ih - sh, s.height());
//std::cerr << "contents " << cw << " " << ch << std::endl;

  //---

  contents_->move(fw, fw);

  contents_->resize(iw - sw, ih - sh);

  if (w_) {
    w_->move((hvis ? -hbar_->value() : 0), (vvis ? -vbar_->value() : 0));
    w_->resize(cw, ch);
  }

  if (hvis) {
    hbar_->move(fw, ih + fw - sh);
    hbar_->resize(iw - sw, sh);

    hbar_->setPageStep(iw - sw);
    hbar_->setRange(0, cw - hbar_->pageStep());
  }

  if (vvis) {
    vbar_->move(fw + iw - sw, fw);
    vbar_->resize(sw, ih - sh);

    vbar_->setPageStep(ih - sh);
    vbar_->setRange(0, ch - vbar_->pageStep());
  }
}

bool
CQFrameMenuScrollArea::
eventFilter(QObject *, QEvent *e)
{
  menu_->processFrameEvent(this, e);

  return true;
}

QSize
CQFrameMenuScrollArea::
minimumSizeHint() const
{
  int fw = frameWidth();

  if (cw_ <= 0 || cw_ <= 0) {
    QSize s(0, 0);

    if (w_)
      s = CQWidgetUtil::SmartMinSize(w_);

    int sw = vbar_->sizeHint().width ();
    int sh = hbar_->sizeHint().height();

    return QSize(s.width() + 2*fw + sw, s.height() + 2*fw + sh);
  }
  else
    return QSize(2*fw + 32, 2*fw + 32);
}

QSize
CQFrameMenuScrollArea::
maximumSizeHint() const
{
  if (w_ && menu_->sizeHintIsMax()) {
    int fw = frameWidth();

    QSize s = w_->sizeHint();

    return QSize(s.width() + 2*fw, s.height() + 2*fw);
  }
  else
    return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}
