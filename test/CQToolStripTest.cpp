#include <CQToolStripTest.h>
#include <CQToolStrip.h>
#include <QApplication>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QToolButton>
#include <iostream>

class LineEdit : public QLineEdit {
 public:
  LineEdit(QWidget *parent=0) :
   QLineEdit(parent) {
  }

 public:
  QSize sizeHint() const {
    QFontMetrics fm(font());

    return QSize(128, fm.height() + 8);
  }

  QSize minimumSizeHint() const {
    QFontMetrics fm(font());

    return QSize(64, fm.height() + 4);
  }
};

int
main(int argc, char **argv)
{
  QApplication app(argc, argv);

  CQToolStripTest *test = new CQToolStripTest;

  test->show();

  return app.exec();
}

CQToolStripTest::
CQToolStripTest()
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->setMargin(2); layout->setSpacing(2);

  strip_ = new CQToolStrip;

  strip_->addWidget("Button", new QPushButton("One"));
  strip_->addWidget("Edit 1", new LineEdit());
  strip_->addWidget("Tool"  , new QToolButton());
  strip_->addWidget("Edit 2", new LineEdit());
  strip_->addWidget(new QToolButton());

  layout->addWidget(strip_);

  QPushButton *button = new QPushButton("Quit");

  connect(button, SIGNAL(clicked()), this, SLOT(close()));

  layout->addWidget(button);
}
