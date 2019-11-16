#include <stdio.h>
#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QTimer>
#include <QMouseEvent>
#include <QToolButton>
#include <QPushButton>
#include <QCheckBox>
#include <QIcon>
#include <QLCDNumber>
#include <QMessageBox>

#include "game.h"

Game *game;

void Game::update() {

	if (game->hint) {
		int x = QCursor::pos().x() - guiBoard[0]->mapToGlobal(QPoint(0,0)).x();
		int y = QCursor::pos().y() - guiBoard[0]->mapToGlobal(QPoint(0,0)).y();

		x /= STONE_SIZE;
		y /= STONE_SIZE;
		if (x < BOARD_SIZE && y < BOARD_SIZE && x >= 0 && y >= 0) {
			doHint(x, y);
		} else {
			doHint(-1, -1);
		}
	}

}



/* 可以监测鼠标事件的窗口 */
class QMouseEventWindow : public QWidget {
	protected:
		void mouseReleaseEvent(QMouseEvent *event);
};

/* 在窗口上释放鼠标 */
void QMouseEventWindow::mouseReleaseEvent(QMouseEvent *event) {
	printf("Mouse click on: (%d, %d)\n", event->x(), event->y());
	int x = event->x();
	int y = event->y();
	x /= STONE_SIZE;
	y /= STONE_SIZE;
	if (x < BOARD_SIZE && y < BOARD_SIZE && x >= 0 && y >= 0) {
		printf("Drop: (%d, %d)\n", x, y);
		game->userDrop(x, y);
	}
}

void QWidget::keyPressEvent(QKeyEvent *event){
    switch (event->key()) {
    case Qt::Key_Escape:
        this->close();
        break;
    }
}

