#include <CQToolStrip.h>
#include <CQWidgetUtil.h>
#include <QLabel>
#include <QLineEdit>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOption>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <iostream>

CQToolStrip::
CQToolStrip(QWidget *parent) :
 QWidget(parent), orientation_(Qt::Horizontal), menu_(0), splitterPos_(0), labelHeight_(0)
{
  menuButton_ = new CQToolStripMenuButton(this);

  menu_ = new CQToolStripMenu(this);

  menuButton_->setMenu(menu_);
}

CQToolStripArea *
CQToolStrip::
addWidget(QWidget *w)
{
  CQToolStripArea *area = new CQToolStripArea(this);

  area->setWidget(w);

  if (qobject_cast<QLineEdit *>(w))
    area->setResizable(true);

  addArea(area);

  return area;
}

CQToolStripArea *
CQToolStrip::
addWidget(const QString &label, QWidget *w)
{
  CQToolStripArea *area = new CQToolStripArea(this);

  area->setWidget(w);
  area->setLabel (label);

  if (qobject_cast<QLineEdit *>(w))
    area->setResizable(true);

  addArea(area);

  return area;
}

void
CQToolStrip::
addArea(CQToolStripArea *area)
{
  areas_.push_back(area);

  updateLayout(true);
}

void
CQToolStrip::
showEvent(QShowEvent *)
{
  updateLayout(true);
}

void
CQToolStrip::
resizeEvent(QResizeEvent *)
{
  //expandToFit();

  updateLayout(true);
}

void
CQToolStrip::
updateLayout(bool updateSplitters)
{
  if (updateSplitters) {
    menuButton_->hide();

    int n = areas_.size();

    for (int i = 0; i < n; ++i) {
      CQToolStripArea *area = areas_[i];

      area->setVisible(true);
    }

    updateLabelHeight();

    bool clip = reduceSize();

    updateVisible();

    hideSplitters();

    // place areas
    int x = 2;

    for (int i = 0; i < n; ++i) {
      CQToolStripArea *area = areas_[i];

      //if (! area->isVisible()) continue;

      area->updateLayout();

      int w = area->displayWidth();

      area->move  (x, 0);
      area->resize(w, height());

      x += w + 2;

      if (area->isResizable() && i < n - 1) {
        CQToolStripSplitter *splitter = getSplitter();

        splitter->init(i, Qt::Vertical);

        splitter->move  (x - 1, 0);
        splitter->resize(4, height());

        splitter->show();

        connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));

        x += 4; // splitter width
      }
    }

    menuButton_->setVisible(clip);

    if (clip) {
      //menuButton_->resize(8, height());

      menuButton_->move( width () - menuButton_->width (),
                        (height() - menuButton_->height())/2);

      menuButton_->raise();
    }
  }
  else {
    int spliiterNum = 0;

    // place areas
    int x = 2;

    int n = areas_.size();

    for (int i = 0; i < n; ++i) {
      CQToolStripArea *area = areas_[i];

      //if (! area->isVisible()) continue;

      int w = area->displayWidth();

      area->move  (x, 0);
      area->resize(w, height());

      x += w + 2;

      if (area->isResizable() && i < n - 1) {
        CQToolStripSplitter *splitter = splitters_[spliiterNum++];

        disconnect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));

        splitter->move(x - 1, 0);

        connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved(int,int)));

        x += 4; // splitter width
      }
    }
  }
}

void
CQToolStrip::
updateLabelHeight()
{
  int n = areas_.size();

  // determine label height
  labelHeight_ = 0;

  for (int i = 0; i < n; ++i) {
    CQToolStripArea *area = areas_[i];

    //if (! area->isVisible()) continue;

    int labelHeight = area->labelMinHeight();

    labelHeight_ = std::max(labelHeight_, labelHeight);
  }
//std::cerr << "labelHeight " << labelHeight_ << std::endl;
}

