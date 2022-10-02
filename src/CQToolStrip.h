/*!
 * Horizontal/Vertical bar of widgets where each widget can be resizable or fixed width.
 *
 * If the bar is not widget enough for all the widgets then they are moved to a popup menu
 *
 */

#include <QToolButton>
#include <CQFrameMenu.h>

class CQToolStripArea;
class CQToolStripSplitter;
class CQToolStripMenuButton;
class CQToolStripMenu;
class QLabel;

class CQToolStrip : public QWidget {
  Q_OBJECT

 public:
  CQToolStrip(QWidget *parent=0);

  Qt::Orientation orientation() const { return orientation_; }

  CQToolStripArea *addWidget(QWidget *w);
  CQToolStripArea *addWidget(const QString &label, QWidget *w);

  void addArea(CQToolStripArea *area);

  void hideSplitters();

  CQToolStripSplitter *getSplitter();

  int labelHeight() const { return labelHeight_; }

  int numAreas() const { return int(areas_.size()); }

  CQToolStripArea *getArea(int i) { return areas_[uint(i)]; }

  void updateLayout(bool updateSplitters);

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

 private:
  void expandToFit(int ind=-1, int fitW=-1);

  void showEvent(QShowEvent *) override;

  void resizeEvent(QResizeEvent *) override;

  int contentsWidth() const;

  std::vector<int> getResizeInds() const;

  void updateLabelHeight();

  bool reduceSize();

  void updateVisible();

 private slots:
  void splitterMoved(int ind, int d);

 private:
  typedef std::vector<CQToolStripArea *>     AreaArray;
  typedef std::vector<CQToolStripSplitter *> Splitters;

  Qt::Orientation        orientation_;
  CQToolStripMenuButton *menuButton_;
  CQToolStripMenu       *menu_;
  AreaArray              areas_;
  int                    splitterPos_;
  Splitters              splitters_;
  int                    labelHeight_;
};

class CQToolStripArea : public QWidget {
  Q_OBJECT

 public:
  enum Flags {
    NoFlags    = 0,
    ResizableX = (1<<0),
    ResizableY = (1<<1),
    Reizable   = (ResizableX | ResizableY)
  };

 public:
  CQToolStripArea(CQToolStrip *strip);

  QWidget *widget() const { return w_; }
  void setWidget(QWidget *w);

  Flags flags() const { return flags_; }
  void setFlags(Flags flags);

  Qt::Alignment alignment() const { return alignment_; }
  void setAlignment(Qt::Alignment alignment);

  void setLabel(const QString &label);
  void unsetLabel();

  bool isResizable() const { return resizable_; }
  void setResizable(bool resizable);

  int labelMinHeight() const;
  int labelHeight() const;

  int displayWidth() const;
  void setDisplayWidth(int w);

  void updateLayout();

  void setClipped(bool clipped);
  bool isClipped() const;

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

 private:
  void resizeEvent(QResizeEvent *) override;

 private:
  CQToolStrip   *strip_;
  QWidget       *w_;
  Flags          flags_;
  Qt::Alignment  alignment_;
  QLabel        *label_;
  bool           resizable_;
  int            displayWidth_;
  bool           clipped_;
};

class CQToolStripSplitter : public QWidget {
  Q_OBJECT

 public:
  CQToolStripSplitter(CQToolStrip *strip);

  void init(int ind, Qt::Orientation orient);

  void mousePressEvent  (QMouseEvent *e) override;
  void mouseMoveEvent   (QMouseEvent *e) override;
  void mouseReleaseEvent(QMouseEvent *e) override;

  void enterEvent(QEvent *e) override;
  void leaveEvent(QEvent *e) override;

 private:
  void paintEvent(QPaintEvent *) override;

 signals:
  void splitterMoved(int ind, int d);

 private:
  struct MouseState {
    bool   pressed;
    QPoint pressPos;

    MouseState() {
      pressed = false;
    }
  };

  CQToolStrip     *strip_;
  int              ind_;
  Qt::Orientation  orient_;
  MouseState       mouseState_;
  bool             mouseOver_;
};

class CQToolStripMenuButton : public QToolButton {
  Q_OBJECT

 public:
  CQToolStripMenuButton(CQToolStrip *strip);

 private:
  void paintEvent(QPaintEvent *) override;

 private:
  CQToolStrip *strip_;
};

class CQToolStripMenuContents;

class CQToolStripMenu : public CQFrameMenu {
  Q_OBJECT

 public:
  CQToolStripMenu(CQToolStrip *strip);

  CQToolStrip *strip() const { return strip_; }

 public slots:
  void addActions();
  void removeActions();

 private:
  CQToolStrip             *strip_;
  CQToolStripMenuContents *contents_;
  QVBoxLayout             *layout_;
};

class CQToolStripMenuContents : public QWidget {
  Q_OBJECT

 public:
  CQToolStripMenuContents(CQToolStripMenu *menu);

  void addArea(CQToolStripArea *area);

  void removeAreas(QWidget *parent);

 private:
  void showEvent(QShowEvent *) override;
  void resizeEvent(QResizeEvent *) override;

  void updateLayout();

  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

 private:
  typedef std::vector<CQToolStripArea *> Areas;

  CQToolStripMenu *menu_;
  Areas            areas_;
};
