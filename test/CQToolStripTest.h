#include <QDialog>

class CQToolStrip;

class CQToolStripTest : public QDialog {
  Q_OBJECT

 public:
  CQToolStripTest();

 private:
  CQToolStrip *strip_;
};