int
CQToolStrip::
contentsWidth() const
{
  int w = 2;

  int n = areas_.size();

  for (int i = 0; i < n; ++i) {
    CQToolStripArea *area = areas_[i];

    int w1 = area->displayWidth();

    w += w1 + 2;

    if (area->isResizable() && i < n - 1)
      w += 4; // splitter width
  }

  return w;
}

std::vector<int>
CQToolStrip::
getResizeInds() const
{
  std::vector<int> inds;

  int n = areas_.size();

  for (int i = 0; i < n; ++i) {
    CQToolStripArea *area = areas_[i];

    if (area->isResizable() && i < n - 1)
      inds.push_back(i);
  }

  return inds;
}

bool
CQToolStrip::
reduceSize()
{
  int cw = contentsWidth();

  int d = cw - width();

  // shrink resizable indexes as mush as possible if too small
  std::vector<int> inds = getResizeInds();

  while (d > 0 && ! inds.empty()) {
    int ind = inds.back();

    inds.pop_back();

    CQToolStripArea *area = areas_[ind];

    int curW = area->displayWidth();
    int minW = area->minimumSizeHint().width();

    if (curW > minW) {
      int newW = std::max(curW - d, minW);

      d -= curW - newW;

      if (newW != curW)
        area->setDisplayWidth(newW);
    }
  }

  return (d > 0);
}

void
CQToolStrip::
updateVisible()
{
  bool visible = true;
  int  visInd  = -1;

  int x = 2;

  int n = areas_.size();

  for (int i = 0; i < n; ++i) {
    CQToolStripArea *area = areas_[i];

    int w = area->displayWidth();

    if (visible) {
      if (x + w + 2 > width()) {
        visible = false;
        visInd  = i;
      }
    }

    area->setClipped(! visible);
    area->setVisible(visible);

    x += w + 2;

    if (area->isResizable() && i < n - 1)
      x += 4;
  }

  //---

  visible = true;

  x = 2;

  int bw = menuButton_->width();

  for (int i = 0; i < visInd; ++i) {
    CQToolStripArea *area = areas_[i];

    int w = area->displayWidth();

    if (visible) {
      if (x + w + bw + 2 > width())
        visible = false;
    }

    area->setClipped(! visible);
    area->setVisible(visible);

    x += w + 2;

    if (area->isResizable() && i < n - 1)
      x += 4;
  }
}

void
CQToolStrip::
hideSplitters()
{
  for (int i = 0; i < splitterPos_; ++i)
    splitters_[i]->hide();

  splitterPos_ = 0;
}

CQToolStripSplitter *
CQToolStrip::
getSplitter()
{
  while (splitterPos_ >= int(splitters_.size()))
    splitters_.push_back(new CQToolStripSplitter(this));

  CQToolStripSplitter *splitter = splitters_[splitterPos_++];

  return splitter;
}

void
CQToolStrip::
splitterMoved(int ind, int d)
{
  int fitW = contentsWidth();

  CQToolStripArea *area = areas_[ind];

  int min_area_width = area->minimumSizeHint().width();
  int max_area_width = width() - minimumSizeHint().width();
//std::cerr << "area min width " << min_area_width << std::endl;
//std::cerr << "area max width " << max_area_width << std::endl;

  int w = std::min(std::max(area->displayWidth() + d, min_area_width), max_area_width);

  d = w - area->displayWidth();
//std::cerr << "d " << d << std::endl;

  if (d > 0) {
    // calc width
    int cw = contentsWidth() + d;

    int dw = cw - width();

    if (dw > 0) {
//std::cerr << "shrink others " << dw << std::endl;
      int n = areas_.size();

      for (int i = ind + 1; i < n - 1; ++i) {
        CQToolStripArea *area1 = areas_[i];

        //if (! area1->isVisible()) continue;

        if (! area1->isResizable()) continue;

        int w1    = area1->displayWidth();
        int minW1 = area1->minimumSizeHint().width();

        if (w1 > minW1) {
          w1 -= dw;

          int newW1 = std::max(w1 - dw, minW1);

          dw -= w1 - newW1;

          if (newW1 != w1)
            area1->setDisplayWidth(newW1);
        }
      }

      cw = width() + dw;
    }

    //---

    int maxW = width() - contentsWidth() + area->displayWidth();

    if (w > maxW) w = maxW;

    area->setDisplayWidth(w);

    //---

    expandToFit(ind, fitW);
  }
  else {
    int minW = area->minimumSizeHint().width();

    if (w < minW) w = minW;

    area->setDisplayWidth(w);

    //---

    expandToFit(ind, fitW);
  }

  updateLayout(false);
}