int main(int argc, char *argv[]) {

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

	QApplication app(argc, argv);

    /* 常数 */
	int boardSize = BOARD_SIZE;
	int stoneSize = STONE_SIZE;
	int boardWidth = stoneSize*boardSize;

    /* 新窗口 */
	QWidget *window = new QMouseEventWindow;
    window->setWindowTitle("红蓝棋");
	window->setFixedSize(boardWidth+200, boardWidth);
	QWidget *board = new QWidget(window);
	board->setFixedSize(boardWidth, boardWidth);
    board->setStyleSheet("background-color:white;");

    /* 载入棋子图片 */
	QPixmap *stoneWPix = new QPixmap(":/img/stoneW.png");
	QPixmap *stoneWPixS = new QPixmap;
	*stoneWPixS = stoneWPix->scaled(stoneSize, stoneSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	QPixmap *stoneBPix = new QPixmap(":/img/stoneB.png");
	QPixmap *stoneBPixS = new QPixmap;
	*stoneBPixS = stoneBPix->scaled(stoneSize, stoneSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	QPixmap *stoneWaPix = new QPixmap(":/img/stoneWa.png");
	QPixmap *stoneWaPixS = new QPixmap;
	*stoneWaPixS = stoneWaPix->scaled(stoneSize, stoneSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	QPixmap *stoneBaPix = new QPixmap(":/img/stoneBa.png");
	QPixmap *stoneBaPixS = new QPixmap;
	*stoneBaPixS = stoneBaPix->scaled(stoneSize, stoneSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	QPixmap *stoneWsPix = new QPixmap(":/img/stoneWs.png");
	QPixmap *stoneWsPixS = new QPixmap;
	*stoneWsPixS = stoneWsPix->scaled(stoneSize, stoneSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	QPixmap *stoneBsPix = new QPixmap(":/img/stoneBs.png");
	QPixmap *stoneBsPixS = new QPixmap;
	*stoneBsPixS = stoneBsPix->scaled(stoneSize, stoneSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	QPixmap *nullPix = new QPixmap(":/img/null.png");


    /* 棋盘网络排版 */
	QGridLayout *grid = new QGridLayout;
	grid->setSpacing(0);
	grid->setMargin(0);
	grid->setGeometry(QRect(0, 0, 10, 100));

    /* 构建棋盘 */
	QLabel *square[BOARD_SIZE*BOARD_SIZE];
	for (int i=0; i<boardSize; i++) {
		for (int j=0; j<boardSize; j++) {
			square[i*BOARD_SIZE+j] = new QLabel;
            square[i*BOARD_SIZE+j]->setFrameStyle(QFrame::Panel + QFrame::Plain);
			square[i*BOARD_SIZE+j]->setAlignment(Qt::AlignCenter);
			square[i*BOARD_SIZE+j]->setAttribute(Qt::WA_TranslucentBackground);
			square[i*BOARD_SIZE+j]->setAttribute(Qt::WA_TransparentForMouseEvents, true);
			grid->addWidget(square[i*BOARD_SIZE+j], i, j);
		}
	}
	QLabel *turnT = new QLabel(window);
	turnT->setGeometry(boardWidth, 20, 200, 14);
    turnT->setText("<center>下一个:</center>");
	QLabel *turn = new QLabel(window);
	turn->setGeometry(boardWidth+100-(STONE_SIZE/2), 45, STONE_SIZE, STONE_SIZE);
	turn->setFrameStyle(QFrame::Sunken);

    /* 游戏物件 */
	game = new Game(square, turn,
		stoneWPixS, stoneBPixS,
		stoneWaPixS, stoneBaPixS,
		stoneWsPixS, stoneBsPixS,
		nullPix);

	/* UI */
    QCheckBox *hintCheckBox = new QCheckBox("显示提示");
	hintCheckBox->setParent(window);
	hintCheckBox->setGeometry(boardWidth+50, 210, 100, 20);
	QObject::connect(hintCheckBox, SIGNAL(stateChanged(int)), game, SLOT(hintSwitch(int)));

	QLabel *bScore = new QLabel(window);
	bScore->setGeometry(boardWidth, 100, 200, 14);
    bScore->setText("<center>蓝方</center>");
	QLabel *wScore = new QLabel(window);
	wScore->setGeometry(boardWidth, 150, 200, 14);
    wScore->setText("<center>红方</center>");
	QLCDNumber *lcdB = new QLCDNumber(window);
	QLCDNumber *lcdW = new QLCDNumber(window);
	lcdB->setGeometry(boardWidth+50, 120, 100, 24);
    lcdB->setPalette(Qt::blue);
	lcdW->setGeometry(boardWidth+50, 170, 100, 24);
    lcdW->setPalette(Qt::red);
	QObject::connect(game, SIGNAL(updateBScore(int)), lcdB, SLOT(display(int)));
	QObject::connect(game, SIGNAL(updateWScore(int)), lcdW, SLOT(display(int)));

    QCheckBox *aiBCheckBox = new QCheckBox("电脑");
	aiBCheckBox->setParent(window);
	aiBCheckBox->setGeometry(boardWidth+140, 100, 50, 14);
	QObject::connect(aiBCheckBox, SIGNAL(stateChanged(int)), game, SLOT(aiBSwitch(int)));

    QCheckBox *aiWCheckBox = new QCheckBox("电脑");
	aiWCheckBox->setParent(window);
	aiWCheckBox->setGeometry(boardWidth+140, 150, 50, 14);
	QObject::connect(aiWCheckBox, SIGNAL(stateChanged(int)), game, SLOT(aiWSwitch(int)));

    /* 按键 */
    QPushButton *undoBtn = new QPushButton("悔棋");
	undoBtn->setParent(window);
	undoBtn->setGeometry(boardWidth+140, 64, 42, 22);
	undoBtn->setEnabled(false);
	QObject::connect(undoBtn, SIGNAL(clicked()), game, SLOT(undo()));
	QObject::connect(game, SIGNAL(undoAvai(bool)), undoBtn, SLOT(setEnabled(bool)));

    QPushButton *restartBtn = new QPushButton("重新开始");
	restartBtn->setParent(window);
	restartBtn->setGeometry(boardWidth+50, 242, 100, 36);
    restartBtn->setStyleSheet("QPushButton{border:1px groove gray;}"
                           "QPushButton:hover{background-color: rgb(85, 170, 255);}"
                           "QPushButton:pressed{background-color: rgb(81, 162, 243);}");
	QObject::connect(restartBtn, SIGNAL(clicked()), game, SLOT(reset()));

    QPushButton *exitBtn = new QPushButton("退出");
	exitBtn->setParent(window);
	exitBtn->setGeometry(boardWidth+50, 272, 100, 36);
    exitBtn->setStyleSheet("QPushButton{border:1px groove gray;}"
                           "QPushButton:hover{background-color: rgb(85, 170, 255);}"
                           "QPushButton:pressed{background-color: rgb(81, 162, 243);}");
	QObject::connect(exitBtn, SIGNAL(clicked()), &app, SLOT(quit()));

    QTimer *update = new QTimer;
        update->start(10);
        QObject::connect(update, SIGNAL(timeout()), game, SLOT(update()));


    /* 准备窗口*/
	board->setLayout(grid);
	window->setMouseTracking(true);
	window->show();

	game->reset();

	return app.exec();
}