void
CQToolStrip::
expandToFit(int stopInd, int fitW)
{
  if (fitW < 0)
    fitW = contentsWidth();
//std::cerr << "Expand to fit " << fitW << std::endl;

  int n = areas_.size();

  for (int i = n - 2; i >= 0; --i) {
    if (i == stopInd) break;

    CQToolStripArea *area1 = areas_[i];

    //if (! area1->isVisible()) continue;

    if (! area1->isResizable()) continue;

    int dw = fitW - contentsWidth();

    int w1 = area1->displayWidth() + dw;

    int minW1 = area1->minimumSizeHint().width();

    if (w1 < minW1) w1 = minW1;

    area1->setDisplayWidth(w1);

    return;
  }
}

QSize
CQToolStrip::
sizeHint() const
{
  int w = 2, h = 0;

  int n = areas_.size();

  for (int i = 0; i < n; ++i) {
    CQToolStripArea *area = areas_[i];

    w += area->minimumSizeHint().width() + 2;
    h  = std::max(h, area->sizeHint().height());

    if (area->isResizable() && i < n - 1)
      w += 4;
  }

//std::cerr << "CQToolStrip::sizeHint " << w << " " << h << std::endl;
  return QSize(w, h);
}

QSize
CQToolStrip::
minimumSizeHint() const
{
  int h = 0;

  int n = areas_.size();

  for (int i = 0; i < n; ++i) {
    CQToolStripArea *area = areas_[i];

    h = std::max(h, area->minimumSizeHint().height());
  }

  int w = 32;

//std::cerr << "CQToolStrip::minimumSizeHint " << w << " " << h << std::endl;
  return QSize(w, h);
}

//-------

CQToolStripArea::
CQToolStripArea(CQToolStrip *strip) :
 QWidget(strip), strip_(strip), w_(0), flags_(NoFlags),
 alignment_(Qt::AlignLeft | Qt::AlignBottom), label_(0), resizable_(false),
 displayWidth_(-1), clipped_(false)
{
}

void
CQToolStripArea::
setWidget(QWidget *w)
{
  if (w_)
    w_->setParent(0);

  w_ = w;

  if (w_)
    w_->setParent(this);
}

void
CQToolStripArea::
setFlags(Flags flags)
{
  flags_ = flags;
}

void
CQToolStripArea::
setAlignment(Qt::Alignment alignment)
{
  alignment_ = alignment;
}

void
CQToolStripArea::
setLabel(const QString &label)
{
  if (! label_)
    label_ = new QLabel(this);

  QString label1 = QString("<small><bold>%1</bold></small>").arg(label);

  label_->setText(label1);
}

void
CQToolStripArea::
unsetLabel()
{
  delete label_;

  label_ = 0;
}

void
CQToolStripArea::
setResizable(bool resizable)
{
  resizable_ = resizable;
}

int
CQToolStripArea::
labelMinHeight() const
{
  return (label_ ? label_->minimumSizeHint().height() : 0);
}

int
CQToolStripArea::
labelHeight() const
{
  QWidget *parent = parentWidget();

  int lh = strip_->labelHeight();

  if (! qobject_cast<CQToolStrip *>(parent))
    lh = (label_ ? label_->minimumSizeHint().height() : 0);

  return lh;
}

int
CQToolStripArea::
displayWidth() const
{
  if (displayWidth_ >= 0)
    return displayWidth_;
  else
    return minimumSizeHint().width();
}

void
CQToolStripArea::
setDisplayWidth(int w)
{
//std::cerr << "set display width " << w << std::endl;
  displayWidth_ = w;
}

void
CQToolStripArea::
resizeEvent(QResizeEvent *)
{
  updateLayout();
}

void
CQToolStripArea::
updateLayout()
{
  if (! w_) return;

  if (label_)
    label_->move(0, 0);

  int lh = labelHeight();

  w_->move  (0, lh);
  w_->resize(width(), height() - lh);
}

void
CQToolStripArea::
setClipped(bool clipped)
{
  clipped_ = clipped;
}

bool
CQToolStripArea::
isClipped() const
{
  return clipped_;
  //return (x() + width() > strip_->width());
}

QSize
CQToolStripArea::
sizeHint() const
{
  int lh = labelHeight();

  int w = 0, h = 0;

  if (w_) {
    w = w_->sizeHint().width();
    h = w_->sizeHint().height();
  }

  if (label_) {
    QSize ls = label_->minimumSizeHint();

    w  = std::max(w, ls.width());
    h += lh;
  }

//std::cerr << "CQToolStripArea::sizeHint " << w << " " << h << std::endl;
  return QSize(w, h);
}

QSize
CQToolStripArea::
minimumSizeHint() const
{
  int lh = labelHeight();

  QSize s;

  if (w_)
    s = CQWidgetUtil::SmartMinSize(w_);

  if (label_) {
    QSize ls = label_->minimumSizeHint();

    s = QSize(std::max(s.width(), ls.width()), s.height() + lh);
  }

//std::cerr << "CQToolStripArea::minimumSizeHint " << s.width() << " " << s.height() << std::endl;
  return s;
}

//------

CQToolStripSplitter::
CQToolStripSplitter(CQToolStrip *strip) :
 QWidget(strip), strip_(strip), ind_(-1), orient_(Qt::Vertical), mouseOver_(false)
{
  setCursor(Qt::SplitHCursor);

  setVisible(false);
}

void
CQToolStripSplitter::
init(int ind, Qt::Orientation orient)
{
  ind_    = ind;
  orient_ = orient;

  if (orient_ == Qt::Vertical)
    setCursor(Qt::SplitHCursor);
  else
    setCursor(Qt::SplitVCursor);
}

void
CQToolStripSplitter::
paintEvent(QPaintEvent *)
{
  QStylePainter ps(this);

  QStyleOption opt;

  opt.initFrom(this);

  opt.rect  = rect();
  opt.state = (orient_ == Qt::Horizontal ? QStyle::State_None : QStyle::State_Horizontal);

  if (mouseState_.pressed)
    opt.state |= QStyle::State_Sunken;

  if (mouseOver_)
    opt.state |= QStyle::State_MouseOver;

  ps.drawControl(QStyle::CE_Splitter, opt);
}

void
CQToolStripSplitter::
mousePressEvent(QMouseEvent *e)
{
  mouseState_.pressed  = true;
  mouseState_.pressPos = e->globalPos();

  update();
}

void
CQToolStripSplitter::
mouseMoveEvent(QMouseEvent *e)
{
  if (! mouseState_.pressed) return;

  if (orient_ == Qt::Horizontal) {
    int dy = e->globalPos().y() - mouseState_.pressPos.y();

    if (dy)
      emit splitterMoved(ind_, dy);
  }
  else {
    int dx = e->globalPos().x() - mouseState_.pressPos.x();

    if (dx)
      emit splitterMoved(ind_, dx);
  }

  mouseState_.pressPos = e->globalPos();

  update();
}

void
CQToolStripSplitter::
mouseReleaseEvent(QMouseEvent *)
{
  mouseState_.pressed = false;

  update();
}

void
CQToolStripSplitter::
enterEvent(QEvent *)
{
  mouseOver_ = true;

  update();
}

void
CQToolStripSplitter::
leaveEvent(QEvent *)
{
  mouseOver_ = false;

  update();
}

//-----------

CQToolStripMenuButton::
CQToolStripMenuButton(CQToolStrip *strip) :
 QToolButton(strip), strip_(strip)
{
  setObjectName("menu_button");

  setFocusPolicy(Qt::NoFocus);

  setAutoRaise(true);

  setPopupMode(QToolButton::MenuButtonPopup);

  setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

  int ext = style()->pixelMetric(QStyle::PM_ToolBarExtensionExtent);

  setFixedSize(ext + 2, ext + 2);

  QStyleOption opt;

  opt.init(this);

  if (strip_->orientation() == Qt::Horizontal)
    setIcon(style()->standardIcon(QStyle::SP_ToolBarHorizontalExtensionButton, &opt));
  else
    setIcon(style()->standardIcon(QStyle::SP_ToolBarVerticalExtensionButton, &opt));
}

void
CQToolStripMenuButton::
paintEvent(QPaintEvent *)
{
  QStylePainter p(this);

  QStyleOptionToolButton opt;

  initStyleOption(&opt);

  // We do not need to draw both extension arrows
  opt.features &= ~QStyleOptionToolButton::HasMenu;

  p.drawComplexControl(QStyle::CC_ToolButton, opt);
}

//------

CQToolStripMenu::
CQToolStripMenu(CQToolStrip *strip) :
 CQFrameMenu(true), strip_(strip)
{
  setObjectName("menu");

  contents_ = new CQToolStripMenuContents(this);

  setWidget(contents_);

  connect(this, SIGNAL(openMenu()), this, SLOT(addActions()));
  connect(this, SIGNAL(closeMenu()), this, SLOT(removeActions()));
}

void
CQToolStripMenu::
addActions()
{
  // add clipped items to menu
  int n = strip_->numAreas();

  for (int i = 0; i < n; ++i) {
    CQToolStripArea *area = strip_->getArea(i);

    if (! area->isClipped()) continue;

    contents_->addArea(area);
  }
}

void
CQToolStripMenu::
removeActions()
{
  contents_->removeAreas(strip_);

  strip_->updateLayout(true);
}

//------

CQToolStripMenuContents::
CQToolStripMenuContents(CQToolStripMenu *menu) :
 QWidget(0), menu_(menu)
{
}

void
CQToolStripMenuContents::
addArea(CQToolStripArea *area)
{
  areas_.push_back(area);

  area->setParent(this);
  area->show();
}

void
CQToolStripMenuContents::
removeAreas(QWidget *parent)
{
  for (uint i = 0; i < areas_.size(); ++i) {
    areas_[i]->setParent(parent);

    areas_[i]->show();
  }

  areas_.clear();
}

void
CQToolStripMenuContents::
showEvent(QShowEvent *)
{
  updateLayout();
}

void
CQToolStripMenuContents::
resizeEvent(QResizeEvent *)
{
  updateLayout();
}

void
CQToolStripMenuContents::
updateLayout()
{
  int x = 2;
  int y = 2;

  for (uint i = 0; i < areas_.size(); ++i) {
    CQToolStripArea *area = areas_[i];

    QSize s  = area->sizeHint();
    QSize ms = area->minimumSizeHint();

    area->move(x, y);

    if (area->isResizable())
      area->resize(width() - 4, s.height());
    else
      area->resize(ms.width(), s.height());

    y += s.height() + 2;
  }
}

QSize
CQToolStripMenuContents::
sizeHint() const
{
  int w = 0;
  int h = 2;

  for (uint i = 0; i < areas_.size(); ++i) {
    CQToolStripArea *area = areas_[i];

    QSize s = area->sizeHint();

    w = std::max(w, s.width());

    h += s.height() + 2;
  }

  return QSize(w + 4, h);
}

QSize
CQToolStripMenuContents::
minimumSizeHint() const
{
  int w = 0;
  int h = 2;

  for (uint i = 0; i < areas_.size(); ++i) {
    CQToolStripArea *area = areas_[i];

    QSize s = area->minimumSizeHint();

    w = std::max(w, s.width());

    h += s.height() + 2;
  }

  return QSize(w + 4, h);
}
